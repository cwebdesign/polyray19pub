/** @file image_decoder.cc
 *  @brief Thin format-agnostic image decode layer over the legacy JPEG/GIF/PNG loaders.
 *
 *  This module accepts a unified @c ImageSource from a path, @c FILE* handle,
 *  or memory buffer, detects or honors the requested image format, invokes the
 *  existing legacy decoders, converts their @c Img output into a neutral
 *  @c DecodedBitmap, and can also populate a fresh legacy @c Img from that
 *  neutral bitmap representation. It does not replace the old decoders yet; it
 *  wraps them behind a cleaner API so callers no longer need to depend directly
 *  on format-specific entry points such as @c read_JPEG_image().
 */
#include "image_decoder.h"

#include <cstring>
#include <string>
#include <utility>

#include "image.h"
#include "jpeg.h"
#include "memory.h"
#include "factory.h"
#include "pngread.h"

namespace {

constexpr unsigned char kPngSignature[8] = {
    0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A
};

/** @brief Build a failed decode result with a normalized error payload.
 *  @param code Normalized decode error code.
 *  @param message Human-readable error message.
 *  @return Decode result with no image and the supplied error state.
 */
DecodeStatus make_decode_error(DecodeErrorCode code, std::string message)
{
    DecodeStatus result;
    result.code = code;
    result.message = std::move(message);
    return result;
}

/** @brief Build a failed Img-population result with a normalized error payload.
 *  @param code Normalized population error code.
 *  @param message Human-readable error message.
 *  @return Population result containing the supplied error state.
 */
ImgPopulateResult make_populate_error(ImgPopulateErrorCode code, std::string message)
{
    ImgPopulateResult result;
    result.status.code = code;
    result.status.message = std::move(message);
    return result;
}

/** @brief Return a display name for diagnostics, or a fallback string when absent.
 *  @param source Unified image source description.
 *  @param fallback Fallback C-string used when the source has no display name.
 *  @return Pointer to a stable C-string owned by @p source or the fallback literal.
 */
const char* source_name_or_default(const ImageSource& source, const char* fallback)
{
    return source.display_name.empty() ? fallback : source.display_name.c_str();
}

/** @brief Return a display name for diagnostics, or a fallback string when absent.
 *  @param source Unified image source description.
 *  @param fallback Fallback C-string used when the source has no display name.
 *  @return Pointer to a stable C-string owned by @p source or the fallback literal.
 */
std::string source_name_or_default_cpp(const ImageSource& source, std::string fallback)
{
    return source.display_name.empty() ? fallback : source.display_name;
}

/** @brief Return a display name for diagnostics, or a fallback string when absent.
 *  @note Kept alongside the C-string helper for future C++-native call sites.
 *  @param source Unified image source description.
 *  @param fallback Fallback C-string used when the source has no display name.
 *  @return Copy of the source display name or the fallback string.
 */
std::string source_name_or_default_cpp(const ImageSource& source,
    const char* fallback)
{
    return source.display_name.empty() ? std::string(fallback) : source.display_name;
}

/** @brief Duplicate a source name into Polyray-managed memory for legacy @c Img storage.
 *  @param name Null-terminated source name to copy.
 *  @return Newly allocated C-string, or @c nullptr on allocation failure.
 */
char* duplicate_filename(const char* name)
{
    const size_t len = std::strlen(name);
    auto* copy = static_cast<char*>(polyray_malloc(len + 1));
    if (copy == nullptr) {
        return nullptr;
    }
    std::memcpy(copy, name, len + 1);
    return copy;
}

/** @brief Duplicate a source name into Polyray-managed memory for legacy @c Img storage.
 *  @note Kept alongside the C-string helper for future std::string-based call sites.
 *  @param name Source name to copy.
 *  @return Newly allocated C-string, or @c nullptr on allocation failure.
 */
char* duplicate_filename_cpp(const std::string& name)
{
    const size_t len = name.size();
    auto* copy = static_cast<char*>(polyray_malloc(len + 1));
    if (copy == nullptr) {
        return nullptr;
    }
    std::memcpy(copy, name.c_str(), len + 1);
    return copy;
}

/** @brief Detect an image format from leading bytes in a memory buffer.
 *  @param data Pointer to the first byte of the candidate image data.
 *  @param size Number of bytes available in @p data.
 *  @return Detected image format, or @c ImageFormat::unknown if no known signature matches.
 */
ImageFormat sniff_format_from_bytes(const unsigned char* data, size_t size)
{
    if (size >= 2 && data[0] == 0xFF && data[1] == 0xD8) {
        return ImageFormat::jpeg;
    }
    if (size >= 2 && data[0] == 'P' && data[1] == '1') {
        return ImageFormat::pbm;
    }
    if (size >= 8 && std::memcmp(data, kPngSignature, sizeof(kPngSignature)) == 0) {
        return ImageFormat::png;
    }
    if (size >= 6 && std::memcmp(data, "GIF87a", 6) == 0) {
        return ImageFormat::gif;
    }
    if (size >= 6 && std::memcmp(data, "GIF89a", 6) == 0) {
        return ImageFormat::gif;
    }
    return ImageFormat::unknown;
}

/** @brief Detect an image format by probing the first bytes of a seekable file.
 *  @param file Seekable file handle positioned arbitrarily by the caller.
 *  @return Detected image format, or @c ImageFormat::unknown on seek/probe failure.
 */
ImageFormat sniff_format_from_file(FILE* file)
{
    unsigned char probe[8] = { 0 };
    if (std::fseek(file, 0, SEEK_SET) != 0) {
        return ImageFormat::unknown;
    }
    const size_t bytes = std::fread(probe, 1, sizeof(probe), file);
    std::fseek(file, 0, SEEK_SET);
    return sniff_format_from_bytes(probe, bytes);
}

/** @brief Invoke the legacy decoder matching a concrete image format.
 *  @param file Open file handle containing the encoded image data.
 *  @param format Concrete image format to decode.
 *  @param img Legacy image structure to fill.
 *  @return 1 on successful decode, 0 when the selected decoder fails or is unsupported.
 */
int decode_file_into_img(FILE* file, ImageFormat format, Img* img)
{
    switch (format) {
    case ImageFormat::jpeg:
        return read_JPEG_image(file, img);
    case ImageFormat::gif:
        return read_GIF_image(file, img);
    case ImageFormat::png:
        return read_PNG_image(file, img);
    case ImageFormat::pbm:
		//return read_PBM_image(file, img);
		return 0; // PBM not implemented yet
    default:
        return 0;
    }
}

/** @brief Infer bytes per pixel from the legacy @c Img layout fields.
 *  @param img Legacy image structure produced by an existing decoder.
 *  @return Number of bytes per pixel, or 0 when the layout is unsupported.
 */
int bytes_per_pixel_from_img(const Img& img)
{
    switch (img.pixel_subtype) { //pixel sub-type identifier
    case 1:
    case 3:
        return 1;
    case 2:
        if (img.bytes_per_pixel == 32) {
            return 4;
        }
        return 3;
    default:
        return 0;
    }
}

/** @brief Convert a decoded legacy @c Img into the neutral @c DecodedBitmap form.
 *  @param img Legacy image structure filled by an existing decoder.
 *  @param format Source image format used to produce @p img.
 *  @return Successful decode result holding a neutral bitmap, or an error result if the image layout is unsupported.
 */
DecodeResult decoded_bitmap_from_img(const Img& img, ImageFormat format)
{
    DecodedBitmap bitmap;
    bitmap.width = img.width;
    bitmap.height = img.length;
    bitmap.origin = (img.scanline_order == 0 ? ImageOrigin::top_left : ImageOrigin::bottom_left);
    bitmap.source_format = format;
    DecodeResult res;

    const int bytes_per_pixel = bytes_per_pixel_from_img(img);
    if (img.width == 0 || img.length == 0 || bytes_per_pixel == 0 || img.image == nullptr) {
        res.status = make_decode_error(DecodeErrorCode::invalid_data, "Decoded image has invalid layout");
        return res;
    }

    switch (img.pixel_subtype) {
    case 1:
        bitmap.pixel_format = PixelFormat::index8;
        break;
    case 2:
        bitmap.pixel_format = (bytes_per_pixel == 4 ? PixelFormat::rgba8 : PixelFormat::rgb8);
        break;
    case 3:
        bitmap.pixel_format = PixelFormat::grey8;
        break;
    default:
        res.status = make_decode_error(DecodeErrorCode::unsupported_feature, "Decoded image format is unsupported");
        return res;
    }

    bitmap.row_stride_bytes = static_cast<size_t>(img.width) * static_cast<size_t>(bytes_per_pixel);
    bitmap.pixels.resize(bitmap.row_stride_bytes * static_cast<size_t>(img.length));
    for (unsigned row = 0; row < img.length; ++row) {
        if (img.image[row] == nullptr) {
            res.status = make_decode_error(DecodeErrorCode::invalid_data, "Decoded image row is null");
            return res;
        }
        std::memcpy(bitmap.pixels.data() + row * bitmap.row_stride_bytes,
                    img.image[row],
                    bitmap.row_stride_bytes);
    }

    if (bitmap.pixel_format == PixelFormat::index8) {
        if (img.cmap == nullptr && img.cmlen != 0) {
            res.status = make_decode_error(DecodeErrorCode::invalid_data, "Indexed image is missing palette data");
            return res;
        }
        bitmap.palette.entry_count = img.cmlen;
        bitmap.palette.rgb.resize(static_cast<size_t>(img.cmlen) * 3);
        if (img.cmlen != 0) {
            std::memcpy(bitmap.palette.rgb.data(), img.cmap, bitmap.palette.rgb.size());
        }
    }

    DecodeResult result;
    result.image = std::move(bitmap);
    return result;
}

/** @brief Decode a seekable file source through the legacy loaders and normalize the result.
 *  @param file Open seekable file handle containing encoded image bytes.
 *  @param source Unified source metadata used for diagnostics and source naming.
 *  @param options Decode options including optional format hint.
 *  @return Decode result containing a neutral bitmap on success, or a normalized error on failure.
 */
DecodeResult decode_open_file(FILE* file, const ImageSource& source, const DecodeOptions& options)
{
    DecodeResult res;
    if (file == nullptr) {
        res.status=make_decode_error(DecodeErrorCode::io_error, "Image source file handle is null");
        return res;
    }

    ImageFormat format = options.format_hint;
    if (format == ImageFormat::unknown) {
        format = sniff_format_from_file(file);
    }
    if (format == ImageFormat::unknown || format == ImageFormat::bmp || format == ImageFormat::tga) {
        res.status=make_decode_error(DecodeErrorCode::unsupported_format, "Unsupported or undetected image format");
        return res;
    }

    auto* decoded = FactoryImg();

    decoded->filename = source_name_or_default_cpp(source, "<image>");
    if (decoded->filename == "") {
        FreeImg(decoded);
        res.status=make_decode_error(DecodeErrorCode::out_of_memory, "Failed to allocate image source name");
        return res;
    }

    if (!decode_file_into_img(file, format, decoded)) {
        FreeImg(decoded);
        res.status=make_decode_error(DecodeErrorCode::invalid_data, "Existing decoder rejected image data");
        return res;
    }

    DecodeResult result = decoded_bitmap_from_img(*decoded, format);
    FreeImg(decoded);
    return result;
}

/** @brief Decode an image supplied as an in-memory byte buffer.
 *  @param source Unified image source whose active payload is a memory view.
 *  @param options Decode options including optional format hint.
 *  @return Decode result containing a neutral bitmap on success, or a normalized error on failure.
 */
DecodeResult decode_memory_source(const ImageSource& source, const DecodeOptions& options)
{
    DecodeResult res;
    const MemoryView& view = source.memory.view;
    if (view.data == nullptr || view.size == 0) {
        res.status=make_decode_error(DecodeErrorCode::invalid_data, "Memory image source is empty");
        return res;
    }

    FILE* tmp = std::tmpfile();
    if (tmp == nullptr) {
        res.status=make_decode_error(DecodeErrorCode::io_error, "Failed to create temporary file for memory image source");
        return res;
    }

    const size_t written = std::fwrite(view.data, 1, view.size, tmp);
    if (written != view.size) {
        std::fclose(tmp);
        res.status= make_decode_error(DecodeErrorCode::io_error, "Failed to stage memory image source");
        return res;
    }
    if (std::fflush(tmp) != 0 || std::fseek(tmp, 0, SEEK_SET) != 0) {
        std::fclose(tmp);
        res.status=make_decode_error(DecodeErrorCode::io_error, "Failed to prepare staged memory image source");
        return res;
    }

    DecodeResult result = decode_open_file(tmp, source, options);
    std::fclose(tmp);
    return result;
}

/** @brief Allocate row pointers and per-row storage for a legacy @c Img.
 *  @param out_img Legacy image structure to update.
 *  @param width Output width in pixels.
 *  @param length Output height in pixels.
 *  @param row_stride_bytes Number of bytes to allocate for each row.
 *  @return Successful population result when allocation completes, or an out-of-memory error result.
 */
ImgPopulateResult allocate_img_rows(Img& out_img, unsigned width, unsigned length, size_t row_stride_bytes)
{
    auto** rows = static_cast<unsigned char**>(polyray_malloc(length * sizeof(unsigned char*)));
    if (rows == nullptr) {
        return make_populate_error(ImgPopulateErrorCode::out_of_memory, "Failed to allocate image row table");
    }

    for (unsigned row = 0; row < length; ++row) {
        rows[row] = nullptr;
    }

    for (unsigned row = 0; row < length; ++row) {
        rows[row] = static_cast<unsigned char*>(polyray_malloc(row_stride_bytes));
        if (rows[row] == nullptr) {
            for (unsigned cleanup = 0; cleanup < row; ++cleanup) {
                polyray_free(rows[cleanup]);
            }
            polyray_free(rows);
            return make_populate_error(ImgPopulateErrorCode::out_of_memory, "Failed to allocate image row data");
        }
    }

    out_img.width = width;
    out_img.length = length;
    out_img.image = rows;
    return ImgPopulateResult{};
}

/** @brief Convert a legacy @c DecodeResult into the @c std::expected return form.
 *  @param result Decode result produced by one of the legacy-style decode helpers.
 *  @return The decoded bitmap, or the carried error status if decoding failed.
 */
std::expected<DecodedBitmap, DecodeStatus> to_expected(DecodeResult&& result)
{
    if (result.status.code != DecodeErrorCode::none || !result.image.has_value()) {
        return std::unexpected(std::move(result.status));
    }
    return std::move(*result.image);
}

} // namespace

