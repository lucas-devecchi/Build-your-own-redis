#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

const size_t k_max_msg = 4096;

// Log a msg
static void msg(const char *msg) {
    fprintf(stderr, "%s\n", msg);
}

// Log an error and abort
static void die(const char *msg) {
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

#endif