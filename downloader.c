#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <curl/curl.h>
#include <string.h>

#define CHUNK_SIZE 8192  // Buffer size for downloading

// Data structure to pass arguments to the download_chunk function
typedef struct {
    char url[1024];
    char output_file[256];
    long start;
    long end;
    int thread_id;
} ThreadData;

// Function to download a specific range of bytes
void *download_chunk(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize cURL\n");
        return NULL;
    }

    // Open file for writing
    FILE *fp;
    char part_filename[256];
    snprintf(part_filename, sizeof(part_filename), "%s.part%d", data->output_file, data->thread_id);
    fp = fopen(part_filename, "wb");
    if (!fp) {
        fprintf(stderr, "Failed to open file for writing: %s\n", part_filename);
        return NULL;
    }

    // Set range header
    char range_header[64];
    snprintf(range_header, sizeof(range_header), "%ld-%ld", data->start, data->end);

    // Set cURL options
    curl_easy_setopt(curl, CURLOPT_URL, data->url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_RANGE, range_header);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // Perform the download
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "Download failed for chunk %d: %s\n", data->thread_id, curl_easy_strerror(res));
    }
   // Close file and cleanup
    fclose(fp);
    curl_easy_cleanup(curl);
    return NULL;
}

// Function to get file size
long get_file_size(const char *url) {
    CURL *curl = curl_easy_init();
    if (!curl) return -1;

    // Perform a HEAD request to get the file size
    double filesize = -1.0;
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_HEADER, 1L);

    // Perform the request
    if (curl_easy_perform(curl) == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &filesize);
    }

    // Cleanup
    curl_easy_cleanup(curl);
    return (long)filesize;
}

// Function to merge all downloaded chunks
void merge_files(const char *filename, int num_threads) {
    FILE *final_fp = fopen(filename, "wb");
    if (!final_fp) {
        fprintf(stderr, "Failed to create final file: %s\n", filename);
        return;
    }

    // Merge all part files
    for (int i = 0; i < num_threads; i++) {
        char part_filename[256];
        snprintf(part_filename, sizeof(part_filename), "%s.part%d", filename, i);

        FILE *part_fp = fopen(part_filename, "rb");
        if (!part_fp) {
            fprintf(stderr, "Failed to open part file: %s\n", part_filename);
            continue;
        }

        char buffer[CHUNK_SIZE];
        size_t bytes_read;
        while ((bytes_read = fread(buffer, 1, CHUNK_SIZE, part_fp)) > 0) {
            fwrite(buffer, 1, bytes_read, final_fp);
        }

        fclose(part_fp);
        remove(part_filename);  // Delete part file after merging
    }

    fclose(final_fp);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <URL> <output_file> <num_threads>\n", argv[0]);
        return 1;
    }

    char *url = argv[1];
    char *output_file = argv[2];
    int num_threads = atoi(argv[3]);

    if (num_threads < 1) {
        printf("Number of threads must be at least 1\n");
        return 1;
    }

    long file_size = get_file_size(url);
    if (file_size <= 0) {
        printf("Failed to get file size. Ensure the URL is correct.\n");
        return 1;
    }

    printf("File size: %ld bytes\n", file_size);

    // Create threads for downloading
    pthread_t threads[num_threads]; // Array of thread IDs
    ThreadData thread_data[num_threads]; // Array of thread data

    // Calculate the chunk size for each thread
    long chunk_size = file_size / num_threads;
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].start = i * chunk_size;
        thread_data[i].end = (i == num_threads - 1) ? file_size - 1 : (thread_data[i].start + chunk_size - 1);
        strcpy(thread_data[i].url, url);
        strcpy(thread_data[i].output_file, output_file);
        thread_data[i].thread_id = i;

        // Create a thread for downloading a chunk
        pthread_create(&threads[i], NULL, download_chunk, &thread_data[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Merge all downloaded chunks
    merge_files(output_file, num_threads);
    printf("Download completed: %s\n", output_file);
    return 0;
}
