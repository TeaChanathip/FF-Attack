#include "dummy.h"
#include <stdint.h>
#include <stdlib.h>

int dummy_light1() {
    int x = 0;
    for (int i=0; i<10; i++) {
        x++;
    }
    return x;
}

uint64_t dummy_heavy() {
    uint64_t x = 0;
    uint64_t y = 1;
    for (int i=0; i<10000; i++) {
        x = x+y;
        y = x;
    }
    return x;
}

int dummy_light2() {
    int x = 0;
    for (int i=10; i<0; i--) {
        x-=2;
    }
    return x;
}
