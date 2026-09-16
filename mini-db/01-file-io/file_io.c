#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define PAGE_SIZE 4096

void page_write(int fd, int page_id, const char *data) {
    off_t offset = (off_t) page_id * PAGE_SIZE;

    if(lseek(fd, offset,SEEK_SET) == -1) {
        perror("lseek");
        exit(1);
    }

    ssize_t written = write(fd, data, PAGE_SIZE);
    
    if (written != PAGE_SIZE) {
        perror("write");
        exit(1);
    }

    if (fsync(fd) == -1) {
        perror("fsync");
        exit(1);
    }
}

void page_read(int fd, int page_id, char *buffer) {
    off_t offset = (off_t) page_id * PAGE_SIZE;

    if (lseek(fd, offset, SEEK_SET) == -1) {
        perror("lseek");
        exit(1);
    } 

    ssize_t bytes = read(fd,buffer,PAGE_SIZE);

    if(bytes < 0) {
        perror("read");
        exit(1);
    }
}

int main(void) {
    int fd = open("database.db", O_RDWR | O_CREAT,0644);

    if (fd < 0) {
        perror("open");
        exit(1);
    }

    //page 하나 크기의 버퍼
    char write_buffer[PAGE_SIZE];
    memset(write_buffer, 0, PAGE_SIZE);

    strcpy(write_buffer, "Hello DB");

    page_write(fd, 2, write_buffer);

    close(fd);

    fd = open("database.db",O_RDWR);
    
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    char read_buffer[PAGE_SIZE];

    memset(read_buffer,0,PAGE_SIZE);
    page_read(fd, 2, read_buffer);

    printf("read: %s\n", read_buffer);

    close(fd);
    return 0;
}