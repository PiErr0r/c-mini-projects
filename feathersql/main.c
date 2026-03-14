#define FEATHER_IMPLEMENTATION
#include "feathersql.h"

int main(int argc, const char **argv) {
    if (argc == 1) {
        feather_repl();
    } else if (argc == 2) {
        feather_run(argv[1]);
    } else {
        feather_usage();
    }
    return 0;
}
