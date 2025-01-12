/*
 * hello-2.c - Demonstrating the module_init() and module_exit() macros.
 * This is preferred over using my_init() and my_exit().
 */
#include <linux/init.h> /* Needed for the macros */
#include <linux/module.h> /* Needed by all modules */
#include <linux/printk.h> /* Needed for pr_info() */

static long myvar_1 = 0;
module_param(myvar_1, long, 0);
MODULE_PARM_DESC(myvar_1, "Variable of type long");

static int myvar_2 = 0;
module_param(myvar_2, int, 0);
MODULE_PARM_DESC(myvar_2, "Variable of type int");

static int myintarray[2] = {0.0, 0.0};
static int myintarray_argc = 0;
module_param_array(myintarray, int, &myintarray_argc, 0000);
MODULE_PARM_DESC(myintarray, "Array (size 2) of integers");

static int __init my_init(void)
{
    pr_info("Initializing module\n");
    pr_info("Value of myvar_1 is: %li\n", myvar_1);
    pr_info("Value of myvar_2 is: %i\n", myvar_2);

    int i = 0;
    for(i = 0; i < 2; ++i)
    {
        pr_info("Value of myintarray is: %d\n", myintarray[i]);
    }

    pr_info("%i indices have been un-initialized\n", myintarray_argc);

    return 0;
}

static void __exit my_exit(void)
{
    pr_info("Cleaning up module\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
