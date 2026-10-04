#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    char* key; // owned by the map
    int value;
    struct Node* next;
} Node;

typedef struct {
    Node** buckets;
    size_t bucket_count;
    size_t size;
} StrIntMap;

static uint64_t hash_str(const char* s) { // FNV-1a: simple and good enough
    uint64_t h = 1469598103934665603ULL;
    for (; *s; s++) {
        h ^= (unsigned char)*s;
        h *= 1099511628211ULL;
    }
    return h;
}

static bool map_init(StrIntMap* m, size_t buckets) {
    m->buckets = calloc(buckets, sizeof *m->buckets); // all NULL
    m->bucket_count = m->buckets ? buckets : 0;
    m->size = 0;
    return m->buckets != NULL;
}

static void map_free(StrIntMap* m) {
    for (size_t i = 0; i < m->bucket_count; i++) {
        Node* n = m->buckets[i];
        while (n) {
            Node* next = n->next; // read next BEFORE freeing n
            free(n->key);
            free(n);
            n = next;
        }
    }
    free(m->buckets);
    *m = (StrIntMap){0};
}

static bool map_rehash(StrIntMap* m, size_t new_count) {
    Node** nb = calloc(new_count, sizeof *nb);
    if (!nb) return false;
    for (size_t i = 0; i < m->bucket_count; i++) {
        Node* n = m->buckets[i];
        while (n) { // move nodes; no key copies or frees needed
            Node* next = n->next;
            size_t j = hash_str(n->key) % new_count;
            n->next = nb[j];
            nb[j] = n;
            n = next;
        }
    }
    free(m->buckets);
    m->buckets = nb;
    m->bucket_count = new_count;
    return true;
}

static bool map_put(StrIntMap* m, const char* key, int value) {
    size_t i = hash_str(key) % m->bucket_count;
    for (Node* n = m->buckets[i]; n; n = n->next)
        if (strcmp(n->key, key) == 0) {
            n->value = value;
            return true;
        }
    if ((double)(m->size + 1) / (double)m->bucket_count > 0.75) {
        if (!map_rehash(m, m->bucket_count * 2)) return false;
        i = hash_str(key) % m->bucket_count;
    }
    Node* n = malloc(sizeof *n);
    if (!n) return false;
    size_t len = strlen(key) + 1;
    n->key = malloc(len);
    if (!n->key) {
        free(n);
        return false;
    }
    memcpy(n->key, key, len);
    n->value = value;
    n->next = m->buckets[i];
    m->buckets[i] = n;
    m->size++;
    return true;
}

static bool map_get(const StrIntMap* m, const char* key, int* out) {
    for (Node* n = m->buckets[hash_str(key) % m->bucket_count]; n; n = n->next)
        if (strcmp(n->key, key) == 0) {
            *out = n->value;
            return true;
        }
    return false;
}

static bool map_remove(StrIntMap* m, const char* key) {
    // A pointer to the link that points at the current node lets us unlink without special cases.
    for (Node** link = &m->buckets[hash_str(key) % m->bucket_count]; *link; link = &(*link)->next) {
        if (strcmp((*link)->key, key) == 0) {
            Node* dead = *link;
            *link = dead->next;
            free(dead->key);
            free(dead);
            m->size--;
            return true;
        }
    }
    return false;
}

int main(void) {
    const char* text = "the quick brown fox jumps over the lazy dog the fox barks and the dog runs "
                       "over the hill while the quick cat sleeps";
    StrIntMap m;
    if (!map_init(&m, 4)) return 1;

    char word[32];
    size_t wl = 0;
    for (const char* p = text;; p++) {
        if (isalpha((unsigned char)*p) && wl < sizeof word - 1) {
            word[wl++] = *p;
        } else if (wl > 0) {
            word[wl] = '\0'; // `word` is a stack buffer; the map makes its own copy
            int count = 0;
            map_get(&m, word, &count);
            if (!map_put(&m, word, count + 1)) {
                map_free(&m);
                return 1;
            }
            wl = 0;
        }
        if (!*p) break;
    }
    printf("%zu distinct words in %zu buckets\n", m.size, m.bucket_count);
    const char* queries[] = {"the", "fox", "dog", "quick", "zebra"};
    for (int i = 0; i < 5; i++) {
        int c;
        if (map_get(&m, queries[i], &c)) printf("  %-6s %d\n", queries[i], c);
        else printf("  %-6s (absent)\n", queries[i]);
    }
    map_remove(&m, "the");
    map_remove(&m, "fox");
    int c;
    printf("after removing 'the' and 'fox': size=%zu, 'the' present? %s\n", m.size, map_get(&m, "the", &c) ? "yes" : "no");
    map_free(&m);
    return 0;
}
