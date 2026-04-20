/*
 * RvbbitSafe Kernel Detector - Finds rootkit artifacts and restores integrity
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/pid.h>
#include <linux/version.h>
#include <asm/paravirt.h>

static unsigned long **sys_call_table;

static unsigned long **find_sys_call_table(void)
{
    unsigned long addr = kallsyms_lookup_name("sys_call_table");
    if (addr) return (unsigned long **)addr;
    unsigned long **sct;
    for (sct = (unsigned long **)0xffffffff82000000; (unsigned long)sct < 0xffffffffffffffff; sct++)
        if (sct[__NR_close] == (unsigned long *)sys_close) return sct;
    return NULL;
}

static void scan_hidden_processes(void)
{
    struct task_struct *task;
    printk(KERN_INFO "[RvbbitSafe] Scanning for hidden processes...\n");
    for_each_process(task) {
        if (!pid_task(find_vpid(task->pid), PIDTYPE_PID))
            printk(KERN_WARNING "[RvbbitSafe] Hidden process: PID %d (%s)\n", task->pid, task->comm);
    }
}

static void restore_syscalls(void)
{
    if (!sys_call_table) return;
    unsigned long orig_kill = kallsyms_lookup_name("sys_kill");
    unsigned long orig_getdents64 = kallsyms_lookup_name("sys_getdents64");
    unsigned long orig_openat = kallsyms_lookup_name("sys_openat");
    unsigned long orig_bpf = kallsyms_lookup_name("sys_bpf");
    write_cr0(read_cr0() & ~0x10000);
    if (orig_kill) sys_call_table[__NR_kill] = (unsigned long *)orig_kill;
    if (orig_getdents64) sys_call_table[__NR_getdents64] = (unsigned long *)orig_getdents64;
    if (orig_openat) sys_call_table[__NR_openat] = (unsigned long *)orig_openat;
    if (orig_bpf) sys_call_table[__NR_bpf] = (unsigned long *)orig_bpf;
    write_cr0(read_cr0() | 0x10000);
    printk(KERN_INFO "[RvbbitSafe] Syscall table restored.\n");
}

static int __init detector_init(void)
{
    printk(KERN_INFO "[RvbbitSafe] Detector loaded.\n");
    sys_call_table = find_sys_call_table();
    scan_hidden_processes();
    restore_syscalls();
    return 0;
}

static void __exit detector_exit(void) { printk(KERN_INFO "[RvbbitSafe] Detector unloaded.\n"); }

module_init(detector_init); module_exit(detector_exit); MODULE_LICENSE("GPL");