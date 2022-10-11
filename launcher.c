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

enum ConfigIndex {
    memory_limit = 0,
    nproc_limit,
    time_limit,
    sandbox_path,
    sandbox_template,
    sandbox_action,
    file_input,
    file_output,
    file_info,
    program,
    CONFIG_INDEX_MAX
};

char *config[CONFIG_INDEX_MAX];

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
    static struct option options[] = {
        [memory_limit]         = {"memory_limit",     required_argument, NULL, 0},
        [nproc_limit]          = {"nproc_limit",      required_argument, NULL, 0},
        [time_limit]           = {"time_limit",       required_argument, NULL, 0},
        [sandbox_path]         = {"sandbox_path",     required_argument, NULL, 0},
        [sandbox_template]     = {"sandbox_template", required_argument, NULL, 0},
        [sandbox_action]       = {"sandbox_action",   required_argument, NULL, 0},
        [file_input]           = {"file_input",       required_argument, NULL, 0},
        [file_output]          = {"file_output",      required_argument, NULL, 0},
        [file_info]            = {"file_info",        required_argument, NULL, 0},
        [program]              = {"program",          required_argument, NULL, 0},
        [CONFIG_INDEX_MAX]     = {"help",             no_argument,       NULL, 0},
        [CONFIG_INDEX_MAX + 1] = {NULL,               0,                 NULL, 0}
    };

    int c, idx = 0;
    while ((c = getopt_long_only(argc, argv, "", options, &idx)) != -1) {
        if (c != 0) break;

        if (idx < CONFIG_INDEX_MAX)
            config[idx] = optarg;
        else if (idx == CONFIG_INDEX_MAX) {
            print_help(argv[0]);
            exit(0);
        }
    }

    for (int i = 0; i < CONFIG_INDEX_MAX; i++) {
        if (!config[i]) {
            print_help(argv[0]);
            LOG_ERR("Missing arguments");
            exit(ERR_ARGUMENTS);
        }
    }
}

void launch_child() {
    char *args[] = {config[program], NULL};
    char *env[7];

    /* build env */ {
        env[0] = malloc(sizeof("LD_PRELOAD=") + strlen(config[sandbox_path]) + 1);
        sprintf(env[0], "LD_PRELOAD=%s", config[sandbox_path]);

        env[1] = malloc(sizeof(LIMIT_MEMORY "=") + strlen(config[memory_limit]) + 1);
        sprintf(env[1], LIMIT_MEMORY "=%s", config[memory_limit]);

        env[2] = malloc(sizeof(LIMIT_NPROC "=") + strlen(config[nproc_limit]) + 1);
        sprintf(env[2], LIMIT_NPROC "=%s", config[nproc_limit]);

        env[3] = malloc(sizeof(LIMIT_TIME "=") + strlen(config[time_limit]) + 1);
        sprintf(env[3], LIMIT_TIME "=%s", config[time_limit]);

        env[4] = malloc(sizeof(SANDBOX_TEMPLATE "=") + strlen(config[sandbox_template]) + 1);
        sprintf(env[4], SANDBOX_TEMPLATE "=%s", config[sandbox_template]);

        env[5] = malloc(sizeof(SANDBOX_ACTION "=") + strlen(config[sandbox_action]) + 1);
        sprintf(env[5], SANDBOX_ACTION "=%s", config[sandbox_action]);

        env[6] = NULL;
    }

    /* build stdin */ {
        int fd = open(config[file_input], O_RDONLY);
        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    /* build stdout */ {
        int fd = open(config[file_output], O_WRONLY | O_CREAT, 0644);
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

    execve(config[program], args, env);
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
            long limit = (strtol(config[time_limit], NULL, 10) + 1000) / 1000 + 2;
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

            FILE *info = fopen(config[file_info], "w");
            dump_info(info, &usage, status, time_usage);
            exit(0);
        }
    }
}