/** @brief Decode an image from a unified source into a neutral bitmap representation.
 *  @param source Unified image source describing a path, @c FILE* handle, or memory buffer.
 *  @param options Decode options controlling format selection and output preferences.
 *  @return Decode result containing a @c DecodedBitmap on success, or a normalized decode error on failure.
 */
std::expected<DecodedBitmap,DecodeStatus> decode_image(const ImageSource& source, const DecodeOptions& options)
{
    switch (source.kind) {
    case SourceKind::file_path: {
        if (source.path.path.empty()) {           
            return std::unexpected(make_decode_error(DecodeErrorCode::invalid_data, "Image path source is empty"));
        }
        FILE* file = std::fopen(source.path.path.c_str(), "rb");
        if (file == nullptr) {
            return std::unexpected(make_decode_error(DecodeErrorCode::io_error, "Failed to open image file"));
        }
        DecodeResult result = decode_open_file(file, source, options);
        std::fclose(file);
        return to_expected(std::move(result));
    }
    case SourceKind::file_handle: {
        if (source.handle.file == nullptr) {
            return std::unexpected(make_decode_error(DecodeErrorCode::io_error, "Image file handle is null"));
        }
        if (!source.handle.seekable) {
            return std::unexpected(make_decode_error(DecodeErrorCode::unsupported_feature, "Non-seekable file handles are not supported"));
        }
        DecodeResult result = decode_open_file(source.handle.file, source, options);
        if (source.handle.ownership == OwnershipMode::owned) {
            std::fclose(source.handle.file);
        }
        return to_expected(std::move(result));
    }
    case SourceKind::memory_buffer:
        return to_expected(decode_memory_source(source, options));
    }

    return std::unexpected(make_decode_error(DecodeErrorCode::internal_error, "Unhandled image source kind"));
}

