/**********************************************************************************************************************
 *  INCLUDES
 *********************************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dfa.h"

/**********************************************************************************************************************
 *  PREPROCESSOR DEFINITIONS
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * GLOBAL VARIABLES DECLARATION
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * LOCAL FUNCTION DECLARATION
 *********************************************************************************************************************/
static bool compare(const char cmp, const char sym, const int op_code);


/**********************************************************************************************************************
 * LOCAL FUNCTION DEFINITION
 *********************************************************************************************************************/
static bool compare(const char cmp, const char sym, const int op_code) {
    bool ret = false;

    switch (op_code) {
        case CON_EQ:
            ret = (cmp == sym);
            break;
        case CON_NEQ:
            ret = (cmp != sym);
            break;
        default:
            break;
    }

    return ret;
}


/**********************************************************************************************************************
 * GLOBAL FUNCTION DEFINITION
 *********************************************************************************************************************/
dfa_t *dfa_init(void) {
    dfa_t *dfa = NULL;

    dfa = (dfa_t *) malloc(sizeof(dfa_t));
    if (NULL != dfa) {
        memset(dfa, 0, sizeof(dfa));
    } else {
        fprintf(stderr, "ERROR: Error allocating DFA.\n");
    }

    return dfa;
}


void dfa_setStartState(dfa_t *dfa, const int state) {
    dfa->start_state = state;
    dfa->current_state = state;
}


void dfa_addState(dfa_t *dfa, const int state_id, const bool has_action, void (*action)(const int)) {
    dfa_state_t *state = NULL;

    state = (dfa_state_t *) malloc(sizeof(dfa_state_t));
    if (NULL != state) {
        state->has_action = has_action;
        state->action = action;
        state->transition_num = 0;

        dfa->states[state_id] = state;
        dfa->num_of_states++;
    } else {
        fprintf(stderr, "ERROR: Error allocating state for DFA.\n");
    }
}


void dfa_addTransition(dfa_t *dfa, const int from_state, const int to_state, char sym, int op_code) {
    dfa_state_t *state = dfa->states[from_state];

    state->transitions[state->transition_num].sym = sym;
    state->transitions[state->transition_num].op_code = op_code;
    state->transitions[state->transition_num].to_state = to_state;

    state->transition_num++;
}


void dfa_transition(dfa_t *dfa, const char ch) {
    dfa_state_t *curr_state = dfa->states[dfa->current_state];

    for (int i = 0; i < curr_state->transition_num; i++) {
        if (compare(curr_state->transitions[i].sym, ch, curr_state->transitions[i].op_code)) {
            dfa->current_state = curr_state->transitions[i].to_state;
            break;
        }
    }
}


void dfa_free(dfa_t *dfa) {
    for (int i = 0; i < dfa->num_of_states; i++) {
        free(dfa->states[i]);
    }
    free(dfa);
}

/***********************************************************************************************************************
 * END OF FILE
 **********************************************************************************************************************/
