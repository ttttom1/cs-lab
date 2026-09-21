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

int main(void) {
    test_basic();
    test_full_capacity();
    return 0;
}