#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>

#define SYS_PRINT_PLUS 450  // plus 시스템 콜 번호
#define SYS_PRINT_MINUS 451 // minus 시스템 콜 번호

int main() {
    char input[256];
    int num1, num2;
    char oper;
    int result;

    while (1) {
        printf("Input: ");
        fgets(input, sizeof(input), stdin);
        // 입력이 없으면 종료
	if(input[0] == '\n'){
		break;
	}

        // 입력 파싱
        if (sscanf(input, "%d %c %d", &num1, &oper, &num2) != 3) {
            printf("Wrong Input!\n");
            continue;
        }

        // 시스템 콜 함수 호출
        if (oper == '+') {
            result = syscall(SYS_PRINT_PLUS, num1, oper, num2);
        } else if (oper == '-') {
            result = syscall(SYS_PRINT_MINUS, num1, oper, num2);
        } else {
            printf("Wrong Input!\n");
            continue;
        }

        // 시스템 콜 결과 출력
        if (result == -1) {
            perror("System Call Failed");
        } else {
            printf("Output: %d\n", result);
        }
    }

    return 0;
}

