/**
 * @file pngread.cc
 * @brief Read PNG files using Lodepng library.
 *
 * Support routines for reading png files
 *
  Polyray - MIT Licensed Revival
  Copyright (C) 1993-1996, Alexander Enzmann, All rights reserved.
  Copyright (C) 1999-2026, Clyde Meli, All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", (C), WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 */
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <vector>
#include "defs3.h"
#include "memory.h"
#include "io_ply.h"
//#include "jpeg.h"
#include "lodepng.h"


#include "pngread.h"


/* The final image after we are done processing the JPEG image */
static struct Img *output_image = nullptr;


/**
 * @brief Read the entire input stream into memory.
 *
 * @param fp Open file positioned anywhere in the stream.
 * @param buffer Destination byte buffer populated with the file contents.
 * @return Non-zero on success, zero if the file cannot be fully read.
 */
static int read_file_to_buffer(FILE *fp, std::vector<unsigned char> &buffer)
{
    if(fp == nullptr) return 0;

    if(std::fseek(fp, 0, SEEK_END) != 0) return 0;
    long size = std::ftell(fp);
    if(size < 0) return 0;
    if(std::fseek(fp, 0, SEEK_SET) != 0) return 0;

    buffer.resize(static_cast<size_t>(size));
    if(size == 0) return 0;

    size_t got = std::fread(buffer.data(), 1, static_cast<size_t>(size), fp);
    return got == static_cast<size_t>(size);
}

/**
 * @brief Copy the decoded PNG palette into Polyray's BGR palette layout.
 *
 * @param image Destination image metadata and palette storage.
 * @param color LodePNG color description that owns the RGBA palette entries.
 */
static void copy_palette_to_img(Img& image, const LodePNGColorMode& color)
{
    const size_t palette_size = color.palettesize;

    image.cmap = (unsigned char *)polyray_malloc(palette_size * 3u);
    if(image.cmap == nullptr)
        serror("Out of memory allocating PNG palette");

    image.cmlen = static_cast<unsigned>(palette_size);
    image.cmsiz = 3;

    for(size_t i = 0; i < palette_size; ++i)
    {
        const size_t src = i * 4u;
        const size_t dst = i * 3u;
        image.cmap[dst + 0u] = color.palette[src + 2u];
        image.cmap[dst + 1u] = color.palette[src + 1u];
        image.cmap[dst + 2u] = color.palette[src + 0u];
    }
}

/**
 * @brief Copy 8-bit palette indices row-by-row into the output image buffer.
 *
 * @param image Destination Polyray image.
 * @param decoded Indexed pixel rows returned by LodePNG.
 */
static void copy_indexed_rows(Img *image, const std::vector<unsigned char> &decoded)
{
    const size_t rowbytes = static_cast<size_t>(image->width);

    for(unsigned y = 0; y < image->length; ++y)
    {
        memcpy(image->image[y], decoded.data() + static_cast<size_t>(y) * rowbytes, rowbytes);
    }
}

/**
 * @brief Copy non-indexed rows into the output image, swapping RGB to BGR.
 *
 * @param image Destination Polyray image.
 * @param decoded Raw decoded bytes returned by LodePNG.
 * @param samples_per_pixel Number of output channels per pixel.
 * @param grayscale Non-zero when the decoded image is grayscale data.
 */
static void copy_direct_rows(Img *image, const std::vector<unsigned char> &decoded,
    unsigned samples_per_pixel, int grayscale)
{
    const size_t rowbytes = static_cast<size_t>(image->width) * samples_per_pixel;

    for(unsigned y = 0; y < image->length; ++y)
    {
        const unsigned char *src = decoded.data() + static_cast<size_t>(y) * rowbytes;
        unsigned char *dst = image->image[y];

        if(grayscale)
        {
            memcpy(dst, src, rowbytes);
            continue;
        }

        for(unsigned x = 0; x < image->width; ++x)
        {
            const size_t off = static_cast<size_t>(x) * samples_per_pixel;
            dst[off + 0u] = src[off + 2u];
            dst[off + 1u] = src[off + 1u];
            dst[off + 2u] = src[off + 0u];
            if(samples_per_pixel == 4u)
                dst[off + 3u] = src[off + 3u];
        }
    }
}

