CC = gcc
CFLAGS = -Wall -Wextra -pthread
LIBS = -lcurl -lpthread
TARGET = downloader
SRC = downloader.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET) *.part*