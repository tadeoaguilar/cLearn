// A table-driven finite state machine: the standard way to structure device logic.
//
// A garage-door controller. One button cycles the door, a sensor detects obstacles,
// and limit switches report "fully open" and "fully closed".
#include <stdio.h>

typedef enum { ST_CLOSED, ST_OPENING, ST_OPEN, ST_CLOSING, ST_STOPPED, ST_COUNT } State;
typedef enum { EV_BUTTON, EV_OPEN_LIMIT, EV_CLOSED_LIMIT, EV_OBSTACLE, EV_COUNT } Event;

static const char* const state_names[] = {"CLOSED", "OPENING", "OPEN", "CLOSING", "STOPPED"};
static const char* const event_names[] = {"button", "open-limit", "closed-limit", "obstacle"};

typedef void (*Action)(void);
static void motor_up(void) { printf("    action: motor UP\n"); }
static void motor_down(void) { printf("    action: motor DOWN\n"); }
static void motor_stop(void) { printf("    action: motor STOP\n"); }
static void reverse(void) { printf("    action: obstacle! motor STOP then UP\n"); }

typedef struct {
    int valid; // 0 = the event is ignored in this state
    State next;
    Action action;
} Transition;

// The whole behaviour is in this table. IGNORE means "this event does nothing in this state".
// Every state × event combination is visible in one place, which makes reviews easy.
#define IGNORE {0}
#define GO(next, action) {1, next, action}
static const Transition table[ST_COUNT][EV_COUNT] = {
    //                 BUTTON                    OPEN_LIMIT              CLOSED_LIMIT             OBSTACLE
    [ST_CLOSED]  = { GO(ST_OPENING, motor_up),   IGNORE,                  IGNORE,                    IGNORE },
    [ST_OPENING] = { GO(ST_STOPPED, motor_stop), GO(ST_OPEN, motor_stop), IGNORE,                    IGNORE },
    [ST_OPEN]    = { GO(ST_CLOSING, motor_down), IGNORE,                  IGNORE,                    IGNORE },
    [ST_CLOSING] = { GO(ST_STOPPED, motor_stop), IGNORE,                  GO(ST_CLOSED, motor_stop), GO(ST_OPENING, reverse) },
    [ST_STOPPED] = { GO(ST_CLOSING, motor_down), IGNORE,                  IGNORE,                    IGNORE },
};

static State dispatch(State s, Event e) {
    const Transition* t = &table[s][e];
    if (!t->valid) {
        printf("  %-8s + %-12s → ignored\n", state_names[s], event_names[e]);
        return s;
    }
    printf("  %-8s + %-12s → %s\n", state_names[s], event_names[e], state_names[t->next]);
    t->action();
    return t->next;
}

int main(void) {
    const Event script[] = {EV_BUTTON, EV_OPEN_LIMIT, EV_BUTTON, EV_OBSTACLE, EV_OPEN_LIMIT,
                            EV_BUTTON, EV_BUTTON,     EV_BUTTON, EV_CLOSED_LIMIT, EV_OBSTACLE};
    State s = ST_CLOSED;
    for (size_t i = 0; i < sizeof script / sizeof script[0]; i++) s = dispatch(s, script[i]);
    printf("final state: %s\n", state_names[s]);
    return 0;
}
