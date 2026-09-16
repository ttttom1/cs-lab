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
    // TODO 1: 현재 파일 크기를 구한다.
    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size < 0) {
        perror("lseek");
        return -1;
    }

    // TODO 2: 현재 파일 크기로 새 page_id 계산
    int page_id = (int)(file_size / PAGE_SIZE);

    // TODO 3: 4096 bytes짜리 빈 Page 준비
    char empty_page[PAGE_SIZE] = {0};

    // TODO 4: 새 Page를 파일 끝에 기록
    off_t offset = (off_t)page_id * PAGE_SIZE;
    ssize_t written = pwrite(fd, empty_page, PAGE_SIZE, offset);

    // TODO 5: 기록된 크기가 PAGE_SIZE인지 검사
    if (written != PAGE_SIZE) {
        perror("pwrite");
        return -1;
    }

    // TODO 6: 성공하면 새 page_id 반환
    return page_id;
}