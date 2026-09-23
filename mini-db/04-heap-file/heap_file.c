#include "heap_file.h"
#include "page.h"
#include "disk_manager.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

int heap_insert(
    HeapFile *heap,
    const void *record,
    uint16_t length,
    RID *rid
) {
    // 1. 기본 Null Pointer 및 유효성 검사
    if (!heap || !record || !rid) {
        return -1;
    }

    // 2. [Invariant 선검사] 단일 레코드가 단일 페이지의 최대를 벗어남
    uint32_t max_record_size = PAGE_SIZE - sizeof(PageHeader) - sizeof(Slot);
    if (length > max_record_size) {
        return -1;
    }
    /*
     * TODO 1
     * 현재 database.db에 Page가 몇 개 있는지 알아낸다.
     */
    uint32_t page_count = heap_page_count(heap);

    char page[PAGE_SIZE];

    /*
     * TODO 2
     *
     * 기존 모든 Page를 순회한다.
     *
     * page_read()
     *      ↓
     * page_insert()
     *
     * 성공하면:
     * page_write()
     * RID 설정
     * return 0
     */
    for (uint32_t page_id = 0; page_id < page_count; page_id++) {
        // 디스크에서 페이지 읽기
        if (page_read(heap->fd, page_id, page) != 0) {
            return -1;
        }

        //해당 페이지에 레코드 삽입 시도
        int slot_id = page_insert(page, record,  length);
        if (slot_id == -1) {
            continue;
        }

        if (page_write(heap->fd, page_id, page) != 0) {
            return -1;
        }

        rid->page_id = page_id;
        rid->slot_id = (uint16_t)slot_id;
        return 0;
    }

    /*
     * TODO 3
     *
     * 모든 Page가 꽉 찼다면
     *
     * page_allocate()
     * page_init()
     * page_insert()
     * page_write()
     *
     * RID 설정
     */
    int new_page_id = page_allocate(heap->fd);
    if (new_page_id < 0) {
        return -1;
    }

    page_init(page);
    int slot_id = page_insert(page, record,length);
    if (slot_id == -1) {
        return -1;
    }

    if(page_write(heap->fd, (uint32_t)new_page_id, page)) {
        return -1;
    }

    rid->page_id = (uint32_t)new_page_id;
    rid->slot_id = (uint16_t)slot_id;

    return 0;
}

int heap_get(
    HeapFile *heap,
    RID rid,
    void  *buffer,
    uint16_t buffer_size
) {
    // 1. 유효성 검사
    if (!heap || !buffer || buffer_size == 0) {
        return -1;
    }

    // 2. RID 의 page_id가 현재 파일의 총 페이지 수 범위를 벗어나는지  검사
    uint32_t page_count = heap_page_count(heap);
    if  (rid.page_id >= page_count) {
        return -1;
    }

    char page[PAGE_SIZE];

    //3. 디스크에서 해당 page_id 페이지 읽기
    if (page_read(heap->fd, rid.page_id,  page) != 0) {
        return -1;
    }

    // 4. Slotted Page 내부에서 slot_id 가져오기
    int record_length = page_get(page,rid.slot_id, buffer, buffer_size);
    if (record_length < 0) {
        return -1;
    }
    return  record_length;
}

int heap_delete(
    HeapFile  *heap,
    RID rid
) {
    if (!heap  || heap->fd < 0) {
        return -1;
    }

    uint32_t page_count = heap_page_count(heap);
    if (rid.page_id >= page_count) {
        return  -1;
    }

    char page[PAGE_SIZE];

    if (page_read(heap->fd, rid.page_id,page) != 0) {
        return  -1;
    }

    if (page_delete(page,rid.slot_id) !=  0) {
        return -1;
    }

    if (page_write(heap->fd, rid.page_id,page) != 0) {
        return -1;
    }

    return 0;
}