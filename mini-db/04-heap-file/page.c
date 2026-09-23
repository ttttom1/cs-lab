#include "page.h"
#include <string.h>
#include <stdio.h>

void page_init(void *page) {
    if (!page) return;

    // 페이지 전체 0으로 초기화
    memset(page, 0, PAGE_SIZE);

    PageHeader *header = (PageHeader *)page;
    header->slot_count = 0;
    header->free_start = sizeof(PageHeader);
    header->free_end = PAGE_SIZE;
}

int page_insert(void *page, const void *record, uint16_t length) {
    if (!page || !record || length == 0) return -1;

    PageHeader *header = (PageHeader *)page;

    // 추가. 재사용 가능한 삭제된 슬롯이 있는지 확인
    int reusable_slot_id = -1;
    for (uint16_t i = 0; i <  header->slot_count; i++) {
        uint16_t slot_offset = PAGE_SIZE - (i + 1) * sizeof(Slot);
        Slot *s = (Slot *)((char *)page + slot_offset);
        if (s->length == 0) {
            reusable_slot_id = i;
            break;
        }
    }

    // 1. 필요한 여유 공간 계산 (Record 크기 + Slot 1개 크기)
    uint32_t needed_space = length + (reusable_slot_id != -1 ? 0 : sizeof(Slot));
    uint32_t current_free = header->free_end - header->free_start;

    // 공간 부족 시 실패
    if (current_free < needed_space) {
        page_compact(page);
        //recalculate space
        current_free = header->free_end - header->free_start;

        if (current_free < needed_space) {
            return -1;
        }
    }

    memcpy((char *)page + header->free_start, record, length);

    uint16_t slot_id;
    Slot *slot;

    if (reusable_slot_id != -1) {
        slot_id = (uint16_t)reusable_slot_id;
        uint16_t slot_offset = PAGE_SIZE - (slot_id + 1) * sizeof(Slot);
        slot = (Slot *)((char *) page + slot_offset);
    }  else {
        slot_id  = header->slot_count;
        uint16_t slot_offset = PAGE_SIZE - (slot_id + 1) * sizeof(Slot);
        slot = (Slot *)((char *)page + slot_offset);

        header->free_end -= sizeof(Slot);
        header->slot_count++;
    }

    slot->offset = header->free_start;
    slot->length = length;
    header->free_start += length;

    return (int)slot_id;
}

int page_get(const void *page, uint16_t slot_id, void *buffer, uint16_t buffer_size) {
    if (!page || !buffer) return -1;

    const PageHeader *header = (const PageHeader *)page;

    // 유효하지 않은 slot_id 검사
    if (slot_id >= header->slot_count) {
        return -1;
    }

    // 해당 Slot 위치 찾기
    uint16_t slot_offset = PAGE_SIZE - (slot_id + 1) * sizeof(Slot);
    const Slot *slot = (const Slot *)((const char *)page + slot_offset);

    if (slot->length == 0) {
        return -1;
    }

    // 버퍼 크기가 Record 데이터보다 작은 경우 실패
    if (buffer_size < slot->length) {
        return -1;
    }

    // Record 복사
    memcpy(buffer, (const char *)page + slot->offset, slot->length);

    return (int)slot->length;
}

int page_delete(void *page, uint16_t slot_id) {
    if (!page) return -1;

    PageHeader *header = (PageHeader *) page;

    if (slot_id >= header->slot_count) {
        return -1;
    }

    uint16_t slot_offset = PAGE_SIZE -  (slot_id  + 1) * sizeof(Slot);
    Slot *slot = (Slot *) ((char *)page + slot_offset);

    if (slot->length == 0) {
        return -1;
    }

    slot->length = 0;

    return 0;
}

void page_compact(void *page) {
    if (!page) return;

    PageHeader *header = (PageHeader *) page;
    uint16_t new_free_start = sizeof(PageHeader);

    for (uint16_t  slot_id = 0;  slot_id < header->slot_count; slot_id++) {
        uint16_t slot_offset = PAGE_SIZE -  (slot_id + 1) * sizeof(Slot);
        Slot *slot = (Slot *)((char *)page +  slot_offset);

        if (slot->length ==  0) {
            continue;
        }

        uint16_t old_offset = slot->offset;

        memmove((char *)page + new_free_start, (char *)page + old_offset, slot->length);

        slot->offset = new_free_start;

        new_free_start += slot->length;
    }
    header->free_start = new_free_start;
}

int page_read(int fd, uint32_t page_id, void *page) {
    if(fd < 0 || !page)  return -1;

    //page 오프셋
    off_t offset = (off_t)page_id * PAGE_SIZE;
    if (lseek(fd, offset, SEEK_SET) == -1) return -1;

    ssize_t bytes_read = read(fd, page, PAGE_SIZE);
    if (bytes_read != PAGE_SIZE) return -1;

    return 0;
}

int page_write(int fd, uint32_t page_id, const void *page) {
    if (fd < 0 || !page) return -1;

    off_t offset = (off_t)page_id * PAGE_SIZE;
    if (lseek(fd, offset, SEEK_SET) == -1)  ruturn -1;

    ssize_t bytes_written = write(fd, page, PAGE_SIZE);
    if (bytes_written != PAGE_SIZE) return -1;

    return 0;
}

// 파일 끝에 새로운 빈 페이지 공간을 할당하고 그 page_id를 반환
int page_allocate(int fd) {
    if (fd < 0) return -1;

    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size == -1) return -1;

    uint32_t new_page_id = (uint32_t)(file_size / PAGE_SIZE);

    char empty_page[PAGE_SIZE];
    page_init(empty_page);

    if (write(fd, empty_page,  PAGE_SIZE) != PAGE_SIZE) {
        return -1;
    }

    retur (int) new_page_id;
}

uint32_t heap_page_count(HeapFile *heap) {
    if (!heap || heap->fd < 0) return 0;

    off_t file_size = lseek(heap->fd, 0, SEEK_END);
    if (file_size <= 0) return 0;

    return (uint32_t)(file_size / PAGE_SIZE);
}