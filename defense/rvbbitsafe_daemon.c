/*
 * RvbbitSafe User-Space Daemon - Terminates hidden miners and removes persistence
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

static void kill_hidden_miner(void)
{
    DIR *dir = opendir("/proc");
    struct dirent *ent;
    while ((ent = readdir(dir))) {
        int pid = atoi(ent->d_name);
        if (pid <= 0) continue;
        char path[256], cmdline[256] = "";
        snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
        int fd = open(path, O_RDONLY);
        if (fd >= 0) { read(fd, cmdline, sizeof(cmdline)-1); close(fd); }
        if (strstr(cmdline, "xmrig") && !strstr(cmdline, "rvbbitsafe")) {
            printf("[RvbbitSafe] Terminating hidden miner PID %d\n", pid);
            kill(pid, SIGKILL);
        }
    }
    closedir(dir);
}

static void remove_persistence(void)
{
    system("rm -f /lib/modules/*/kernel/drivers/acpi/rvbbit.ko 2>/dev/null");
    system("rm -f /etc/modules-load.d/acpi.conf 2>/dev/null");
    system("systemctl disable rvbbit-helper.service 2>/dev/null");
    system("rm -f /etc/systemd/system/rvbbit-helper.service 2>/dev/null");
    system("rm -f /tmp/.rvbbit* 2>/dev/null");
    printf("[RvbbitSafe] Persistence artifacts removed.\n");
}

int main()
{
    printf("[RvbbitSafe] Daemon started. Monitoring...\n");
    while (1) { kill_hidden_miner(); remove_persistence(); sleep(30); }
    return 0;
}