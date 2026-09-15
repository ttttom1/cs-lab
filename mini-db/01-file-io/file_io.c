#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define PAGE_SIZE 4096

int main(void) {
    //Todo1 
    //database.db를 읽기/쓰기 가능하게 연다.
    //파일이 없으면 생성한다.
    int fd = open("database.db", O_RDWR | O_CREATE,0644);

    if (fd < 0) {
        perror("open");
        exit(1);
    }

    //page 하나 크기의 버퍼
    char page[PAGE_SIZE];

    // 전체를 0으로 초기화
    memset(page, 0, PAGE_SIZE);

    //TODO 2
    //page 안에 "Hello DB" 저장
    strcpy(page, "Hello DB");

    //우리가 쓸 page
    int page_id = 2;

    //Todo 3
    // page_id 를 실제파일 OFFSET으로 변환 
    off_t offset = (off_t)page_id * PAGE_SIZE;

    //todo4 :  lseek()를 이용해 page 2 위치로 이동
    if (lseek(fd, offset, SEEK_SET) < 0) {
        perror("lseek");
        close(fd);
        exit(1);
    } 

    // todo5 : write() 로 정확히 PAGE_SIZE 만큼 저장
    if (write(fd, page, PAGE_SIZE) < 0) {
        perror("write");
        close(fd);
        exit(1);
    }

    ///Todo 6
    if (fsync(fd) < 0) {
        perror("fsync");
        close(fd);
        exit(1);
    }

    close(fd);
    printf("write complete\n");
    return 0;
}