// Error handling in C: return codes, errno and the goto-cleanup pattern.
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    ERR_OK = 0,
    ERR_OPEN = -1,
    ERR_NOMEM = -2,
    ERR_EMPTY = -3,
} Err;

static const char* err_str(Err e) {
    switch (e) {
    case ERR_OK: return "ok";
    case ERR_OPEN: return "cannot open file";
    case ERR_NOMEM: return "out of memory";
    case ERR_EMPTY: return "file is empty";
    }
    return "unknown";
}

// Count lines and the longest line. Acquires a FILE* and a buffer, and must release both on every path.
static Err file_stats(const char* path, size_t* lines, size_t* longest) {
    Err rc = ERR_OK;
    FILE* f = fopen(path, "r");
    if (!f) {
        rc = ERR_OPEN;
        goto out;
    }
    char* line = malloc(1024);
    if (!line) {
        rc = ERR_NOMEM;
        goto close_file;
    }
    *lines = 0;
    *longest = 0;
    while (fgets(line, 1024, f)) {
        size_t len = strcspn(line, "\n");
        if (len > *longest) *longest = len;
        (*lines)++;
    }
    if (*lines == 0) rc = ERR_EMPTY;

    free(line); // cleanup runs in reverse order of acquisition
close_file:
    fclose(f);
out:
    return rc;
}

int main(void) {
    // errno: set by library functions on failure. Only meaningful right after a failure.
    FILE* f = fopen("/definitely/not/here.txt", "r");
    if (!f) printf("fopen failed: errno=%d (%s)\n", errno, strerror(errno));

    // Create a small file, then analyse it and a missing one.
    const char* path = "ch21_errors_demo.txt";
    FILE* out = fopen(path, "w");
    if (!out) return 1;
    fputs("first line\na much longer second line\nthird\n", out);
    fclose(out);

    const char* paths[] = {path, "missing.txt"};
    for (int i = 0; i < 2; i++) {
        size_t lines = 0, longest = 0;
        Err e = file_stats(paths[i], &lines, &longest);
        if (e == ERR_OK) printf("%s: %zu lines, longest %zu chars\n", paths[i], lines, longest);
        else printf("%s: error %d (%s)\n", paths[i], e, err_str(e));
    }
    remove(path);
    return EXIT_SUCCESS;
}
