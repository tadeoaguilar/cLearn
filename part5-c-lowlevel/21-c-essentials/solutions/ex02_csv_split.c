#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Splits `line` in place. Every separator becomes '\0'; fields[i] points at each field.
static size_t split(char* line, char sep, char* fields[], size_t max_fields) {
    size_t count = 0;
    if (max_fields == 0) return 0;
    fields[count++] = line;
    for (char* p = line; *p; p++) {
        if (*p == '\n' || *p == '\r') { // tolerate a trailing newline from fgets
            *p = '\0';
            break;
        }
        if (*p == sep) {
            *p = '\0';
            if (count == max_fields) break; // ignore extra fields
            fields[count++] = p + 1;
        }
    }
    return count;
}

typedef struct {
    char name[32];
    int age;
    char city[32];
} Person;

typedef enum { PARSE_OK, PARSE_FIELD_COUNT, PARSE_EMPTY_NAME, PARSE_BAD_AGE } ParseResult;

static ParseResult parse_person(char* line, Person* out) {
    char* f[4];
    size_t n = split(line, ',', f, 4);
    if (n != 4) return PARSE_FIELD_COUNT;
    if (f[0][0] == '\0') return PARSE_EMPTY_NAME;
    snprintf(out->name, sizeof out->name, "%s", f[0]);

    char* end;
    errno = 0;
    long age = strtol(f[1], &end, 10);
    if (end == f[1] || *end || errno || age < 0 || age > 150) return PARSE_BAD_AGE;
    out->age = (int)age;

    // f[2] (nickname) is optional and ignored; f[3] is the city.
    snprintf(out->city, sizeof out->city, "%s", f[3][0] ? f[3] : "(unknown)");
    return PARSE_OK;
}

int main(void) {
    char demo[] = "a,,b,";
    char* fields[8];
    size_t n = split(demo, ',', fields, 8);
    printf("\"a,,b,\" has %zu fields:", n);
    for (size_t i = 0; i < n; i++) printf(" <%s>", fields[i]);
    printf("\n\n");

    static const char* messages[] = {"ok", "wrong number of fields", "empty name", "invalid age"};
    const char* rows[] = {"Ana,28,,Mexico City", "Luis,abc,,Lima", ",30,,Quito", "Carmen,41", "Beto,35,Bob,\n"};
    for (int i = 0; i < 5; i++) {
        char line[128];
        snprintf(line, sizeof line, "%s", rows[i]); // split needs a writable copy
        Person p;
        ParseResult r = parse_person(line, &p);
        if (r == PARSE_OK) printf("row %d: name=%s age=%d city=%s\n", i, p.name, p.age, p.city);
        else printf("row %d: error: %s\n", i, messages[r]);
    }
    return 0;
}
