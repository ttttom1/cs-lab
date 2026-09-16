#include "disk_manager.h"

#include <stdio.h>
#include <string.h>

int main(void) {

    int fd = db_open("database.db");

    if (fd < 0) {
        return 1;
    }

    int page_id = page_allocate(fd);

    printf("allocated page =  %d\n", page_id);

    char write_page[PAGE_SIZE] = {0};

    strcpy(write_page, "My first allocated page");

    if (page_write(fd, page_id, write_page) < 0) {
        printf("page_write failed\n");
        return 1;
    }

    char read_page[PAGE_SIZE] ={ 0 };

    if (page_read(fd, page_id, read_page) < 0) {
        printf("page_read failed\n");
        return 1;
    }

    printf("read = %s\n", read_page);

    db_close(fd);

    return 0;
}