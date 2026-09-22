#include "page.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

void test_basic() {
    printf("=== Test 1: Basic Insert & Get ===\n");
    char page[PAGE_SIZE];
    page_init(page);

    int s0 = page_insert(page, "AAA", 3);
    int s1 = page_insert(page, "Hello Database", 14);
    int s2 = page_insert(page, "CCCCCCCC", 8);

    printf("Inserted slots: %d, %d, %d\n", s0, s1, s2);
    assert(s0 == 0 && s1 == 1 && s2 == 2);

    char buf[100];
    memset(buf, 0, sizeof(buf));

    page_get(page, 0, buf, sizeof(buf));
    printf("Slot 0: %s\n", buf);
    assert(strcmp(buf, "AAA") == 0);

    memset(buf, 0, sizeof(buf));
    page_get(page, 1, buf, sizeof(buf));
    printf("Slot 1: %s\n", buf);
    assert(strcmp(buf, "Hello Database") == 0);

    memset(buf, 0, sizeof(buf));
    page_get(page, 2, buf, sizeof(buf));
    printf("Slot 2: %s\n", buf);
    assert(strcmp(buf, "CCCCCCCC") == 0);

    printf("Basic Test Passed!\n\n");
}

void test_full_capacity() {
    printf("=== Test 2: Full Capacity Test ===\n");
    char page[PAGE_SIZE];
    page_init(page);

    char record[32];
    memset(record, 'A', sizeof(record));

    int count = 0;
    while (1) {
        int slot = page_insert(page, record, sizeof(record));
        if (slot < 0) break;
        count++;
    }

    PageHeader *header = (PageHeader *)page;
    printf("Total records inserted: %d\n", count);
    printf("slot_count: %d\n", header->slot_count);
    printf("free_start: %d\n", header->free_start);
    printf("free_end: %d\n", header->free_end);
    printf("Remaining space: %d\n", header->free_end - header->free_start);

    // Invariant 검증
    assert(header->free_start <= header->free_end);
    assert(header->free_end <= PAGE_SIZE);
    assert(header->free_start >= sizeof(PageHeader));

    printf("Capacity Test Passed!\n\n");
}

void test_delete() {
    printf("=== Test 3: Delete Record & Single Fragmentation ===\n");
    char page[PAGE_SIZE];
    page_init(page);

    int s0 = page_insert(page, "AAA", 3);
    int s1 = page_insert(page, "BBBBBBBB", 8);
    int s2 = page_insert(page, "CCCCC", 5);

    assert(s0 == 0);
    assert(s1 == 1);
    assert(s2 == 2);

    char buf[100];

    // 1. 삭제 전 정상 조회 확인
    assert(page_get(page, 1, buf, sizeof(buf)) == 8);

    // 2. Slot 1 삭제 시도
    int del_res = page_delete(page, 1);
    assert(del_res == 0); // 삭제 성공

    // 3. 삭제 후 조회 확인 (s1은 실패해야 하고, s0/s2는 살아있어야 함)
    assert(page_get(page, 0, buf, sizeof(buf)) == 3);
    assert(page_get(page, 1, buf, sizeof(buf)) < 0); // 실패(-1)
    assert(page_get(page, 2, buf, sizeof(buf)) == 5);

    // 4. 이미 삭제된 slot_id 재삭제 시도 (실패해야 함)
    assert(page_delete(page, 1) == -1);

    // 5. 범위 밖 slot_id 삭제 시도 (실패해야 함)
    assert(page_delete(page, 99) == -1);

    printf("Delete Test Passed!\n\n");
}

void test_auto_compact_and_reuse()  {
    printf("===Test 4: Auto Compaction & Slot  Reuse ===\n");
    char page[PAGE_SIZE];
    page_init(page);

    //페이지를 꽉 채움
    char record[32];
    memset(record, 'A', sizeof(record));

    for (int i  = 0  ;i < 113;i++) {
        assert(page_insert(page,record, sizeof(record)) == i);
    }

    assert(page_insert(page,record,sizeof(record)) == -1);

    // 중간 레코드 2개 삭제 (Slot 10, Slot 20 삭제 -> 총 64B 구멍 발생)
    assert(page_delete(page, 10) == 0);
    assert(page_delete(page, 20) == 0);

    int new_slot = page_insert(page, record, sizeof(record));
    printf("Reused slot ID: %d\n", new_slot);
    assert(new_slot == 10);

    PageHeader *header = (PageHeader *)page;
    assert(header->slot_count == 113);

    printf("Auto Compaction & Reuse Test Passed!\n\n");
}

int main(void) {
    test_basic();
    test_full_capacity();
    test_delete();
    test_auto_compact_and_reuse();
    return 0;
}