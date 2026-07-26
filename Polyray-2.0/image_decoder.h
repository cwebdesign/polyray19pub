// image_decoder.h

  #ifndef IMAGE_DECODER_H
  #define IMAGE_DECODER_H

  #include <cstddef>
  #include <cstdint>
  #include <cstdio>
  #include <optional>
  #include <string>
  #include <vector>
  #include <expected>

  #include "defs3.h"

  enum class ImageFormat {
      unknown,
      jpeg,
      gif,
      png,
      tga,
      bmp,
      pbm
  };

  enum class PixelFormat {
      grey8,
      rgb8,
      rgba8,
      index8
  };

  enum class ImageOrigin {
      top_left,
      bottom_left
  };

  enum class DecodeErrorCode {
      none,
      unsupported_format,
      invalid_data,
      truncated_data,
      unsupported_feature,
      out_of_memory,
      io_error,
      internal_error
  };

  enum class SourceKind {
      file_path,
      file_handle,
      memory_buffer
  };

  enum class OwnershipMode {
      borrowed,
      owned
  };

  enum class PreferredOutput {
      native,
      grey8,
      rgb8
  };

  enum class ImgPopulateErrorCode {
      none,
      unsupported_pixel_format,
      unsupported_origin,
      out_of_memory,
      invalid_bitmap,
      internal_error
  };

  struct MemoryView {
      const unsigned char* data = nullptr;
      size_t size = 0;
  };

  struct PathImageSource {
      std::string path;
  };

  struct FileImageSource {
      FILE* file = nullptr;
      OwnershipMode ownership = OwnershipMode::borrowed;
      bool seekable = true;
  };

  struct MemoryImageSource {
      MemoryView view;
  };

  struct ImageSource {
      SourceKind kind = SourceKind::memory_buffer;
      std::string display_name;
      PathImageSource path;
      FileImageSource handle;
      MemoryImageSource memory;

      static ImageSource FromPath(std::string path_value,
                                  std::string display_name_value = "")
      {
          ImageSource source;
          source.kind = SourceKind::file_path;
          source.path.path = std::move(path_value);
          source.display_name = display_name_value.empty() ? source.path.path : std::move(display_name_value);
          return source;
      }

      static ImageSource FromFile(FILE* file_value,
                                  OwnershipMode ownership_value = OwnershipMode::borrowed,
                                  std::string display_name_value = "",
                                  bool seekable_value = true)
      {
          ImageSource source;
          source.kind = SourceKind::file_handle;
          source.display_name = std::move(display_name_value);
          source.handle.file = file_value;
          source.handle.ownership = ownership_value;
          source.handle.seekable = seekable_value;
          return source;
      }

      static ImageSource FromMemory(const unsigned char* data_value,
                                    size_t size_value,
                                    std::string display_name_value = "")
      {
          ImageSource source;
          source.kind = SourceKind::memory_buffer;
          source.display_name = std::move(display_name_value);
          source.memory.view.data = data_value;
          source.memory.view.size = size_value;
          return source;
      }
  };

  struct DecodeOptions {
      ImageFormat format_hint = ImageFormat::unknown;
      PreferredOutput preferred_output = PreferredOutput::native;
      bool strict_mode = false;
      bool allow_palette = true;
      bool collect_warnings = true;
  };

  struct DecodedPalette {
      std::vector<unsigned char> rgb;
      size_t entry_count = 0;
  };

  struct DecodedBitmap {
      uint32_t width = 0;
      uint32_t height = 0;
      PixelFormat pixel_format = PixelFormat::rgb8;
      ImageOrigin origin = ImageOrigin::top_left;
      size_t row_stride_bytes = 0;
      std::vector<unsigned char> pixels;
      DecodedPalette palette;
      ImageFormat source_format = ImageFormat::unknown;
  };

  struct DecodeWarning {
      std::string message;
  };

  struct DecodeStatus {
      DecodeErrorCode code = DecodeErrorCode::none;
      std::string message;
      std::vector<DecodeWarning> warnings;
  };


  struct ImgPopulateOptions {
      bool preserve_origin_flag = true;
      bool allow_rgba_drop_alpha = false;
      bool force_rgb_from_indexed = false;
  };

  struct ImgPopulateStatus {
      ImgPopulateErrorCode code = ImgPopulateErrorCode::none;
      std::string message;
  };

  struct ImgPopulateResult {
      ImgPopulateStatus status;
  };

  struct DecodeResult { //not needed anymore.
      DecodeStatus status;
      std::optional<DecodedBitmap> image;
  };

  std::expected<DecodedBitmap, DecodeStatus> decode_image(
      const ImageSource& source,
      const DecodeOptions& options = {}
  );

  std::expected<void, ImgPopulateStatus> populate_img_from_bitmap(
      const DecodedBitmap& bitmap,
      Img& out_img,
      const ImgPopulateOptions& options = {}
  );

  bool decode_image_into_img(
      const ImageSource& source,
      Img* out_img,
      const DecodeOptions& decode_options = {},
      const ImgPopulateOptions& populate_options = {}
  );

  #endif
