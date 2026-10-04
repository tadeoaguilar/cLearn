#include <stdio.h>
#include <string.h> // only used to check our versions

static size_t my_strlen(const char* s) {
    const char* p = s;
    while (*p) p++;
    return (size_t)(p - s);
}

static int my_strcmp(const char* a, const char* b) {
    while (*a && *a == *b) {
        a++;
        b++;
    }
    // Compare as unsigned char, like the standard requires.
    return (unsigned char)*a - (unsigned char)*b;
}

static size_t my_strlcpy(char* dst, const char* src, size_t size) {
    size_t len = my_strlen(src);
    if (size > 0) {
        size_t n = len < size - 1 ? len : size - 1;
        for (size_t i = 0; i < n; i++) dst[i] = src[i];
        dst[n] = '\0';
    }
    return len; // >= size means truncated
}

static void my_strrev(char* s) {
    size_t n = my_strlen(s);
    if (n < 2) return;
    for (char *lo = s, *hi = s + n - 1; lo < hi; lo++, hi--) {
        char t = *lo;
        *lo = *hi;
        *hi = t;
    }
}

static int sign(int x) { return (x > 0) - (x < 0); }

int main(void) {
    const char* words[] = {"", "a", "hello", "hello world"};
    for (int i = 0; i < 4; i++)
        printf("my_strlen(\"%s\") = %zu (strlen %zu)\n", words[i], my_strlen(words[i]), strlen(words[i]));

    const char* pairs[][2] = {{"abc", "abd"}, {"abc", "abc"}, {"b", "abc"}, {"ab", "abc"}, {"\xff", "a"}};
    int ok = 1;
    for (int i = 0; i < 5; i++) {
        int mine = sign(my_strcmp(pairs[i][0], pairs[i][1]));
        int real = sign(strcmp(pairs[i][0], pairs[i][1]));
        printf("my_strcmp case %d: %d (strcmp %d)\n", i, mine, real);
        ok &= mine == real;
    }

    char buf[6];
    size_t r = my_strlcpy(buf, "truncate me", sizeof buf);
    printf("my_strlcpy → \"%s\", returned %zu → %s\n", buf, r, r >= sizeof buf ? "truncated" : "fits");
    r = my_strlcpy(buf, "fits", sizeof buf);
    printf("my_strlcpy → \"%s\", returned %zu → %s\n", buf, r, r >= sizeof buf ? "truncated" : "fits");

    char word[] = "stressed";
    my_strrev(word);
    printf("reversed \"stressed\" = \"%s\"\n", word);
    printf(ok ? "all strcmp checks passed\n" : "MISMATCH\n");
    return ok ? 0 : 1;
}
