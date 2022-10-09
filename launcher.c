#include "err.h"
#include "resource.h"
#include "sandbox.h"
#include "utils/log.h"

#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

struct Config {
    char *memory_limit;
    char *nproc_limit;
    char *time_limit;
    char *sandbox_path;
    char *sandbox_template;
    char *sandbox_action;
    char *file_input;
    char *file_output;
    char *file_info;
    char *program;
} config;

void print_help(char *self) {
    LOG_WARN("Usage:");
    LOG_WARN("  %s [options]", self);
    LOG_WARN("Options:");
    LOG_WARN("  --memory_limit     memory limit in MB");
    LOG_WARN("  --nproc_limit      number of processes limit");
    LOG_WARN("  --time_limit       time limit in ms");
    LOG_WARN("  --sandbox_path     path to sandbox");
    LOG_WARN("  --sandbox_template sandbox template");
    LOG_WARN("  --sandbox_action   sandbox action");
    LOG_WARN("  --file_input       path to input file");
    LOG_WARN("  --file_output      path to output file");
    LOG_WARN("  --file_info        path to info file");
    LOG_WARN("  --program          program to run");
    LOG_WARN("  --help             print this help message");
}

void parse(int argc, char *argv[]) {
    static struct option long_options[] = {
        {"memory_limit",     required_argument, NULL, 0},
        {"nproc_limit",      required_argument, NULL, 0},
        {"time_limit",       required_argument, NULL, 0},
        {"sandbox_path",     required_argument, NULL, 0},
        {"sandbox_template", required_argument, NULL, 0},
        {"sandbox_action",   required_argument, NULL, 0},
        {"file_input",       required_argument, NULL, 0},
        {"file_output",      required_argument, NULL, 0},
        {"file_info",        required_argument, NULL, 0},
        {"program",          required_argument, NULL, 0},
        {"help",             no_argument,       NULL, 0},
        {NULL,               0,                 NULL, 0}
    };

    int c, idx = 0;
    while ((c = getopt_long_only(argc, argv, "", long_options, &idx)) != -1) {
        if (c != 0) break;

        const char *key = long_options[idx].name;
        char       *val = optarg;

        if (strcmp(key, "memory_limit") == 0) {
            config.memory_limit = val;
        } else if (strcmp(key, "nproc_limit") == 0) {
            config.nproc_limit = val;
        } else if (strcmp(key, "time_limit") == 0) {
            config.time_limit = val;
        } else if (strcmp(key, "sandbox_path") == 0) {
            config.sandbox_path = val;
        } else if (strcmp(key, "sandbox_template") == 0) {
            config.sandbox_template = val;
        } else if (strcmp(key, "sandbox_action") == 0) {
            config.sandbox_action = val;
        } else if (strcmp(key, "file_input") == 0) {
            config.file_input = val;
        } else if (strcmp(key, "file_output") == 0) {
            config.file_output = val;
        } else if (strcmp(key, "file_info") == 0) {
            config.file_info = val;
        } else if (strcmp(key, "program") == 0) {
            config.program = val;
        } else if (strcmp(key, "help") == 0) {
            print_help(argv[0]);
            exit(0);
        }
    }

    char **cfg = (char **)&config;
    for (int i = 0; i < 10; i++) {
        if (!cfg[i]) {
            print_help(argv[0]);
            LOG_ERR("Missing arguments");
            exit(ERR_ARGUMENTS);
        }
    }
}

void launch_child() {
    char *args[] = {config.program, NULL};
    char *env[7];

    {
        env[0] = malloc(sizeof("LD_PRELOAD=") + strlen(config.sandbox_path) + 1);
        sprintf(env[0], "LD_PRELOAD=%s", config.sandbox_path);

        env[1] = malloc(sizeof(LIMIT_MEMORY "=") + strlen(config.memory_limit) + 1);
        sprintf(env[1], LIMIT_MEMORY "=%s", config.memory_limit);

        env[2] = malloc(sizeof(LIMIT_NPROC "=") + strlen(config.nproc_limit) + 1);
        sprintf(env[2], LIMIT_NPROC "=%s", config.nproc_limit);

        env[3] = malloc(sizeof(LIMIT_TIME "=") + strlen(config.time_limit) + 1);
        sprintf(env[3], LIMIT_TIME "=%s", config.time_limit);

        env[4] = malloc(sizeof(SANDBOX_TEMPLATE "=") + strlen(config.sandbox_template) + 1);
        sprintf(env[4], SANDBOX_TEMPLATE "=%s", config.sandbox_template);

        env[5] = malloc(sizeof(SANDBOX_ACTION "=") + strlen(config.sandbox_action) + 1);
        sprintf(env[5], SANDBOX_ACTION "=%s", config.sandbox_action);

        env[6] = NULL;
    }

    {
        int fd = open(config.file_input, O_RDONLY);
        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    {
        int fd = open(config.file_output, O_WRONLY | O_CREAT, 0644);
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

    execve(config.program, args, env);
}

void dump_info(FILE *dest, struct rusage *usage, int status, long long time_usage) {
    fprintf(dest, "{\"real_time\":%lld,\"cpu_time\":%ld,\"memory\":%ld,", time_usage,
            usage->ru_utime.tv_sec * 1000 + usage->ru_utime.tv_usec / 1000, usage->ru_maxrss);
    if (WIFEXITED(status))
        fprintf(dest, "\"status\":\"exited\",\"code\":%d}", WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        fprintf(dest, "\"status\":\"killed\",\"code\":%d}", WTERMSIG(status));
    else
        fprintf(dest, "\"status\":\"unknown\",\"code\":0}");
}

int main(int argc, char *argv[]) {
    parse(argc, argv);

    pid_t child = fork();
    if (child < 0) {
        LOG_ERR("Failed to fork (child)");
        exit(ERR_FORK);

    } else if (child == 0) { // Program
        launch_child();
        LOG_ERR("Failed to execute child program");
        exit(ERR_EXEC);

    } else { // Supervisor
        pid_t killer = fork();
        if (killer < 0) {
            LOG_ERR("Failed to fork (killer)");
            exit(ERR_FORK);

        } else if (killer == 0) { // Killer
            long limit = (strtol(config.time_limit, NULL, 10) + 1000) / 1000 + 2;
            LOG_INFO("Killer started, time limit: %lds", limit);
            sleep(limit); // two more seconds
            LOG_WARN("Killer killed child");
            kill(child, SIGKILL);
            exit(0);

        } else { // stat
            int             status;
            struct rusage   usage;
            pid_t           ret;
            struct timespec time_begin, time_end;

            clock_gettime(CLOCK_MONOTONIC, &time_begin);
            ret = wait4(child, &status, 0, &usage);
            clock_gettime(CLOCK_MONOTONIC, &time_end);

            long long time_usage =
                (time_end.tv_sec - time_begin.tv_sec) * 1000 + (time_end.tv_nsec - time_begin.tv_nsec) / 1000000;

            kill(killer, SIGKILL);

            if (ret < 0) {
                LOG_ERR("Failed to wait for child process");
                exit(ERR_WAIT);
            }

            FILE *info = fopen(config.file_info, "w");
            dump_info(info, &usage, status, time_usage);
            exit(0);
        }
    }
}
