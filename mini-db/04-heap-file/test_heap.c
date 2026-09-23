#include <stdio.h>
#include <string.h>
#include <assert.h>
#include  <unistd.h>
#include "heap_file.h"
#include "page.h"

void test_heap_file() {
    printf("=== A3 Heap File Test  Start  ===\n");
    const char *test_db = "test_heap.db";
    unlink(test_db);

    HeapFile heap;
    assert(heap_open(&heap, test_db) == 0);
    //
    // Test 1. 기본 삽입 및 rid 조회
    //
    printf("[1] Basic Insert & Get Test... \n");
    RID rid1, rid2;
    char buf[100];

    assert(heap_insert(&heap, "Hello Heap", 10, &rid1) == 0);
    assert(heap_insert(&heap, "Database System", 15, &rid2) == 0);

    // 첫 시작이므로 0번 페이지에 들어가야 함
    assert(rid1.page_id == 0 && rid1.slot_id == 0);
    assert(rid2.page_id == 0 && rid2.slot_id == 1);

    // 조회 검증
    assert(heap_get(&heap, rid1, buf, sizeof(buf)) == 10);
    assert(memcmp(buf, "Hello Heap", 10) == 0);

    assert(heap_get(&heap, rid2, buf, sizeof(buf)) == 15);
    assert(memcmp(buf, "Database System", 15) == 0);
    printf("-> Passed!\n\n");

    // -------------------------------------------------------------
    // Test 2: Multi-Page Auto Expansion (페이지 자동 확장)
    // -------------------------------------------------------------
    printf("[2] Multi-Page Expansion Test...\n");

    char dummy_record[500];
    memset(dummy_record, 'A', sizeof(dummy_record));

    RID last_rid;
    for (int i = 0; i < 20; i++) {
        assert(heap_insert(
            &heap, 
            dummy_record,
            sizeof(dummy_record),
            &last_rid
            ) == 0
        );
    }

    // 페이지가 1개 이상으로 증가했는지 확인
    uint32_t total_pages = heap_page_count(&heap);
    assert(total_pages > 1);
    printf(
        "-> Total Pages Allocated: %u\n",
        total_pages
    );

    printf(
        "-> Last RID: page=%u slot=%u\n\n",
        last_rid.page_id,
        last_rid.slot_id
    );
    // -------------------------------------------------------------
    // Test 3: Delete & Get Failure (삭제 검증)
    // -------------------------------------------------------------
    printf("[3] Delete Test...\n");
    assert(heap_delete(&heap, rid1) == 0);
    
    // 삭제된 RID로 조회 시 실패(-1)해야 함X
    assert(heap_get(&heap, rid1, buf, sizeof(buf)) == -1);
    printf("-> Passed!\n\n");

    // -------------------------------------------------------------
    // Test 4: Slot Reuse (삭제된 슬롯 재활용)
    // -------------------------------------------------------------
    printf("[4] Slot Reuse Test...\n");
    RID new_rid;
    assert(heap_insert(&heap, "Reused Slot", 11, &new_rid) == 0);

    // 방금 삭제된 rid1의 위치(page_id: 0, slot_id: 0)로 재활용되어 들어갔는지 확인
    assert(new_rid.page_id == rid1.page_id);
    assert(new_rid.slot_id == rid1.slot_id);

    assert(heap_get(&heap, new_rid, buf, sizeof(buf)) == 11);
    assert(memcmp(buf, "Reused Slot", 11) == 0);
    printf("-> Passed!\n\n");

    heap_close(&heap);
    unlink(test_db); // 테스트 파일 정리
    printf("=== All A3 Heap File Tests Passed! ===\n");
}

int main() {
    test_heap_file();
    return 0;
}
