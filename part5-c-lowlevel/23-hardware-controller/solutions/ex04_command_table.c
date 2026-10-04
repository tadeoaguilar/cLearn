#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int (*CmdHandler)(int argc, char** argv);

typedef struct {
    const char* name;
    int min_args, max_args; // not counting the command name
    CmdHandler handler;
    const char* usage;
    const char* help;
} Command;

static int g_duty = 0;
static bool g_auto = true;

static bool parse_long(const char* s, long lo, long hi, long* out) {
    char* end;
    errno = 0;
    long v = strtol(s, &end, 10);
    if (end == s || *end || errno || v < lo || v > hi) return false;
    *out = v;
    return true;
}

static int cmd_status(int argc, char** argv) {
    (void)argc;
    (void)argv;
    printf("mode=%s duty=%d%%\n", g_auto ? "AUTO" : "MANUAL", g_duty);
    return 0;
}
static int cmd_auto(int argc, char** argv) {
    (void)argc;
    (void)argv;
    g_auto = true;
    printf("ok\n");
    return 0;
}
static int cmd_manual(int argc, char** argv) {
    (void)argc;
    long duty;
    if (!parse_long(argv[1], 0, 100, &duty)) {
        printf("error: duty must be 0-100, got '%s'\n", argv[1]);
        return -1;
    }
    g_auto = false;
    g_duty = (int)duty;
    printf("ok: manual %ld%%\n", duty);
    return 0;
}
static int cmd_set(int argc, char** argv) {
    // set <param> <value> [unit]
    printf("ok: %s = %s%s%s\n", argv[1], argv[2], argc > 3 ? " " : "", argc > 3 ? argv[3] : "");
    return 0;
}
static int cmd_help(int argc, char** argv);

static const Command commands[] = {
    {"status", 0, 0, cmd_status, "", "show the current state"},
    {"auto", 0, 0, cmd_auto, "", "automatic fan curve"},
    {"manual", 1, 1, cmd_manual, "<0-100>", "fixed duty cycle"},
    {"set", 2, 3, cmd_set, "<param> <value> [unit]", "change a parameter"},
    {"help", 0, 1, cmd_help, "[command]", "list commands or describe one"},
};
enum { NCOMMANDS = sizeof commands / sizeof commands[0] };

static const Command* find(const char* name) {
    for (size_t i = 0; i < NCOMMANDS; i++)
        if (strcmp(commands[i].name, name) == 0) return &commands[i];
    return NULL;
}

static int cmd_help(int argc, char** argv) {
    for (size_t i = 0; i < NCOMMANDS; i++) {
        const Command* c = &commands[i];
        if (argc == 2 && strcmp(argv[1], c->name) != 0) continue;
        printf("  %-7s %-24s %s\n", c->name, c->usage, c->help);
    }
    return 0;
}

// Splits `line` in place on spaces. No heap: argv points into the line buffer.
static int tokenize(char* line, char** argv, int max) {
    int argc = 0;
    char* p = line;
    while (*p && argc < max) {
        while (*p == ' ' || *p == '\t') *p++ = '\0';
        if (!*p) break;
        argv[argc++] = p;
        while (*p && *p != ' ' && *p != '\t') p++;
    }
    return argc;
}

static int execute(char* line) {
    char* argv[8];
    int argc = tokenize(line, argv, 8);
    if (argc == 0) return 0;
    const Command* c = find(argv[0]);
    if (!c) {
        printf("error: unknown command '%s' (try 'help')\n", argv[0]);
        return -1;
    }
    int nargs = argc - 1;
    if (nargs < c->min_args || nargs > c->max_args) {
        printf("error: usage: %s %s\n", c->name, c->usage);
        return -1;
    }
    return c->handler(argc, argv);
}

int main(void) {
    const char* script[] = {"help", "status", "manual 40", "status", "manual 140", "manual",
                            "  set  target   45 C ", "launch rockets", "help manual", "auto", ""};
    for (size_t i = 0; i < sizeof script / sizeof script[0]; i++) {
        char line[64];
        snprintf(line, sizeof line, "%s", script[i]);
        printf("> %s\n", script[i]);
        execute(line);
    }
    return 0;
}
