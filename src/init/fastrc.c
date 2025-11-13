#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/reboot.h>
#include <linux/reboot.h>
#include <sys/mount.h>
#include <errno.h>

#include "../../include/generated/autoconf.h"

static int verbose = 0;

// --- Logging ---
static void log_msg(const char *msg) {
    if (verbose) dprintf(STDOUT_FILENO, ":: %s\n", msg);
}

static void log_errno(const char *msg) {
    dprintf(STDOUT_FILENO, ":: %s: %s\n", msg, strerror(errno));
}

// --- Commands ---
static char *sysinit_cmd = NULL;
static char *shutdown_cmd = NULL;
static char *reboot_cmd = NULL;
static char *ctrlaltdel_cmd = NULL;

// --- Respawn ---
#define MAX_RESPAWN 64
typedef struct { pid_t pid; char *cmd; } respawn_entry;
static respawn_entry respawns[MAX_RESPAWN];
static size_t respawn_count = 0;

static pid_t spawn(const char *cmd) {
    if (!cmd || !*cmd) return -1;
    pid_t pid = fork();
    if (pid == 0) {
        execl("/bin/sh", "sh", "-c", cmd, NULL);
        log_errno("exec cmd");
        _exit(1);
    }
    return pid;
}

static void add_respawn(const char *cmd) {
    if (respawn_count >= MAX_RESPAWN) return;
    respawns[respawn_count].cmd = strdup(cmd);
    respawns[respawn_count].pid = spawn(cmd);
    respawn_count++;
}

// --- Script runner ---
static void run_script(const char *path) {
    if (!path || !*path) return;

    if (access(path, X_OK) != 0) {
        log_errno("script not executable");
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
        execl(path, path, NULL);
        log_errno("exec script failed");
        _exit(1);
    }
    waitpid(pid, NULL, 0);
}

// --- Signals ---
static void sigchld_handler(int sig) {
    (void)sig;
    while (1) {
        pid_t dead = waitpid(-1, NULL, WNOHANG);
        if (dead <= 0) break;
        for (size_t i = 0; i < respawn_count; i++) {
            if (respawns[i].pid == dead) {
                respawns[i].pid = spawn(respawns[i].cmd);
                break;
            }
        }
    }
}

static void sigint_handler(int sig) {
    (void)sig;
    log_msg("ctrl+alt+del pressed");
    if (ctrlaltdel_cmd) run_script(ctrlaltdel_cmd);
}

// --- Poweroff / Reboot ---
static void do_poweroff(void) {
    sync();
    if (reboot(LINUX_REBOOT_CMD_POWER_OFF) < 0)
        log_errno("poweroff failed");
    for (;;) asm volatile("hlt");
}

static void do_reboot(void) {
    sync();
    if (reboot(LINUX_REBOOT_CMD_RESTART) < 0)
        log_errno("reboot failed");
    for (;;) asm volatile("hlt");
}

static void sigterm_handler(int sig) {
    (void)sig;
    dprintf(STDOUT_FILENO, ":: Shutdown requested\n");
    if (shutdown_cmd) run_script(shutdown_cmd);
    do_poweroff();
}

static void sigusr1_handler(int sig) {
    (void)sig;
    dprintf(STDOUT_FILENO, ":: Reboot requested\n");
    if (reboot_cmd) run_script(reboot_cmd);
    do_reboot();
}

// --- Config parsing ---
static void parse_conf(void) {
    FILE *f = fopen("/etc/fastrc/conf", "r");
    if (!f) return;
    char line[256];
    while (fgets(line, sizeof(line), f))
        if (strncmp(line, "verbose=", 8) == 0)
            verbose = atoi(line + 8);
    fclose(f);
}

static int parse_inittab(void) {
    FILE *f = fopen("/etc/fastrc/inittab", "r");
    if (!f) { log_errno("open inittab"); return -1; }

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = 0;
        if (!*line || line[0] == '#') continue;
        char *eq = strchr(line, '=');
        if (!eq) continue;
        *eq = 0;
        char *key = line, *val = eq + 1;
        while (*val == ' ' || *val == '\t') val++;

        if      (!strcmp(key, "sysinit"))     sysinit_cmd     = strdup(val);
        else if (!strcmp(key, "shutdown"))    shutdown_cmd    = strdup(val);
        else if (!strcmp(key, "reboot"))      reboot_cmd      = strdup(val);
        else if (!strcmp(key, "ctrlaltdel"))  ctrlaltdel_cmd  = strdup(val);
        else if (!strcmp(key, "respawn"))     add_respawn(val);
    }
    fclose(f);
    return 0;
}

// --- Automount ---
static void do_automounts(void) {
#ifdef CONFIG_MOUNT_FS
    log_msg("Mounting filesystems...");
    #ifdef CONFIG_MOUNT_FS_DEV
    mount("devtmpfs", "/dev", "devtmpfs", 0, "");
    #endif
    #ifdef CONFIG_MOUNT_FS_PROC
    mount("proc", "/proc", "proc", 0, "");
    #endif
    #ifdef CONFIG_MOUNT_FS_SYS
    mount("sysfs", "/sys", "sysfs", 0, "");
    #endif
    #ifdef CONFIG_MOUNT_FS_TMP
    mount("tmpfs", "/tmp", "tmpfs", 0, "");
    #endif
#endif
}

// --- Main ---
int main(void) {
    if (getpid() != 1) {
        fprintf(stderr, "FastRC must be ran as PID 1\n");
        exit(1);
    }

    parse_conf();

    struct sigaction sa = {0};
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NOCLDSTOP | SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);

    signal(SIGINT,  sigint_handler);
    signal(SIGTERM, sigterm_handler);
    signal(SIGUSR1, sigusr1_handler);

    reboot(LINUX_REBOOT_CMD_CAD_OFF);

    if (parse_inittab() < 0) exit(1);
    do_automounts();

    if (sysinit_cmd) run_script(sysinit_cmd);
    else { log_errno("No sysinit configured, exiting..."); exit(1); }

    for (;;) pause();
}

