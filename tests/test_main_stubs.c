/*
 * Stub implementations for functions that are excluded from the unit-test build.
 * The real versions live in main.c, which is not linked into the modular tests.
 */

#include <stdlib.h>

#include "test_stubs.h"

void
quit(int sig)
{
    (void)sig;
    exit(0);
}

void
shell(void)
{
    /* Tests do not spawn subshells. */
}

void
endit(int sig)
{
    (void)sig;
    exit(1);
}

void
leave(int sig)
{
    (void)sig;
    exit(0);
}

void
my_exit(int status)
{
    exit(status);
}

void
playit(void)
{
    /* Not used in tests. */
}
