// C strings: storage, safe copying, parsing and tokenizing.
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void dump(const char* label, const char* buf, size_t size) {
    printf("%-10s", label);
    for (size_t i = 0; i < size; i++) {
        if (buf[i] == '\0') printf("[\\0]");
        else printf("[%c] ", buf[i]);
    }
    printf("  strlen=%zu sizeof=%zu\n", strlen(buf), size);
}

// Parse an int strictly: the whole string must be a number in range.
static bool parse_int(const char* s, int* out) {
    char* end;
    errno = 0;
    long v = strtol(s, &end, 10);
    if (end == s || *end != '\0') return false;                   // no digits, or trailing junk
    if (errno == ERANGE || v < INT_MIN || v > INT_MAX) return false; // out of range
    *out = (int)v;
    return true;
}

int main(void) {
    char s[6] = "hi";
    dump("s", s, sizeof s);

    const char* literal = "hello"; // in read-only memory: literal[0] = 'H' would be UB
    printf("literal \"%s\" lives at %p\n\n", literal, (void*)literal);

    // Safe copy/concatenate: snprintf always NUL-terminates and reports truncation.
    char small[8];
    int needed = snprintf(small, sizeof small, "%s, %s!", "Hello", "world");
    printf("snprintf wrote \"%s\"; needed %d chars, buffer holds %zu → %s\n", small, needed, sizeof small - 1,
           needed >= (int)sizeof small ? "TRUNCATED" : "ok");

    // strncpy trap: no terminator when the source is too long.
    char dst[4];
    strncpy(dst, "abcdef", sizeof dst);
    printf("strncpy left dst unterminated: last byte is '%c' (not '\\0')\n", dst[3]);
    dst[sizeof dst - 1] = '\0'; // you must always do this yourself
    printf("after manual fix: \"%s\"\n\n", dst);

    const char* inputs[] = {"42", "-17", "12abc", "", "99999999999"};
    for (size_t i = 0; i < sizeof inputs / sizeof inputs[0]; i++) {
        int v;
        if (parse_int(inputs[i], &v)) printf("parse_int(\"%s\") = %d\n", inputs[i], v);
        else printf("parse_int(\"%s\") → error (atoi would return %d silently)\n", inputs[i], atoi(inputs[i]));
    }

    // strtok modifies its input (replaces delimiters with '\0'), so the input must be writable.
    char csv[] = "red,green,,blue";
    printf("\ntokens of \"%s\":", csv);
    for (char* tok = strtok(csv, ","); tok; tok = strtok(NULL, ",")) printf(" <%s>", tok);
    printf("\n(note: strtok skips the empty field, and it isn't thread-safe; see strtok_r or exercise 2)\n");
    return 0;
}
