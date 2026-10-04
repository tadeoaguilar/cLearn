// Ownership conventions in C: who allocates, who frees.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- 1. create/destroy: the library allocates the object itself ---- */
typedef struct {
    char* name; // owned: freed in player_destroy
    int hp;
} Player;

static char* dup_string(const char* s) { // returns a heap string. CALLER FREES.
    size_t n = strlen(s) + 1;            // +1 for the '\0'
    char* copy = malloc(n);
    if (copy) memcpy(copy, s, n);
    return copy;
}

static Player* player_create(const char* name) {
    Player* p = malloc(sizeof *p);
    if (!p) return NULL;
    p->name = dup_string(name);
    if (!p->name) { // partial failure: undo what succeeded
        free(p);
        return NULL;
    }
    p->hp = 100;
    return p;
}

static void player_destroy(Player* p) {
    if (!p) return;
    free(p->name); // deep free: members first...
    free(p);       // ...then the struct itself
}

/* ---- 2. init/free: the CALLER provides the storage, we only manage what's inside ---- */
typedef struct {
    int* scores;
    size_t count;
} Scoreboard;

static int scoreboard_init(Scoreboard* sb, size_t count) {
    sb->scores = calloc(count, sizeof *sb->scores);
    sb->count = sb->scores ? count : 0;
    return sb->scores ? 0 : -1;
}
static void scoreboard_free(Scoreboard* sb) {
    free(sb->scores);
    sb->scores = NULL;
    sb->count = 0;
}

/* ---- 3. caller-allocated buffer: no heap at all ---- */
static int describe(const Player* p, char* buf, size_t size) {
    return snprintf(buf, size, "%s (%d hp)", p->name, p->hp); // returns the length it needed
}

/* ---- 4. borrowed return value: points into the object; don't free it, don't keep it ---- */
static const char* player_name(const Player* p) { return p->name; }

/* ---- 5. ownership transfer: after this call the team owns the player ---- */
typedef struct {
    Player* members[4];
    size_t len;
} Team;

static int team_adopt(Team* t, Player* p) { // takes ownership on success only
    if (t->len == 4) return -1;          // on failure the caller still owns p
    t->members[t->len++] = p;
    return 0;
}
static void team_free(Team* t) {
    for (size_t i = 0; i < t->len; i++) player_destroy(t->members[i]);
    t->len = 0;
}

int main(void) {
    Player* hero = player_create("Ada");
    if (!hero) return 1;

    Scoreboard sb; // lives on the stack; only its array is on the heap
    if (scoreboard_init(&sb, 3) != 0) {
        player_destroy(hero);
        return 1;
    }
    sb.scores[0] = 50;

    char line[32];
    describe(hero, line, sizeof line);
    printf("caller buffer: %s\n", line);

    const char* borrowed = player_name(hero);
    printf("borrowed name: %s (valid only while hero is alive)\n", borrowed);

    Team team = {0};
    if (team_adopt(&team, hero) != 0) player_destroy(hero);
    hero = NULL; // we no longer own it; clear our copy to avoid accidental use
    Player* sidekick = player_create("Grace");
    if (sidekick && team_adopt(&team, sidekick) != 0) player_destroy(sidekick);

    printf("team has %zu members:", team.len);
    for (size_t i = 0; i < team.len; i++) printf(" %s", team.members[i]->name);
    printf("\n");

    team_free(&team);    // frees both players
    scoreboard_free(&sb); // frees the array; sb itself goes away with the stack frame
    printf("everything released: run with `leaks --atExit --` or ASan to verify\n");
    return 0;
}
