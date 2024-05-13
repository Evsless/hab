#ifndef __DFA_H__
#define __DFA_H__

#include <stdbool.h>

#define MAX_TRANSITIONS 32u
#define MAX_STATES      64u

typedef enum {
    CON_EQ,
    CON_NEQ,
    CON_G,
    CON_GEQ,
    CON_L,
    CON_LEQ,
} condition_t;

typedef struct dfa_transition
{
    char sym;
    int op_code;
    int to_state;
} dfa_transition_t;


typedef struct dfa_state
{
    bool has_action;
    int transition_num;
    void (*action)(const int);
    dfa_transition_t transitions[MAX_TRANSITIONS];
} dfa_state_t;


typedef struct dfa
{
    int start_state;
    int current_state;
    int num_of_states;
    dfa_state_t *states[MAX_STATES];
} dfa_t;

dfa_t *dfa_init(void);
void dfa_free(dfa_t *dfa);

void dfa_addState(dfa_t *dfa, const int state_id, const bool has_action, void (*action)(const int));
void dfa_addTransition(dfa_t *dfa, const int from_state, const int to_state, char sym, int op_code);
void dfa_setStartState(dfa_t *dfa, const int state);
void dfa_transition(dfa_t *dfa, const char ch);

#endif /* __DFA_H__ */
