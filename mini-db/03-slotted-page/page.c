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

    // 1. 필요한 여유 공간 계산 (Record 크기 + Slot 1개 크기)
    uint32_t required_space = length + sizeof(Slot);
    uint32_t current_free_space = header->free_end - header->free_start;

    // 공간 부족 시 실패
    if (current_free_space < required_space) {
        return -1;
    }

    uint16_t slot_id = header->slot_count;

    // 2. Record 데이터 기록 (free_start 위치)
    memcpy((char *)page + header->free_start, record, length);

    // 3. Slot 위치 계산 (오른쪽에서 왼쪽으로 배치)
    uint16_t slot_offset = PAGE_SIZE - (slot_id + 1) * sizeof(Slot);
    Slot *slot = (Slot *)((char *)page + slot_offset);

    // 4. Slot 정보 작성
    slot->offset = header->free_start;
    slot->length = length;

    // 5. Header 위치 상태 업데이트
    header->free_start += length;
    header->free_end -= sizeof(Slot);
    header->slot_count++;

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

int page_delete(void  *page, uint16_t slot_id) {
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