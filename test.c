#include "utils/log.h"
#include <stdlib.h>

int main() {
    LOG_INFO("Testing Memory Limit");
    void *p = malloc(sizeof(int) * 1024 * 1024 * 10);
    if (!p) {
        LOG_ERR("malloc failed");
    }

    LOG_INFO("Testing NPROC Limit");
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
    } else if (pid == 0) {
        LOG_INFO("Child process");
        exit(0);
    } else {
        LOG_INFO("Parent process");
    }

    LOG_INFO("Testing Time Limit 1");
    sleep(5);

    LOG_INFO("Testing Time Limit 2");
    for (volatile int i = 0; i != -1; i++)
        ;

    LOG_INFO("Exiting...");
    return 0;
}
