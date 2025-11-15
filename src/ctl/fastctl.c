#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

void usage(const char *prog) {
    printf("Usage: %s <command>\n", prog);
    printf("Commands:\n");
    printf("  poweroff   Poweroff the system\n");
    printf("  reboot     Reboot the system\n");
}

int main(int argc, char **argv) {
    pid_t init_pid = 1; // FastRC is PID 1
    int sig = 0;
    const char *cmd = NULL;

    // First check if called with argument: "fastctt"
    if (argc >= 2) {
        cmd = argv[1];
    } else {
        // Otherwise, check program name
        const char *progname = strrchr(argv[0], '/');
        progname = progname ? progname + 1 : argv[0];
        cmd = progname;
    }

    if (strcmp(cmd, "poweroff") == 0) {
        sig = SIGTERM;  // tell FastRC to shutdown
    } else if (strcmp(cmd, "reboot") == 0) {
        sig = SIGUSR1;  // tell FastRC to reboot
    } else {
        usage(argv[0]);
        return 1;
    }

    if (kill(init_pid, sig) != 0) {
        perror("Failed to send signal to FastRC");
        return 1;
    }

    return 0;
}

