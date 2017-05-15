#ifndef BLUR_OMP_IMAGE_UTILS_H
#define BLUR_OMP_IMAGE_UTILS_H

struct _Image {
    float *raw_data;
    float **data;
    unsigned width;
    unsigned height;
};

typedef struct _Image Image;

Image *read_png(const char *path);
Image *create_empty_image(unsigned width, unsigned height);
void save_image(const Image *image, const char *path);
void destroy_image(Image **image_ptr);

#endif
