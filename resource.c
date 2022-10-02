#include "resource.h"
#include "err.h"
#include "utils/log.h"

#include <stdlib.h>
#include <sys/resource.h>

#define UNSET_ENV(name)                                               \
    do {                                                              \
        if (unsetenv(name)) {                                         \
            LOG_ERR("Failed to unset environment variable %s", name); \
            exit(ERR_UNSETENV);                                       \
        }                                                             \
    } while (0)

#define SET_LIMIT(resource, limit, name, unit)                         \
    do {                                                               \
        if (limit) {                                                   \
            LOG_INFO("Setting " name " limit to %ld " unit, limit);    \
            if (setrlimit(resource, &(struct rlimit){limit, limit})) { \
                LOG_ERR("Failed to set " name " limit");               \
                exit(ERR_RLIMIT_SET);                                  \
            }                                                          \
        }                                                              \
    } while (0)

void setup_rlimit(void) {
    LOG_INFO("Setting resource limit");

    char *mem_limit_str   = getenv(LIMIT_MEMORY); // in mb
    char *nproc_limit_str = getenv(LIMIT_NPROC);
    char *time_limit_str  = getenv(LIMIT_TIME); // in ms

    long mem_limit = 0, nproc_limit = 0, time_limit = 0;

    if (mem_limit_str) {
        // convert to bytes and double the memory limit
        mem_limit = strtol(mem_limit_str, NULL, 10) * 1024 * 1024 * 2;
    }

    if (nproc_limit_str) {
        nproc_limit = strtol(nproc_limit_str, NULL, 10);
    }

    if (time_limit_str) {
        // add 2 seconds to avoid time limit exceeded
        long limit = strtol(time_limit_str, NULL, 10);
        time_limit = limit ? (limit / 1000 + 2) : 0;
    }

    SET_LIMIT(RLIMIT_AS, mem_limit, "memory", "bytes");
    SET_LIMIT(RLIMIT_NPROC, nproc_limit, "nproc", "processes"); // per user
    SET_LIMIT(RLIMIT_CPU, time_limit, "time", "seconds"); // except blocked time

    UNSET_ENV(LIMIT_MEMORY);
    UNSET_ENV(LIMIT_NPROC);
    UNSET_ENV(LIMIT_TIME);
}
