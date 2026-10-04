// build: also-compile ex02_mem.S
#include <stdio.h>
#include <string.h>

void* asm_memset(void* dst, int c, size_t n);
size_t asm_count_char(const char* s, int c);

int main(void) {
    char buf[16];
    memset(buf, '.', sizeof buf);
    void* r = asm_memset(buf + 2, '#', 5);
    buf[sizeof buf - 1] = '\0';
    printf("buf = \"%s\", returned dst? %s\n", buf, r == buf + 2 ? "yes" : "no");
    asm_memset(buf, 'z', 0); // n = 0 must not touch memory
    printf("after n=0: \"%s\"\n", buf);

    const char* text = "she sells sea shells by the sea shore";
    const char probes[] = {'s', 'e', 'z', ' '};
    for (int i = 0; i < 4; i++) {
        size_t expected = 0;
        for (const char* p = text; *p; p++) expected += *p == probes[i];
        printf("count '%c' = %zu (expected %zu)\n", probes[i], asm_count_char(text, probes[i]), expected);
    }
    printf("count in \"\" = %zu\n", asm_count_char("", 'a'));
    printf("count 0xE9 in \"caf\\xE9\" = %zu (bytes above 127 work too)\n", asm_count_char("caf\xE9", 0xE9));
    return 0;
}