/**
 * @brief Allocate and initialize the row table used by a Polyray image.
 *
 * @param image Image to initialize.
 * @param width Image width in pixels.
 * @param height Image height in pixels.
 * @param samples_per_pixel Number of stored channels per pixel.
 * @param indexed Non-zero for palette-based images.
 */
static void init_img_rows(Img *image, unsigned width, unsigned height,
           unsigned samples_per_pixel, int indexed)
{
    image->copy = 0;
    image->width = static_cast<int>(width);
    image->length = static_cast<int>(height);
    image->scanline_order = 0x20;
    image->bytes_per_pixel = static_cast<int>(samples_per_pixel * 8);
    image->cflag = indexed ? 1 : 0;
    image->pixel_subtype = indexed ? 1 : (samples_per_pixel == 1 ? 3 : 2);
    image->cmsiz = 0;
    image->cmlen = 0;
    image->cmap = nullptr;

    image->image = (unsigned char **)polyray_malloc(height * sizeof(unsigned char *));
    if(image->image == nullptr) serror("Out of memory allocating image rows");

    size_t rowbytes = static_cast<size_t>(width) * samples_per_pixel;

    for(unsigned y = 0; y < height; ++y)
    {
        image->image[y] = (unsigned char *)polyray_malloc(rowbytes);
        if(image->image[y] == nullptr)
            serror("Out of memory allocating image row");
    }
}

// new PNG reader
//input already open ifile
//output structure img
/**
 * @brief Decode a PNG file into a Polyray image using LodePNG.
 *
 * @param ifile Open PNG stream.
 * @param img Destination image populated on success.
 * @return 1 on success, 0 on decode or I/O failure.
 */
int read_PNG_image(FILE *ifile, Img *img)
{
    unsigned width = 0, height = 0;
    lodepng::State state;
    std::vector<unsigned char> file_data;
    if(!read_file_to_buffer(ifile, file_data)) {
        return 0;
    }

    unsigned error = lodepng_inspect(&width, &height, &state,
        file_data.data(), file_data.size());
    if(error != 0) {
        return 0;
    }

    const auto& color = state.info_png.color;
    const bool indexed = (color.colortype == LCT_PALETTE);
    const bool grayscale =
        (color.colortype == LCT_GREY || color.colortype == LCT_GREY_ALPHA);
    const bool has_alpha =
        (color.colortype == LCT_GREY_ALPHA || color.colortype == LCT_RGBA);

    output_image = img;
    std::vector<unsigned char> decoded;

    state.decoder.color_convert = 1;

    if(indexed) {
        state.info_raw.colortype = LCT_PALETTE;
        state.info_raw.bitdepth = 8;
    } else if(grayscale) {
        state.info_raw.colortype = has_alpha ? LCT_GREY_ALPHA : LCT_GREY;
        state.info_raw.bitdepth = 8;
    } else {
        state.info_raw.colortype = has_alpha ? LCT_RGBA : LCT_RGB;
        state.info_raw.bitdepth = 8;
    }

    error = lodepng::decode(decoded, width, height, state, file_data);
    if(error != 0) {
        return 0;
    }

    if(indexed) {
        init_img_rows(img, width, height, 1, 1);
        copy_palette_to_img(*img, color);
        copy_indexed_rows(img, decoded);
    } else {
        const unsigned samples_per_pixel = grayscale ? (has_alpha ? 2u : 1u) : (has_alpha ? 4u : 3u);
        init_img_rows(img, width, height, samples_per_pixel, 0);
        copy_direct_rows(img, decoded, samples_per_pixel, grayscale ? 1 : 0);
    }

    return 1;
}


