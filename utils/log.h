#ifndef WOJ_SANDBOX_LOG_H
#define WOJ_SANDBOX_LOG_H

#include <stdio.h>
#include <unistd.h>

// Log
#define COLOR_RED          "\x1B[1;31m"
#define COLOR_YELLOW       "\x1B[1;33m"
#define COLOR_GREEN        "\x1B[1;32m"
#define COLOR_STDOUT_RESET "\x1B[0m"

#define _LOG(color, level, fmt, ...)                                                                     \
    do {                                                                                                 \
        fprintf(stderr, color "[" level "]\t(%s:%d):\t" fmt COLOR_STDOUT_RESET "\n", __FILE__, __LINE__, \
                ##__VA_ARGS__);                                                                          \
    } while (0)

#define LOG_INFO(fmt, ...) _LOG(COLOR_GREEN, "I", fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) _LOG(COLOR_YELLOW, "W", fmt, ##__VA_ARGS__)
#define LOG_ERR(fmt, ...)  _LOG(COLOR_RED, "E", fmt, ##__VA_ARGS__)

#endif // WOJ_SANDBOX_LOG_H
