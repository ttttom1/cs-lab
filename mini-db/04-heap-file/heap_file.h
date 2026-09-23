#ifndef HEAP_FILE_H
#define HEAP_FILE_H

#include <stdint.h>

typedef struct {
    uint32_t page_id;
    uint16_t slot_id;
} RID;

typedef struct {
    int fd;
} HeapFile;

int  heap_open(
    HeapFile  *heap,
    const char *filename
);

void heap_close(
    HeapFile  *heap
);

int heap_insert(
    HeapFile *heap,
    const  void  *record,
    uint16_t length,
    RID rid
);

int heap_get(
    HeapFile *heap,
    RID rid,
    void  *buffer,
    uint16_t buffer_size
);

int heap_delete(
    HeapFile  *heap,
    RID rid
);

#endif