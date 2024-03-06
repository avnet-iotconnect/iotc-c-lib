#ifndef IOTCL_CONFIG_H
#define IOTCL_CONFIG_H

#include <string.h> // for strtok

// This is an example config file where we override IOTCL_ENDLN for our tests
#define IOTCL_ENDLN "\n"

// We are compiling test with c99 to ensure compatibility at that level,
// so just routing to strtok because we won't be using threads in tests, so it's all safe
// using an inline function may not be ideal ideal, but it is quick for this purpose
static inline char *my_strtok_r(char *str, const char *delim, char **saveptr) {
    (void) saveptr;
    return strtok(str, delim);
}
#define IOTCL_STRTOK_R my_strtok_r

#define IOTCL_DRA_URL_BUFFER_SLACK 0

#endif //IOTCL_CONFIG_H