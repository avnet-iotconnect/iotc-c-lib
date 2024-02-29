#ifndef IOTCL_CONFIG_H
#define IOTCL_CONFIG_H

// This is an example config file where we override IOTCL_ENDLN for our tests


#define IOTCL_ENDLN "\n"
/*

#define IOTCL_FATAL(err_code, ...) \
    do { \
        printf("IOTCL FATAL (%d): ", err_code); printf(__VA_ARGS__); printf(IOTCL_ENDLN); \
    } while(0)
#endif

#define IOTCL_ERROR(err_code, ...) \
    do { \
        (void)(err_code); \
        printf("IOTCL ERROR (%d): ", err_code); printf(__VA_ARGS__); printf(IOTCL_ENDLN); \
    } while(0)

#define IOTCL_WARN(err_code, ...) \
    do { \
        (void)(err_code); \
        printf("IOTCL WARN (%d): ", err_code); printf(__VA_ARGS__); printf(IOTCL_ENDLN); \
    } while(0)
*/


#endif //IOTCL_CONFIG_H