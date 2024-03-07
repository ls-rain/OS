#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/string.h>

// sys_print_reverse 시스템 콜 함수 정의
asmlinkage long sys_print_reverse(const char __user *src, char __user *dest) {
    char input[100];
    char output[100];
    int len = 0;


    // 입력 문자열을 사용자 공간에서 커널 공간으로 복사
    if (copy_from_user(input, src, sizeof(input))) {
        return -EFAULT;
    }

    // 입력 문자열의 길이 계산
    while (len < sizeof(input) && input[len] != '\0') {
        len++;
    }

    // 문자열을 뒤집음
    int i, j;
    for (i = len - 1, j = 0; i >= 0; i--, j++) {
        output[j] = input[i];
    }
    output[len] = '\0';

    // 출력 결과를 사용자 공간으로 복사
    if (copy_to_user(dest, output, 64)) {
        return -EFAULT;
    }

    return 0;
}

SYSCALL_DEFINE2(print_reverse, const char __user*, src, char __user*, dest) {
    return sys_print_reverse(src, dest);
}
