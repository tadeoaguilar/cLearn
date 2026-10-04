#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* data; // always NUL-terminated once initialized
    size_t len;
    size_t cap; // includes room for the '\0'
} StrBuf;

static bool sb_reserve(StrBuf* sb, size_t extra) {
    size_t need = sb->len + extra + 1;
    if (need <= sb->cap) return true;
    size_t cap = sb->cap ? sb->cap : 16;
    while (cap < need) cap *= 2;
    char* tmp = realloc(sb->data, cap);
    if (!tmp) return false;
    sb->data = tmp;
    sb->cap = cap;
    return true;
}

static bool sb_init(StrBuf* sb) {
    *sb = (StrBuf){0};
    if (!sb_reserve(sb, 0)) return false;
    sb->data[0] = '\0';
    return true;
}

static void sb_free(StrBuf* sb) {
    free(sb->data);
    *sb = (StrBuf){0};
}

static bool sb_append_n(StrBuf* sb, const char* s, size_t n) {
    if (!sb_reserve(sb, n)) return false;
    memcpy(sb->data + sb->len, s, n);
    sb->len += n;
    sb->data[sb->len] = '\0';
    return true;
}

static bool sb_append(StrBuf* sb, const char* s) { return sb_append_n(sb, s, strlen(s)); }
static bool sb_append_char(StrBuf* sb, char c) { return sb_append_n(sb, &c, 1); }

static bool sb_appendf(StrBuf* sb, const char* fmt, ...) {
    va_list args, copy;
    va_start(args, fmt);
    va_copy(copy, args); // a va_list can only be traversed once, so keep a copy for the 2nd pass
    int n = vsnprintf(NULL, 0, fmt, args); // pass 1: measure
    va_end(args);
    bool ok = n >= 0 && sb_reserve(sb, (size_t)n);
    if (ok) {
        vsnprintf(sb->data + sb->len, (size_t)n + 1, fmt, copy); // pass 2: write
        sb->len += (size_t)n;
    }
    va_end(copy);
    return ok;
}

// Transfers ownership of the buffer to the caller. The builder is left empty and reusable.
static char* sb_take(StrBuf* sb) {
    char* out = sb->data;
    *sb = (StrBuf){0};
    return out;
}

int main(void) {
    StrBuf sb;
    if (!sb_init(&sb)) return 1;
    bool ok = sb_append(&sb, "id,name,score\n");
    for (int i = 1; i <= 1000 && ok; i++) ok = sb_appendf(&sb, "%d,player_%03d,%.1f\n", i, i, i * 1.5);
    ok = ok && sb_append_char(&sb, '#');
    if (!ok) {
        sb_free(&sb);
        return 1;
    }
    printf("built %zu bytes (capacity %zu)\n", sb.len, sb.cap);

    char* csv = sb_take(&sb); // now WE own csv
    int lines = 0;
    for (const char* p = csv; *p && lines < 4; p++) {
        putchar(*p);
        if (*p == '\n') lines++;
    }
    printf("...\nlast char: '%c'; builder after take: data=%p len=%zu\n", csv[strlen(csv) - 1], (void*)sb.data, sb.len);
    free(csv);
    sb_free(&sb); // safe: free(NULL)
    return 0;
}
