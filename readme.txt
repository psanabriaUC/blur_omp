Requirements:
- CMake
- libpng
- OpenMP compiler support

Compilation:
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make


Usage:
$ ./blur_omp image.png kernel_size iterations
