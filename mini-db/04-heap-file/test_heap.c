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

void test_heap_persistence() {
    printf("=== A3 Heap File Persistence===\n");
    const char *db_name =  "persistence_test.db";
    unlink(db_name);

    RID  saved_rid1,  saved_rid2;
    char buffer[100];
    // =========================================================================
    // Phase 1: DB 오픈 -> 데이터 삽입 -> 파일 닫기 (Close)
    // =========================================================================
    {
        printf("[Phase 1] Writing records to DB...\n");
        HeapFile heap;
        assert(heap_open(&heap, db_name) == 0);

        assert(heap_insert(&heap, "PERSISTENT_DATA_1", 18, &saved_rid1) == 0);
        assert(heap_insert(&heap, "PERSISTENT_DATA_2", 18, &saved_rid2) == 0);

        heap_close(&heap);
        printf("->  Phase 1 Success: Database  closed.\n\n");
    }
    // =========================================================================
    // Phase 2: DB 재오픈 -> 디스크 데이터 복원 및 검증
    // =========================================================================
    {
        printf("[Phase 2] Reopening DB & Recovering data from Disk...\n");
        HeapFile heap_reopened;
        assert(heap_open(&heap_reopened, db_name) == 0);

        // 재오픈 후에도 총 페이지 수가 유효하게 보존되어 있는지 검증
        uint32_t page_count = heap_page_count(&heap_reopened);
        assert(page_count > 0);

        // 이전 Phase 1에서 받아둔 RID로 디스크에서 그대로 읽어오기
        memset(buffer, 0, sizeof(buffer));
        assert(heap_get(&heap_reopened, saved_rid1, buffer, sizeof(buffer)) == 18);
        assert(strcmp(buffer, "PERSISTENT_DATA_1") == 0);

        memset(buffer, 0, sizeof(buffer));
        assert(heap_get(&heap_reopened, saved_rid2, buffer, sizeof(buffer)) == 18);
        assert(strcmp(buffer, "PERSISTENT_DATA_2") == 0);

        printf("-> Record 1 Recovered: %s\n", buffer);
        printf("-> Phase 2 Success: All data verified from disk!\n\n");

        heap_close(&heap_reopened);
    }
    // =========================================================================
    // Phase 3: DB 재오픈 -> 삭제(Delete) 연산의 영속성 검증
    // =========================================================================
    {
        printf("[Phase 3] Testing Persistence of Delete Operation...\n");
        HeapFile heap;
        assert(heap_open(&heap, db_name) == 0);

        // Record 1 삭제 후 DB 닫기
        assert(heap_delete(&heap, saved_rid1) == 0);
        heap_close(&heap);

        // 다시 DB 열어서 Record 1이 정말 디스크 상에서도 삭제되었는지 확인
        assert(heap_open(&heap, db_name) == 0);
        assert(heap_get(&heap, saved_rid1, buffer, sizeof(buffer)) == -1); // 실패해야 함

        // Record 2는 여전히 잘 남아있어야 함
        assert(heap_get(&heap, saved_rid2, buffer, sizeof(buffer)) == 18);

        heap_close(&heap);
        printf("-> Phase 3 Success: Delete operation persisted correctly!\n\n");
    }

    unlink(db_name); // 테스트 끝난 후 파일 정리
    printf("=== All Persistence Tests Passed Successfully! ===\n");
}

int main() {
    test_heap_file();
    test_heap_persistence();
    return 0;
}
