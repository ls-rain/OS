#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/syscall.h>
#include <unistd.h>

// 시스템 콜 번호
#define SYS_PRINT_REVERSE 449

int main() {
    char input[256];
    char output[256];

    while (1) {
        printf("Input: ");
        fgets(input, sizeof(input), stdin);

        // 입력이 없으면 종료
        if (input[0] == '\n') {
            break;
        }

        // 개행 문자 제거
        input[strcspn(input, "\n")] = '\0';

        // 입력 문자열이 자연수 범위에 있는지 확인
        int is_natural_number = 1;
        int len = strlen(input);
        for (int i = 0; i < len; i++) {
            if (!isdigit(input[i])) {
                is_natural_number = 0;
                break;
            }
        }

        if (is_natural_number) {
            // 시스템 콜을 호출하여 커널 공간에서 처리
            long result = syscall(SYS_PRINT_REVERSE, input, output);

            if (result == 0) {
                printf("Output: %s\n", output);
            } else {
                printf("System call failed.\n");
            }
        } else {
            printf("Output: Wrong Input!\n");
        }
    }

    return 0;
}

