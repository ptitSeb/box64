// SPDX-License-Identifier: MIT
#ifndef _WIN32
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "debug.h"
#include "steamwebhelper.h"

#define STEAMUI_CHECK_INTERVAL_MS 500
#define STEAMUI_KILL_COOLDOWN_S   60
#define STEAMUI_DEFAULT_THRESHOLD 500
#define STEAMUI_MAX_PROCS         4096
#define STEAMUI_MAX_SWH           64
#define STEAMUI_MAX_ANCESTORS     16

typedef struct {
    int pid;
    int ppid;
    unsigned long rss_kb;
} steam_proc_t;

static steam_proc_t steam_procs[STEAMUI_MAX_PROCS];
static pthread_mutex_t steamui_mutex = PTHREAD_MUTEX_INITIALIZER;

static int read_proc_file(int pid, const char* name, char* buf, size_t sz)
{
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/%s", pid, name);
    int fd = open(path, O_RDONLY | O_CLOEXEC);
    if (fd < 0) return -1;
    ssize_t n = read(fd, buf, sz - 1);
    close(fd);
    if (n < 0) return -1;
    buf[n] = '\0';
    return (int)n;
}

static void read_proc_status(int pid, int* ppid, unsigned long* rss_kb)
{
    char buf[2048];
    *ppid = -1;
    *rss_kb = 0;
    if (read_proc_file(pid, "status", buf, sizeof(buf)) < 0) return;
    char* p = strstr(buf, "\nPPid:");
    if (p) sscanf(p + 6, "%d", ppid);
    p = strstr(buf, "\nVmRSS:");
    if (p) sscanf(p + 7, "%lu", rss_kb);
}

static int read_progname(int pid, char* progname, size_t sz)
{
    char buf[4096];
    int n = read_proc_file(pid, "cmdline", buf, sizeof(buf));
    if (n <= 0) return 0;
    size_t len = strlen(buf);
    if (!len) return 0;
    const char* p = strrchr(buf, '/');
    const char* a0 = p ? p + 1 : buf;
    const char* prog = a0;
    if (!strncmp(a0, "box64", 5) || !strncmp(a0, "box86", 5)) {
        // the guest program is the second argument
        if (len + 1 >= (size_t)n) return 0;
        const char* a1 = buf + len + 1;
        if (!*a1) return 0;
        p = strrchr(a1, '/');
        prog = p ? p + 1 : a1;
    }
    snprintf(progname, sz, "%s", prog);
    return 1;
}

static int is_box_process(int pid)
{
    char path[64], buf[512];
    snprintf(path, sizeof(path), "/proc/%d/exe", pid);
    ssize_t n = readlink(path, buf, sizeof(buf) - 1);
    if (n <= 0) return 0;
    buf[n] = '\0';
    const char* p = strrchr(buf, '/');
    p = p ? p + 1 : buf;
    return !strncmp(p, "box64", 5) || !strncmp(p, "box86", 5);
}

static int has_steam_ancestor(int pid, int box64_only)
{
    int ppid;
    unsigned long rss_kb;
    char progname[64];
    for (int hop = 0; hop < STEAMUI_MAX_ANCESTORS && pid >= 1; ++hop) {
        if (read_progname(pid, progname, sizeof(progname)) && !strncmp(progname, "steam", 5)) {
            if (!box64_only)
                return 1;
            if (is_box_process(pid)) {
                char path[64], buf[512];
                snprintf(path, sizeof(path), "/proc/%d/exe", pid);
                ssize_t n = readlink(path, buf, sizeof(buf) - 1);
                if (n > 0) {
                    buf[n] = '\0';
                    const char* p = strrchr(buf, '/');
                    p = p ? p + 1 : buf;
                    if (!strncmp(p, "box64", 5))
                        return 1;
                }
            }
        }
        read_proc_status(pid, &ppid, &rss_kb);
        pid = ppid;
    }
    return 0;
}

static time_t get_monotonic_s()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec;
}

static int steamui_scan(pid_t* swh_pids, int max_swh, int* nswh, int* self_is_swh, pid_t* stub_pids, int max_stub, int* nstub)
{
    unsigned long threshold_kb = (unsigned long)STEAMUI_DEFAULT_THRESHOLD * 1024;
    int nprocs = 0;
    *nswh = 0;
    *nstub = 0;
    *self_is_swh = 0;
    pthread_mutex_lock(&steamui_mutex);
    DIR* d = opendir("/proc");
    if (!d) {
        pthread_mutex_unlock(&steamui_mutex);
        return 0;
    }
    struct dirent* de;
    while ((de = readdir(d))) {
        if (de->d_name[0] < '0' || de->d_name[0] > '9') continue;
        int pid = atoi(de->d_name);
        if (pid <= 0) continue;
        if (nprocs >= STEAMUI_MAX_PROCS) break;
        steam_proc_t* p = &steam_procs[nprocs++];
        p->pid = pid;
        read_proc_status(pid, &p->ppid, &p->rss_kb);
    }
    closedir(d);
    pid_t self = getpid();
    int game_found = 0;
    for (int i = 0; i < nprocs; ++i) {
        char progname[64];
        if (!read_progname(steam_procs[i].pid, progname, sizeof(progname))) continue;
        if (!strcmp(progname, "steamwebhelper-stub")) {
            if (steam_procs[i].pid != self && *nstub < max_stub)
                stub_pids[(*nstub)++] = steam_procs[i].pid;
            continue;
        }
        if (strstr(progname, "steamwebhelper")) {
            if (steam_procs[i].pid == self)
                *self_is_swh = 1;
            else if (is_box_process(steam_procs[i].pid) && *nswh < max_swh)
                swh_pids[(*nswh)++] = steam_procs[i].pid;
            continue;
        }
        if (game_found || steam_procs[i].rss_kb < threshold_kb) continue;
        if (!strncmp(progname, "steam", 5)) continue;   // steam client, steam.exe...
        if (!is_box_process(steam_procs[i].pid)) continue;
        if (has_steam_ancestor(steam_procs[i].ppid, 0))
            game_found = 1;
    }
    pthread_mutex_unlock(&steamui_mutex);
    return game_found;
}

