#ifndef FEATHERSQL_H
#define FEATHERSQL_H

#include <stdio.h>


void feather_repl(void);
void feather_run(const char* filepath);
void feather_usage(void);

#ifdef FEATHER_IMPLEMENTATION

void feather_repl(void) {
    TODO(__func__);
}

void feather_run(const char* filepath) {
    UNUSED(filepath);
    TODO(__func__);
}

void feather_usage(void) {
    fprintf(stderr, "Usage: feathersql [filename]\n");
}


#endif // FEATHER_IMPLEMENTATION

#endif // FEATHERSQL_H
