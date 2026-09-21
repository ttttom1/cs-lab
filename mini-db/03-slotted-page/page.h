#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>

#define PAGE_SIZE 4096

typedef struct {
    uint16_t slot_count;
    uint16_t free_start;
    uint16_t free_end;
} PageHeader;

typedef struct {
    uint16_t offset;
    uint16_t length;
} Slot;


void page_init(void *page);

int page_insert(
    void *page,
    const void *record,
    uint16_t length
);

int page_get(
    const void *page,
    uint16_t slot_id,
    void *buffer,
    uint16_t buffer_size
);

#endif