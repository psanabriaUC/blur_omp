#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include "image_utils.h"

Image *read_png(const char *path) {
    FILE *file;
    Image *image;
    png_byte header[8];
    png_structp png_ptr;
    png_infop info;
    png_bytep *rows;
    size_t i, j;

    file = fopen(path, "rb");
    if (file == NULL) {
        printf("Can't open file %s\n", path);
        return NULL;
    }
    if (fread(header, sizeof(png_byte), 8, file) < 8) {
        printf("IO Error while reading PNG\n");
        return NULL;
    }
    if (png_sig_cmp(header, 0, 8) != 0) {
        printf("Invalid PNG file\n");
        return NULL;
    }

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (png_ptr == NULL) {
        printf("Error initialization of PNG structure");
        fclose(file);
        return NULL;
    }

    info = png_create_info_struct(png_ptr);

    if (info == NULL) {
        printf("Error initialization of PNG structure");
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(file);
        return NULL;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        printf("Error while reading PNG file\n");
        png_destroy_read_struct(&png_ptr, &info, NULL);
        fclose(file);
        return NULL;
    }

    png_init_io(png_ptr, file);
    png_set_sig_bytes(png_ptr, 8);
    png_read_png(png_ptr, info, PNG_TRANSFORM_STRIP_ALPHA |
                                PNG_TRANSFORM_STRIP_16 |
                                PNG_TRANSFORM_PACKING |
                                PNG_TRANSFORM_EXPAND, NULL);
    image = malloc(sizeof(Image));

    image->width = png_get_image_width(png_ptr, info);
    image->height = png_get_image_height(png_ptr, info);
    rows = png_get_rows(png_ptr, info);
    image->data = calloc(image->height, sizeof(float *));
    image->raw_data = calloc(image->height * image->width * 3, sizeof(float));

    for (i = 0; i < image->height; i++) {
        for (j = 0; j < image->width; j++) {
            image->raw_data[3 * (i * image->width + j)] = (float)rows[i][3 * j] / 255;
            image->raw_data[3 * (i * image->width + j) + 1] = (float)rows[i][3 * j + 1] / 255;
            image->raw_data[3 * (i * image->width + j) + 2] = (float)rows[i][3 * j + 2] / 255;
        }
    }

    for (i = 0; i < image->height; i++) {
        image->data[i] = image->raw_data + 3 * i * image->width;
    }

    png_destroy_read_struct(&png_ptr, &info, NULL);
    fclose(file);

    return image;
}

Image *create_empty_image(unsigned width, unsigned height) {
    Image *image;
    unsigned i;

    image = malloc(sizeof(Image));

    image->width = width;
    image->height = height;

    image->data = calloc(image->height, sizeof(float *));
    image->raw_data = calloc(image->height * image->width * 3, sizeof(float));
    for (i = 0; i < image->height; i++) {
        image->data[i] = image->raw_data + 3 * i * image->width;
    }

    return image;
}

void save_image(const Image *image, const char *path) {
    FILE *file;
    png_structp png_ptr;
    png_infop info_ptr;
    png_text image_title;
    png_bytep png_row_buffer = NULL;
    size_t i, j;

    file = fopen(path, "wb");

    if (file == NULL) {
        printf("Can't open file %s\n", path);
        return;
    }

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (png_ptr == NULL) {
        printf("Can't open png write struct\n");
        return;
    }

    info_ptr = png_create_info_struct(png_ptr);

    if (info_ptr == NULL) {
        printf("Can't create png struct\n");
        return;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        printf("Error while writing PNG file\n");
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(file);
        return;
    }

    png_init_io(png_ptr, file);
    png_set_IHDR(png_ptr, info_ptr, image->width, image->height, 8,
                 PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE,
                 PNG_FILTER_TYPE_BASE);
    image_title.compression = PNG_TEXT_COMPRESSION_NONE;
    image_title.key = "Title";
    image_title.text = "Rendered image powered by ImageUtil";
    png_set_text(png_ptr, info_ptr, &image_title, 1);
    png_write_info(png_ptr, info_ptr);

    png_row_buffer = malloc(3 * image->width * sizeof(png_byte));

    for (i = 0; i < image->height; i++) {
        for (j = 0; j < image->width; j++) {
            png_row_buffer[3 * j] = (png_byte)(255 * image->data[i][3 * j]);
            png_row_buffer[3 * j + 1] = (png_byte)(255 * image->data[i][3 * j + 1]);
            png_row_buffer[3 * j + 2] = (png_byte)(255 * image->data[i][3 * j + 2]);
        }
        png_write_row(png_ptr, png_row_buffer);
    }

    free(png_row_buffer);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(file);
}

void destroy_image(Image **image_ptr) {
    if (*image_ptr == NULL)
        return;
    free((*image_ptr)->raw_data);
    free((*image_ptr)->data);
    free(*image_ptr);
    *image_ptr = NULL;
}
