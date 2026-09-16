#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H

#define PAGE_SIZE 4096

int db_open(const char *filename);

void db_close(int fd);

int page_write(
    int fd,
    int page_id,
    const void *buffer
);

int page_read(
    int fd,
    int page_id,
    void *buffer
);

int page_allocate(int fd);

#endif