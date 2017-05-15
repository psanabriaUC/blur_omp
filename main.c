#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <omp.h>
#include "image_utils.h"

void sequential_execution(const char *input_path, const char *output_path, unsigned int kernel_size,
                          unsigned int iterations);
void parallel_execution(const char *input_path, const char *output_path, unsigned int kernel_size,
                        unsigned int iterations, int threads);
void apply_convolution(float **input_image, float **output_image, unsigned width, unsigned height,
                       unsigned int kernel_size, unsigned i, unsigned j);

int main(int argc, char *argv[]) {
    int i, max_threads;
    unsigned kernel_size, iterations;
    double start, end, t1;
    char *filename;

    if (argc < 4) {
        printf("Usage: blur_omp image.png kernel_size iterations\n");
        return 0;
    }

    filename = argv[1];
    kernel_size = (unsigned)atoi(argv[2]);
    iterations = (unsigned)atoi(argv[3]);

    start = omp_get_wtime();
    sequential_execution(filename, "out.png", kernel_size, iterations);
    end = omp_get_wtime();
    t1 = end - start;
    printf("Sequential execution: %f secs.\n", t1);
    printf("-----------------------------\n");

    max_threads = 2 * omp_get_max_threads();

    printf("Parallel form 1\n");
    for (i = 1; i <= max_threads; i++) {
        start = omp_get_wtime();
        parallel_execution(filename, "out_parallel.png", kernel_size, iterations, i);
        end = omp_get_wtime();
        printf("Parallel execution with %d threads: %0.4f secs. Speedup: %0.4f\n", i, end - start, t1 / (end - start));
    }

    return 0;
}

void sequential_execution(const char *input_path, const char *output_path, unsigned int kernel_size,
                          unsigned int iterations) {
    Image *image = read_png(input_path);
    Image *output_image = create_empty_image(image->width, image->height);
    unsigned i, j, c;

    for (c = 0; c < iterations; c++) {
        memset(output_image->raw_data, 0, output_image->height * output_image->width * 3 * sizeof(float));
        for (i = 0; i < image->height; i++) {
            for (j = 0; j < image->width; j++) {
                apply_convolution(image->data, output_image->data, output_image->width, output_image->height, kernel_size, i, j);
            }
        }
        memcpy(image->raw_data, output_image->raw_data, output_image->height * output_image->width * 3 * sizeof(float));
    }
    save_image(output_image, output_path);

    destroy_image(&image);
    destroy_image(&output_image);
}

void parallel_execution(const char *input_path, const char *output_path, unsigned int kernel_size,
                        unsigned int iterations, int threads) {
    Image *image = read_png(input_path);
    Image *output_image = create_empty_image(image->width, image->height);
    unsigned i, j, c;

    for (c = 0; c < iterations; c++) {
        memset(output_image->raw_data, 0, output_image->height * output_image->width * 3 * sizeof(float));

        #pragma omp parallel for schedule(guided) private(i, j) num_threads(threads)
        for (i = 0; i < image->height; i++) {
            for (j = 0; j < image->width; j++) {
                apply_convolution(image->data, output_image->data, output_image->width, output_image->height, kernel_size, i, j);
            }
        }

        memcpy(image->raw_data, output_image->raw_data, output_image->height * output_image->width * 3 * sizeof(float));
    }
    save_image(output_image, output_path);

    destroy_image(&image);
    destroy_image(&output_image);
}

void apply_convolution(float **input_image, float **output_image, unsigned width, unsigned height,
                       unsigned int kernel_size, unsigned i, unsigned j) {
    unsigned p, q, h, w;

    for (p = 0; p < kernel_size; p++) {
        for (q = 0; q < kernel_size; q++) {
            h = (i + (p - kernel_size / 2)) % height;
            w = (j + (q - kernel_size / 2)) % width;
            output_image[i][3 * j] += input_image[h][3 * w] / (kernel_size * kernel_size);
            output_image[i][3 * j + 1] += input_image[h][3 * w + 1] / (kernel_size * kernel_size);
            output_image[i][3 * j + 2] += input_image[h][3 * w + 2] / (kernel_size * kernel_size);
        }
    }
}
