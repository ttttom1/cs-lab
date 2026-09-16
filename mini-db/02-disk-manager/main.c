#include "disk_manager.h"

#include <stdio.h>
#include <string.h>

int main(void) {

    int fd = db_open("database.db");

    if (fd < 0) {
        return 1;
    }

    char write_page[PAGE_SIZE];
    memset(write_page, 0, PAGE_SIZE);

    strcpy(write_page, "Hello Disk Manager");

    if (page_write(fd, 3, write_page) < 0) {
        printf("page_write failed\n");
        return 1;
    }

    char read_page[PAGE_SIZE];
    memset(read_page, 0, PAGE_SIZE);

    if (page_read(fd, 3, read_page) < 0) {
        printf("page_read failed\n");
        return 1;
    }

    printf("read = %s\n", read_page);

    db_close(fd);

    return 0;
}