#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
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
__used __section("__versions") = {
	{ 0x5200e57f, "module_layout" },
	{ 0x5d0c69bc, "i2c_unregister_device" },
	{ 0x4dbcdeb9, "i2c_del_driver" },
	{ 0x894c5b4f, "i2c_register_driver" },
	{ 0x5bb9e213, "i2c_put_adapter" },
	{ 0xfeca7710, "i2c_new_client_device" },
	{ 0xa4a6589, "i2c_get_adapter" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0x86332725, "__stack_chk_fail" },
	{ 0xfe8da975, "i2c_transfer_buffer_flags" },
	{ 0xc5850110, "printk" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("i2c:OLED_SSD1306");

MODULE_INFO(srcversion, "B5423306B76D7B30BEFC403");