static int steamui_pass(time_t* last_kill)
{
    pid_t swh_pids[STEAMUI_MAX_SWH];
    pid_t stub_pids[STEAMUI_MAX_SWH];
    int nswh = 0;
    int nstub = 0;
    int self_is_swh = 0;
    int game_found = steamui_scan(swh_pids, STEAMUI_MAX_SWH, &nswh, &self_is_swh, stub_pids, STEAMUI_MAX_SWH, &nstub);
    if (!game_found) {
        if (!nstub) return 0;
        for (int i = 0; i < nstub; ++i)
            kill(stub_pids[i], SIGKILL);
        printf_log(LOG_INFO, "Game over, removed %d steamwebhelper decoy process(es)\n", nstub);
        return 1;
    }
    if (!nswh && !self_is_swh) return 0;
    time_t now = get_monotonic_s();
    if (now - *last_kill < STEAMUI_KILL_COOLDOWN_S) return 0;
    *last_kill = now;
    for (int i = 0; i < nswh; ++i)
        kill(swh_pids[i], SIGKILL);
    if (self_is_swh) kill(getpid(), SIGKILL); // kill ourselves
    printf_log(LOG_INFO, "killed %d steamwebhelper process(es), an emulated game is using %d MiB or more\n", nswh + self_is_swh, STEAMUI_DEFAULT_THRESHOLD);
    return 1;
}

static void* ondemand_steamui_thread(void* arg)
{
    (void)arg;
    sigset_t set;
    sigfillset(&set);
    pthread_sigmask(SIG_BLOCK, &set, NULL); // block all signals
    if (!BOX64ENV(ondemand_steamui)) return NULL;
    printf_log(LOG_INFO, "On-demand steamui monitor started\n");
    time_t last_kill = get_monotonic_s();
    for (;;) {
        struct timespec ts = {STEAMUI_CHECK_INTERVAL_MS / 1000, (STEAMUI_CHECK_INTERVAL_MS % 1000) * 1000000LL};
        nanosleep(&ts, NULL);
        steamui_pass(&last_kill);
    }
    return NULL;
}

void StartOnDemandSteamUI(void)
{
    static int started = 0;
    if (started) return;
    started = 1;
    if (has_steam_ancestor(getppid(), 1)) return;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_t thread;
    if (pthread_create(&thread, &attr, ondemand_steamui_thread, NULL))
        printf_log(LOG_INFO, "Warning, failed to start the on-demand steamui monitor\n");
    pthread_attr_destroy(&attr);
}

// return 1 if this exec is an attempt to (re)launch steamwebhelper while an emulated game is running, so it should not proceed.
int OnDemandSteamUIBlockExec(const char* path, int n, char* const* argv)
{
    if (!BOX64ENV(ondemand_steamui)) return 0;
    int is_swh = 0;
    if (path) {
        const char* p = strrchr(path, '/');
        p = p ? p + 1 : path;
        if (!strncmp(p, "steamwebhelper", 14)) is_swh = 1;
    }
    if (!is_swh && argv && n >= 3 && argv[0] && argv[1] && argv[2]
        && !strcmp(argv[0], "sh") && !strcmp(argv[1], "-c") && strstr(argv[2], "steamwebhelper.sh"))
        is_swh = 1;
    if (!is_swh && argv && n >= 1 && argv[0]) {
        const char* p = strrchr(argv[0], '/');
        p = p ? p + 1 : argv[0];
        if (!strncmp(p, "steamwebhelper", 14)) is_swh = 1;
    }
    if (!is_swh) return 0;
    // is an emulated game running?
    pid_t swh_pids[STEAMUI_MAX_SWH];
    pid_t stub_pids[STEAMUI_MAX_SWH];
    int nswh = 0;
    int nstub = 0;
    int self_is_swh = 0;
    if (!steamui_scan(swh_pids, STEAMUI_MAX_SWH, &nswh, &self_is_swh, stub_pids, STEAMUI_MAX_SWH, &nstub))
        return 0;
    printf_log(LOG_INFO, "Replacing steamwebhelper by a decoy process, a game is running\n");
    {
        char* stub_argv[] = { (char*)"steamwebhelper-stub", (char*)"86400", NULL };
        const char* stub_paths[] = { "/bin/sleep", "/usr/bin/sleep" };
        for (size_t i = 0; i < sizeof(stub_paths) / sizeof(*stub_paths); ++i)
            execv(stub_paths[i], stub_argv);
    }
    errno = EACCES;
    return 1;
}

#else // _WIN32
void StartOnDemandSteamUI(void) {}
int OnDemandSteamUIBlockExec(const char* path, int n, char* const* argv) { (void)path; (void)n; (void)argv; return 0; }
#endif // _WIN32
