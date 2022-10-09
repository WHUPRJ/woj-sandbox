#include "utils/log.h"
#include <stdlib.h>
#include <unistd.h>

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
        LOG_WARN("Child process");
        LOG_WARN("Exiting...");
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

    // destroy stdin, stdout, stderr
    close(0);
    close(1);
    close(2);
    stdin  = NULL;
    stdout = NULL;
    stderr = NULL;

    return 0;
}
