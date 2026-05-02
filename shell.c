#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX 1024
#define MAX_JOBS 100

// Job structure
struct job {
    int id;
    pid_t pid;
};

struct job jobs[MAX_JOBS];
int job_count = 0;

// Handle zombie processes
void handle_sigchld(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
    char input[MAX];
    char *args[100];

    signal(SIGCHLD, handle_sigchld);

    while (1) {
        printf("myshell> ");
        fgets(input, MAX, stdin);

        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "exit") == 0)
            break;

        // Parse input
        int i = 0;
        args[i] = strtok(input, " ");
        while (args[i] != NULL) {
            i++;
            args[i] = strtok(NULL, " ");
        }

        if (args[0] == NULL)
            continue;

        // Built-in: cd
        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL)
                printf("cd: missing argument\n");
            else
                chdir(args[1]);
            continue;
        }

        // Built-in: jobs
        if (strcmp(args[0], "jobs") == 0) {
            for (int j = 0; j < job_count; j++) {
                printf("[%d] PID: %d\n", jobs[j].id, jobs[j].pid);
            }
            continue;
        }

        // Built-in: fg
        if (strcmp(args[0], "fg") == 0) {
            if (args[1] == NULL) {
                printf("Usage: fg <job_id>\n");
                continue;
            }

            int id = atoi(args[1]);

            for (int j = 0; j < job_count; j++) {
                if (jobs[j].id == id) {
                    waitpid(jobs[j].pid, NULL, 0);
                    printf("Process %d finished\n", jobs[j].pid);
                }
            }
            continue;
        }

        // Background detection
        int background = 0;
        if (i > 0 && strcmp(args[i - 1], "&") == 0) {
            background = 1;
            args[i - 1] = NULL;
        }

        pid_t pid = fork();

        if (pid == 0) {
            execvp(args[0], args);
            perror("Error");
            exit(1);
        } else {
            if (!background) {
                waitpid(pid, NULL, 0);
            } else {
                jobs[job_count].id = job_count + 1;
                jobs[job_count].pid = pid;

                printf("[%d] Running in background (PID: %d)\n",
                       job_count + 1, pid);

                job_count++;
            }
        }
    }

    return 0;
}
