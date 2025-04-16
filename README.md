#  Multithreaded Downloader in C

This project implements a multithreaded download manager in C. It downloads a file from a given URL using multiple threads to improve performance and then merges the downloaded chunks into a single file.


##  Features

- Multi-threaded file download
- HTTP/HTTPS support using libcurl
- File splitting and merging
- Command-line arguments for customization
- Basic error handling



## 🛠️  Requirements

- GCC or Clang compiler
- `libcurl` development library
- POSIX threads (`pthread`)

### 📦 Install Dependencies (Linux)

```bash
sudo apt update
sudo apt install libcurl4-openssl-dev build-essential
```

### Compilation
Compile the source code with:

```bash
gcc -o downloader downloader.c -lcurl -lpthread
```

### 📥 Usage
```bash
./downloader <URL> <output_file> <num_threads>
```

### ⚙️ How It Works
- The program gets the file size using a HEAD request.

- It calculates byte ranges and assigns them to threads.

- Each thread downloads its assigned part into a .partX file.

- All parts are merged into the final output file.


### License
This project is for educational purposes only. No specific license applied.





