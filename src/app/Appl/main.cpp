#include <stdio.h>
#include "hab.h"

int main(void) {
    printf("HELLO\n");
    hab_init();
    hab_run();

    return 0;
}