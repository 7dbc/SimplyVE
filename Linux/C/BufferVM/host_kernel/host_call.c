#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/string.h>
#include <linux/unistd.h>
#include <linux/mm.h>
#include <asm/cacheflush.h>
#include <asm/vmx.h>

#include <linux/kvm_host.h>
#include <asm/kvm_host.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Access non-exported symbols");
MODULE_AUTHOR("Ben Bancroft");

static int (*buffervm_set_memory_rw)(unsigned long addr, int numpages);
static int (*buffervm_set_memory_ro)(unsigned long addr, int numpages);

static int (*vmcall_handle_func)(struct kvm_vcpu *vcpu);

static int buffervm_handle_vmcall(struct kvm_vcpu *vcpu)
{
	unsigned long nr;

	nr = vcpu->arch.regs[VCPU_REGS_RAX];

	printk("[%s] vmcall: 0x%lx\n", __this_module.name, nr);
	return (*vmcall_handle_func)(vcpu);
}

static int __init buffervm_init(void)
{
	unsigned long addr;

	buffervm_set_memory_rw = (void *)kallsyms_lookup_name("set_memory_rw");
	if (!buffervm_set_memory_rw) {
		pr_err("can't find set_memory_rw symbol\n");
		return -ENXIO;
	}

	buffervm_set_memory_ro = (void *)kallsyms_lookup_name("set_memory_ro");
	if (!buffervm_set_memory_ro) {
		pr_err("can't find set_memory_ro symbol\n");
		return -ENXIO;
	}

	unsigned long handler_base_addr = kallsyms_lookup_name("kvm_vmx_exit_handlers");
	uintptr_t *kvm_vmcall_exit_handler = (uintptr_t *)(handler_base_addr + sizeof (uintptr_t) * EXIT_REASON_VMCALL);

	uintptr_t vmcall_handle_func_symbol = kallsyms_lookup_name("handle_vmcall");

	if (*kvm_vmcall_exit_handler != vmcall_handle_func_symbol) {
		pr_err("Cannot patch vmcall handler - original function is wrong. Is kernel newer?\n");
		return -ENXIO;
	}

	vmcall_handle_func = vmcall_handle_func_symbol;

	printk(KERN_INFO "[%s] (0x%lx): 0x%lx actual 0x%lx\n", __this_module.name, handler_base_addr, *kvm_vmcall_exit_handler, vmcall_handle_func_symbol);

	addr = PAGE_ALIGN((uintptr_t) kvm_vmcall_exit_handler) - PAGE_SIZE;

	buffervm_set_memory_rw(addr, 1);
	*kvm_vmcall_exit_handler = &buffervm_handle_vmcall;
	buffervm_set_memory_ro(addr, 1);

	return 0;
}

static void __exit buffervm_exit(void)
{
	unsigned long addr;
	unsigned long handler_base_addr;
	uintptr_t *kvm_vmcall_exit_handler;
	
	handler_base_addr = kallsyms_lookup_name("kvm_vmx_exit_handlers");
	kvm_vmcall_exit_handler = (uintptr_t *)(handler_base_addr + sizeof (uintptr_t) * EXIT_REASON_VMCALL);

	addr = PAGE_ALIGN((uintptr_t) kvm_vmcall_exit_handler) - PAGE_SIZE;

	buffervm_set_memory_rw(addr, 1);
	*kvm_vmcall_exit_handler = vmcall_handle_func;
	buffervm_set_memory_ro(addr, 1);

	printk(KERN_INFO "Goodbye world 1.\n");
}

module_init(buffervm_init);
module_exit(buffervm_exit);
