#include <stdio.h>
#include <stdlib.h>

#include "types.h"

#undef assert
static void exit_with_message(const char* exp, const char* file, const char* func, const i32 line) {
    fprintf(stderr, "Assertion failed: %s (%s: %s: %d)\n", exp, file, func, line);
    fflush(NULL);
    exit(EXIT_FAILURE);
}
#define assert(exp) ((void)((exp) || (exit_with_message(#exp, __FILE__, __func__, __LINE__), 0)))
