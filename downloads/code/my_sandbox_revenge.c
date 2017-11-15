/*
    gcc my_sandbox_revenge.c -o my_sandbox_revenge  -Wl,-s -Wl,-z,now,-z,relro -fno-stack-protector
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#define STACK_SIZE 1000000

#define puts(s)\
write(1, s "\n", strlen(s "\n"))

#define printf(s)\
write(1, s, strlen(s))

void echo(char *buf, int len) {
    //int len;
    size_t map_size;
    size_t guard_size = 0x21000;
    void *new_stack;
    void *tmp_rsp;
    void *tmp_rbp;
    char *tmp_buf;

    map_size = sizeof(char*) + sizeof(char)*len + STACK_SIZE + sizeof(void*)*8;
    map_size = (map_size+0xfff) & (~0xfff);
    new_stack = mmap(NULL, 
                     map_size+guard_size, 
                     PROT_READ | PROT_WRITE, 
                     MAP_ANONYMOUS | MAP_PRIVATE, 
                     -1, 
                     0);
    if (new_stack == MAP_FAILED) {
        puts("Sorry, something went wrong!!!");
        return ;
    }

    munmap(new_stack+map_size, guard_size); // create a "guard hole"
    tmp_rbp = new_stack + sizeof(char)*len + STACK_SIZE + sizeof(void*)*8;
    tmp_rsp = tmp_rbp - sizeof(void*)*8 - sizeof(char)*len;
    tmp_buf = tmp_rsp;

    printf("Entered: ");
    memcpy(tmp_buf, buf, len);

    asm volatile(
            "movq %0, %%rax"::"g"(tmp_buf)
        );
    asm volatile(
            "movq %rbp, %fs:0x200"
        );
    asm volatile(
            "movq %rsp, %fs:0x208"
        );
    asm volatile(
            "movq %0, %%rsp"::"g"(tmp_rsp)
        );
    asm volatile(
            "movq %0, %%rbp"::"g"(tmp_rbp)
        );
    asm volatile(
            "movq %rax, -0x8(%rbp)"
        );

    dprintf(1, tmp_buf);

    asm volatile(
            "movq %fs:0x200, %rbp"
        );
    asm volatile(
            "movq %fs:0x208, %rsp"
        );

    puts("\n");

    munmap(new_stack, sizeof(char*) + sizeof(char) * len + STACK_SIZE);
    return ;
}

int read_n(void *buf, int size) {
    int idx = 0;
    while (size > 0) {
        int ret = read(0, buf+idx, size);
        if (ret < 0) return -1;
        size -= ret;
        idx += ret;
    }
    return 0;
}

int read_until(char *buf, unsigned int size, char c) {
    int idx = 0;
    while (size > 0) {
        int ret = read(0, buf+idx, 1);
        if (ret < 0) return -1;
        if (buf[idx] == c) {
            buf[idx] = '\0';
            break;
        }

        size -= ret;
        idx += ret;
    }
    return 0;
}

int main() {
    unsigned int size;
    char *buf;

    puts("\nWelcome to my echo program!");
    sleep(1);

    puts("\n\nThis program has some bug.");
    puts("I couldn't fix that... :(");
    sleep(1);

    puts("\nAlso a sandbox I developed in the past was defeated by evil hackers.");
    sleep(2);

    puts("\nSo, let me retry. I have a lot of confidence in this brand new sandbox.");
    puts("Crackers will never be able to get shell, haha! :)\n");

    sleep(1);

    printf("Enter the size of your message: ");
    if (read_n(&size, sizeof(unsigned int)) < 0) {
        puts("Sorry, something went wrong!!!");
        return 0;
    }

    if (size+1 < size) {
        puts("Sorry, something went wrong!!!");
        return 0;
    }

    buf = (char*)malloc(size+1);
    if (!buf) {
        puts("Sorry, something went wrong!!!");
        return 0;
    }

    printf("Enter your message: ");
    memset(buf, '\x00', size+1);
    if (read_until(buf, size, '\n') < 0) {
        puts("Sorry, something went wrong!!!");
        return 0;
    }
    echo(buf, size+1);
    free(buf);

    puts("\nSeems you couldn't crack my program. I'm a winner!");
    puts("See you.\n");
}