/** @brief Populate a legacy @c Img from a neutral decoded bitmap.
 *  @param bitmap Neutral bitmap to convert into legacy row/palette storage.
 *  @param out_img Legacy image structure to fill.
 *  @param options Population options controlling orientation handling and format conversions.
 *  @return Population result indicating success or the reason conversion/allocation failed.
 */
std::expected<void, ImgPopulateStatus> populate_img_from_bitmap(const DecodedBitmap& bitmap, Img& out_img, const ImgPopulateOptions& options)
{
    if (bitmap.width == 0 || bitmap.height == 0 || bitmap.row_stride_bytes == 0 || bitmap.pixels.empty()) {
        return std::unexpected(make_populate_error(ImgPopulateErrorCode::invalid_bitmap, "Bitmap is empty").status);
    }
    if (out_img.image != nullptr || out_img.cmap != nullptr) {
        return std::unexpected(make_populate_error(ImgPopulateErrorCode::internal_error, "Output Img already owns image data").status);
    }

    size_t expected_row_stride = 0;
    out_img.copy = 0;
    out_img.cflag = 0;
    out_img.cmlen = 0;
    out_img.cmsiz = 0;
    out_img.cmap = nullptr;
    out_img.scanline_order = (options.preserve_origin_flag && bitmap.origin == ImageOrigin::bottom_left) ? 1u : 0u;

    switch (bitmap.pixel_format) {
    case PixelFormat::grey8:
        out_img.pixel_subtype = 3;
        out_img.bytes_per_pixel = 8;
        expected_row_stride = static_cast<size_t>(bitmap.width);
        break;
    case PixelFormat::rgb8:
        out_img.pixel_subtype = 2;
        out_img.bytes_per_pixel = 24;
        expected_row_stride = static_cast<size_t>(bitmap.width) * 3;
        break;
    case PixelFormat::rgba8:
        if (!options.allow_rgba_drop_alpha) {
            return std::unexpected(make_populate_error(ImgPopulateErrorCode::unsupported_pixel_format, "RGBA bitmap conversion is disabled").status);
        }
        out_img.pixel_subtype = 2;
        out_img.bytes_per_pixel = 24;
        expected_row_stride = static_cast<size_t>(bitmap.width) * 4;
        break;
    case PixelFormat::index8:
        if (bitmap.palette.entry_count == 0 || bitmap.palette.rgb.size() != bitmap.palette.entry_count * 3) {
            return std::unexpected(make_populate_error(ImgPopulateErrorCode::invalid_bitmap, "Indexed bitmap is missing palette data").status);
        }
        if (options.force_rgb_from_indexed) {
            out_img.pixel_subtype = 2;
            out_img.bytes_per_pixel = 24;
            expected_row_stride = static_cast<size_t>(bitmap.width);
        } else {
            out_img.pixel_subtype = 1;
            out_img.bytes_per_pixel = 8;
            expected_row_stride = static_cast<size_t>(bitmap.width);
            out_img.cmsiz = 3;
            out_img.cmlen = static_cast<unsigned>(bitmap.palette.entry_count);
            out_img.cmap = static_cast<unsigned char*>(polyray_malloc(bitmap.palette.rgb.size()));
            if (out_img.cmap == nullptr) {
                return std::unexpected(make_populate_error(ImgPopulateErrorCode::out_of_memory, "Failed to allocate Img palette").status);
            }
            std::memcpy(out_img.cmap, bitmap.palette.rgb.data(), bitmap.palette.rgb.size());
        }
        break;
    }

    if (bitmap.row_stride_bytes < expected_row_stride) {
        if (out_img.cmap != nullptr) {
            polyray_free(out_img.cmap);
            out_img.cmap = nullptr;
        }
        return std::unexpected(make_populate_error(ImgPopulateErrorCode::invalid_bitmap, "Bitmap row stride is too small").status);
    }

    ImgPopulateResult rows_result = allocate_img_rows(out_img, bitmap.width, bitmap.height,
        (bitmap.pixel_format == PixelFormat::rgba8 || (bitmap.pixel_format == PixelFormat::index8 && options.force_rgb_from_indexed))
            ? static_cast<size_t>(bitmap.width) * 3
            : (bitmap.pixel_format == PixelFormat::rgb8 ? static_cast<size_t>(bitmap.width) * 3 : static_cast<size_t>(bitmap.width)));
    if (rows_result.status.code != ImgPopulateErrorCode::none) {
        if (out_img.cmap != nullptr) {
            polyray_free(out_img.cmap);
            out_img.cmap = nullptr;
        }
        return std::unexpected(rows_result.status);
    }

    for (uint32_t row = 0; row < bitmap.height; ++row) {
        const unsigned char* src = bitmap.pixels.data() + static_cast<size_t>(row) * bitmap.row_stride_bytes;
        unsigned char* dst = out_img.image[row];

        if (bitmap.pixel_format == PixelFormat::rgba8) {
            for (uint32_t col = 0; col < bitmap.width; ++col) {
                dst[col * 3 + 0] = src[col * 4 + 0];
                dst[col * 3 + 1] = src[col * 4 + 1];
                dst[col * 3 + 2] = src[col * 4 + 2];
            }
        } else if (bitmap.pixel_format == PixelFormat::index8 && options.force_rgb_from_indexed) {
            for (uint32_t col = 0; col < bitmap.width; ++col) {
                if (static_cast<size_t>(src[col]) >= bitmap.palette.entry_count) {
                    for (uint32_t cleanup = 0; cleanup < bitmap.height; ++cleanup) {
                        polyray_free(out_img.image[cleanup]);
                    }
                    polyray_free(out_img.image);
                    out_img.image = nullptr;
                    out_img.width = 0;
                    out_img.length = 0;
                    return std::unexpected(make_populate_error(ImgPopulateErrorCode::invalid_bitmap,
                        "Indexed bitmap references palette entries out of range").status);
                }
                const size_t palette_index = static_cast<size_t>(src[col]) * 3;
                dst[col * 3 + 0] = bitmap.palette.rgb[palette_index + 0];
                dst[col * 3 + 1] = bitmap.palette.rgb[palette_index + 1];
                dst[col * 3 + 2] = bitmap.palette.rgb[palette_index + 2];
            }
        } else {
            std::memcpy(dst, src, expected_row_stride);
        }
    }

    return {};
}

/** @brief Decode an image source directly into a legacy @c Img using the new interface.
 *  @param source Unified image source describing a path, @c FILE* handle, or memory buffer.
 *  @param out_img Legacy image structure to populate.
 *  @param decode_options Decode options controlling source interpretation and format selection.
 *  @param populate_options Population options controlling conversion into legacy storage.
 *  @return true on success, or false if decode or population fails.
 */
bool decode_image_into_img(const ImageSource& source,
                          Img* out_img,
                          const DecodeOptions& decode_options,
                          const ImgPopulateOptions& populate_options)
{
    if (out_img == nullptr) {
        return false;
    }

    auto decoded = decode_image(source, decode_options);
    if (!decoded) {
        return false;
    }

    auto populated = populate_img_from_bitmap(*decoded, *out_img, populate_options);
    return populated.has_value();
}
