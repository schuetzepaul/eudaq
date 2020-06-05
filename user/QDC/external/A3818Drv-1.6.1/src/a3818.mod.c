#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x2f3152af, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x6bc3fbc0, __VMLINUX_SYMBOL_STR(__unregister_chrdev) },
	{ 0xdb7305a1, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0x22f7940, __VMLINUX_SYMBOL_STR(device_create) },
	{ 0xfae7f184, __VMLINUX_SYMBOL_STR(__class_create) },
	{ 0xdadb06a0, __VMLINUX_SYMBOL_STR(__register_chrdev) },
	{ 0xe9eccca2, __VMLINUX_SYMBOL_STR(proc_create_data) },
	{ 0xd6ee688f, __VMLINUX_SYMBOL_STR(vmalloc) },
	{ 0x19ad6985, __VMLINUX_SYMBOL_STR(__init_waitqueue_head) },
	{ 0xd6b8e852, __VMLINUX_SYMBOL_STR(request_threaded_irq) },
	{ 0x42c8de35, __VMLINUX_SYMBOL_STR(ioremap_nocache) },
	{ 0x5e25804, __VMLINUX_SYMBOL_STR(__request_region) },
	{ 0x3aa796f9, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0xafc7bca5, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x9e3b0d90, __VMLINUX_SYMBOL_STR(pci_disable_link_state) },
	{ 0xe0bcc53e, __VMLINUX_SYMBOL_STR(pci_set_master) },
	{ 0x9eadf5cb, __VMLINUX_SYMBOL_STR(pci_enable_msi) },
	{ 0xa715532f, __VMLINUX_SYMBOL_STR(pci_enable_device) },
	{ 0x91a0ac8e, __VMLINUX_SYMBOL_STR(pci_get_subsys) },
	{ 0xdf566a59, __VMLINUX_SYMBOL_STR(__x86_indirect_thunk_r9) },
	{ 0xfbf495e0, __VMLINUX_SYMBOL_STR(arch_dma_alloc_attrs) },
	{ 0xa46c12d, __VMLINUX_SYMBOL_STR(class_destroy) },
	{ 0xaf78932e, __VMLINUX_SYMBOL_STR(remove_proc_entry) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x9fa09383, __VMLINUX_SYMBOL_STR(device_destroy) },
	{ 0x8d15114a, __VMLINUX_SYMBOL_STR(__release_region) },
	{ 0xedc03953, __VMLINUX_SYMBOL_STR(iounmap) },
	{ 0x999e8297, __VMLINUX_SYMBOL_STR(vfree) },
	{ 0xc4cd60b4, __VMLINUX_SYMBOL_STR(pci_disable_msi) },
	{ 0xc1514a3b, __VMLINUX_SYMBOL_STR(free_irq) },
	{ 0xc364ae22, __VMLINUX_SYMBOL_STR(iomem_resource) },
	{ 0x2ea2c95c, __VMLINUX_SYMBOL_STR(__x86_indirect_thunk_rax) },
	{ 0xffaf4ef8, __VMLINUX_SYMBOL_STR(dma_ops) },
	{ 0xeae3dfd6, __VMLINUX_SYMBOL_STR(__const_udelay) },
	{ 0xb2fd5ceb, __VMLINUX_SYMBOL_STR(__put_user_4) },
	{ 0xb35f33bc, __VMLINUX_SYMBOL_STR(pci_write_config_byte) },
	{ 0xb2d9063f, __VMLINUX_SYMBOL_STR(pci_read_config_byte) },
	{ 0x4096a9cc, __VMLINUX_SYMBOL_STR(pci_read_config_word) },
	{ 0xdb3722a6, __VMLINUX_SYMBOL_STR(pci_read_config_dword) },
	{ 0xb44ad4b3, __VMLINUX_SYMBOL_STR(_copy_to_user) },
	{ 0x78e739aa, __VMLINUX_SYMBOL_STR(up) },
	{ 0x6dc6dd56, __VMLINUX_SYMBOL_STR(down) },
	{ 0x362ef408, __VMLINUX_SYMBOL_STR(_copy_from_user) },
	{ 0x8ddd8aad, __VMLINUX_SYMBOL_STR(schedule_timeout) },
	{ 0xced6bcd8, __VMLINUX_SYMBOL_STR(finish_wait) },
	{ 0x69e43cfd, __VMLINUX_SYMBOL_STR(prepare_to_wait_event) },
	{ 0xfe487975, __VMLINUX_SYMBOL_STR(init_wait_entry) },
	{ 0xa1c76e0a, __VMLINUX_SYMBOL_STR(_cond_resched) },
	{ 0x8ff4079b, __VMLINUX_SYMBOL_STR(pv_irq_ops) },
	{ 0xe5815f8a, __VMLINUX_SYMBOL_STR(_raw_spin_lock_irq) },
	{ 0x91715312, __VMLINUX_SYMBOL_STR(sprintf) },
	{ 0x5b205e34, __VMLINUX_SYMBOL_STR(__wake_up) },
	{ 0x5e331bf, __VMLINUX_SYMBOL_STR(try_module_get) },
	{ 0xf9a482f9, __VMLINUX_SYMBOL_STR(msleep) },
	{  0x4c7f4, __VMLINUX_SYMBOL_STR(module_put) },
	{ 0xce31b8a1, __VMLINUX_SYMBOL_STR(pv_lock_ops) },
	{ 0xe259ae9e, __VMLINUX_SYMBOL_STR(_raw_spin_lock) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "BDF20C79BCA4E3AD4F3F40B");

MODULE_INFO(suserelease, "openSUSE Leap 15.1");
