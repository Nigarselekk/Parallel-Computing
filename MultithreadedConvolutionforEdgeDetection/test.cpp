#include <opencv2/opencv.hpp>
#include <pthread.h>
#include <iostream>
#include <vector>
#include <chrono>

#define MAX_THREADS 32  // Maksimum number of threads

using namespace cv;
using namespace std;

// Sobel X filter matrix (for edge detection)
int sobelX[3][3] = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}
};

// Data structure to be assigned per thread
struct ThreadData {
    int start_row, end_row; // Line spacing to process
    Mat *input_img; // Input image
    Mat *output_img; // Output image
};


// Function that applies the Sobel X filter on the specified line range (Thread function)
void* applySobelXFilterForRow(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int start_row = data->start_row;
    int end_row = data->end_row;
    Mat* input_img = data->input_img;
    Mat* output_img = data->output_img;

    // Filtering process on the image
    for (int i = start_row; i < end_row; i++) {
        for (int j = 1; j < input_img->cols - 1; j++) {
            int sum = 0;
            // Apply the 3x3 Sobel X filter
            for (int k = -1; k <= 1; k++) {
                for (int l = -1; l <= 1; l++) {
                    int y = i + k;
                    int x = j + l;
                      // If the pixel is within the image boundary, process it
                    if (y >= 0 && y < input_img->rows && x >= 0 && x < input_img->cols) {
                        sum += input_img->at<uchar>(y, x) * sobelX[k + 1][l + 1];
                    }
                }
            }
            // Convert the result to an absolute value and assign it to the appropriate range
            output_img->at<uchar>(i, j) = saturate_cast<uchar>(abs(sum));
        }
    }
    return NULL;
}

// Function to apply Sobel X filter using multiple threads
// This function divides the image into sections and processes each section in a separate thread
void applySobelXFilterMultithreaded(Mat* input_img, Mat* output_img, int num_threads) {
    num_threads = min(num_threads, input_img->rows); //Do not open more threads than the maximum number of lines
    vector<pthread_t> threads(num_threads); // Thread array
    vector<ThreadData> thread_data(num_threads);// Data per thread

    int rows_per_thread = input_img->rows / num_threads; // Number of lines to be assigned to each thread

    // Assign row ranges to each thread
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].start_row = i * rows_per_thread;
        thread_data[i].end_row = (i == num_threads - 1) ? input_img->rows : (i + 1) * rows_per_thread;
        thread_data[i].input_img = input_img;
        thread_data[i].output_img = output_img;

        pthread_create(&threads[i], NULL, applySobelXFilterForRow, (void*)&thread_data[i]);
    }
    // Wait for threads to complete
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}

int main() {
    // Image files to be processed
    const char* images[] = {"1MP.jpg", "2MP.jpg", "4MP.jpg", "8MP.jpg", "16MP.jpg", "32MP.jpg"};
    int num_images = sizeof(images) / sizeof(images[0]);
    int thread_options[] = {1, 2, 4, 8, 16, 32}; // Number of threads to use

    // Process for each image
    for (int i = 0; i < num_images; i++) {
        cout << "Processing: " << images[i] << endl;
        Mat input = imread(images[i], IMREAD_GRAYSCALE);
        if (input.empty()) {
            cerr << "Error loading image: " << images[i] << endl;
            continue;
        }

        Mat output = Mat::zeros(input.size(), CV_8UC1);

        // Single thread operation (sequential operation)
        auto start_seq = chrono::high_resolution_clock::now();
        applySobelXFilterForRow(new ThreadData{0, input.rows, &input, &output});
        auto end_seq = chrono::high_resolution_clock::now();
        chrono::duration<double> duration_seq = end_seq - start_seq;
        cout << "Sequential Execution Time: " << duration_seq.count() << " seconds" << endl;

        // Parallel processing (with different thread numbers)
        for (int t = 0; t < 6; t++) {
            int num_threads = thread_options[t]; 
            output = Mat::zeros(input.size(), CV_8UC1);
            auto start = chrono::high_resolution_clock::now();
            applySobelXFilterMultithreaded(&input, &output, num_threads);
            auto end = chrono::high_resolution_clock::now();
            chrono::duration<double> duration = end - start;
            cout << "Threads: " << num_threads << " - Execution Time: " << duration.count() << " seconds" << endl;
        }
    
        // Save result image
        string output_filename = string(images[i]).substr(0, string(images[i]).find(".")) + "-out.jpg";
        imwrite(output_filename, output);
        cout << "Saved: " << output_filename << endl;
    }
    return 0;
}



















