/*
 * hello-3.c - Illustrating the __init, __initdata and __exit macros.
 */
#include <linux/init.h> /* Needed for the macros */
#include <linux/module.h> /* Needed by all modules */
#include <linux/printk.h> /* Needed for pr_info() */

//The __initdata macro places data in seciont: .init.data
static int hello3_data __initdata = 3;

//The __init macro places data in seciont: .init.text.
//It causes the init function to be discarded and its
//memory freed once the init function finishes for built
//in (static) drivers, but not loadable modules. This is
//because for static drivers the Kernel konws re-initialization
//will not occur; you cannot load and unload a static module.
static int __init hello_3_init(void)
{
    pr_info("Hello, world %d\n", hello3_data);
    return 0;
}

//The __exit macro places data in seciont: .exit.text
//It causes the omission of the function when the module
//is built into the kernel. Has no effect for loadable
//modules.
static void __exit hello_3_exit(void)
{
    pr_info("Goodbye, world 3\n");
}

module_init(hello_3_init);
module_exit(hello_3_exit);

MODULE_LICENSE("GPL");
