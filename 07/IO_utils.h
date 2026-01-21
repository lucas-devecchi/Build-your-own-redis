#ifndef IO_UTILS_H
#define IO_UTILS_H

#include <unistd.h> // read, write
#include <assert.h> // assert
#include <stdint.h> // int32_t
#include <stddef.h> // size_t
#include <vector>

// append to the back
static void buf_append(std::vector<uint8_t> &buf, const uint8_t *data, size_t len)
{
    buf.insert(buf.end(), data, data + len);
}

// remove from the front
static void buf_consume(std::vector<uint8_t> &buf, size_t n)
{
    buf.erase(buf.begin(), buf.begin() + n);
}

#endif // IO_UTILS_H