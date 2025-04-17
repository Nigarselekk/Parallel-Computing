#  Multithreaded Convolution for Edge Detection

## Overview
This program applies the Sobel X filter to images using multithreading to improve performance. It processes grayscale images and measures execution time for both sequential and parallel implementations with varying thread counts.

## Prerequisites
Ensure you have the following dependencies installed:
- g++ (GCC)
- OpenCV 4
- pthread library

### Installing OpenCV (if not already installed)
#### macOS (using Homebrew):
```sh
brew install opencv
```
#### Ubuntu (using APT):
```sh
sudo apt update
sudo apt install libopencv-dev
```

## Compilation
To compile the program in **release mode**, use the following command:
```sh
g++ -std=c++17 $(pkg-config --cflags --libs opencv4) test.cpp -o test
```

## Running the Program
After successful compilation, run the executable with:
```sh
./test
```

## Expected Output
The program will process a set of images and display execution times for different thread counts. The filtered images will be saved with "-out.jpg" appended to the original filename.

Example output:
```
Processing: 1MP.jpg
Sequential Execution Time: 0.45 seconds
Threads: 1 - Execution Time: 0.45 seconds
Threads: 2 - Execution Time: 0.30 seconds
Threads: 4 - Execution Time: 0.20 seconds
Threads: 8 - Execution Time: 0.15 seconds
Saved: 1MP-out.jpg
```

## Notes
- Ensure the test images are in the same directory as the executable.
- The program automatically scales the number of threads up to a maximum of 32 or the number of available image rows.
- Processing time varies depending on image size and system hardware.



