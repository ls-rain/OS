#include <linux/kernel.h>
#include <linux/syscalls.h>

asmlinkage long sys_print_minus(int num1, char oper, int num2) {
    int result;

    if (oper == '-') {
        result = num1 + num2;
    }
    printk(KERN_INFO "Result: %d\n", result);
    return result;
}

SYSCALL_DEFINE3(print_minus, int, num1, char, oper, int, num2) {
        return sys_print_minus(num1, oper, num2);
}
