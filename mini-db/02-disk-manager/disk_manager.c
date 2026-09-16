#include "disk_manager.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int db_open(const char *filename) {
    int fd = open(filename,O_RDWR  | O_CREAT, 0644);
    if (fd < 0) {
        perror("open");
        return  -1;
    }
    return fd;
}

void db_close(int fd) {
    if (fd >= 0) {
        close(fd);
    }
}

int page_write(
    int fd,
    int page_id,
    const void *buffer
) {
    off_t offset = (off_t) page_id * PAGE_SIZE;

    ssize_t written = pwrite(fd, buffer,PAGE_SIZE, offset);

    if (written !=  PAGE_SIZE) {
        perror("pwrite");
        return -1;
    }
    return 0;
}

int page_read(
    int fd,
    int page_id,
    void *buffer
) {
    off_t offset = (off_t) page_id *PAGE_SIZE;

    ssize_t bytes = pread(fd, buffer, PAGE_SIZE,offset);

    if (bytes !=PAGE_SIZE) {
        perror("pread");
        return -1;
    }

    return 0;
}

int page_allocate(int fd) {
    //일단 비워둔다.
    (void)  fd;
    return -1;
}