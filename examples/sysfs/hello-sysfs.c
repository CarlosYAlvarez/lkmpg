/*
 * hello-sysfs.c sysfs example
 */
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/sysfs.h>

#define SYSFS_DIR "mymodule"
static struct kobject *mymodule_kernobj;

/* the variable you want to be able to change */
static int myvariable = 0;

/*
 * Called when the file is read. E.g. cat
 */
static ssize_t myvariable_show(struct kobject *kobj,
                               struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "Value of stored variable is: %d\n", myvariable);
}

/*
 * Called when the file is written to. E.g. echo
 */
static ssize_t myvariable_store(struct kobject *kobj,
                                struct kobj_attribute *attr, const char *buf,
                                size_t count)
{
    sscanf(buf, "%d", &myvariable);
    return count;
}
/*
 * __ATTR essentially expands the struct kobj_attribute into:
 * .attr =
 * {
 *     .name = #myvariable
 *     .mode = 0660
 * }
 * .show = myvariable_show
 * .store = myvariable_store
 */
static struct kobj_attribute myvariable_attribute =
    __ATTR(myvariable, 0660, myvariable_show, myvariable_store);

static int __init mymodule_init(void)
{
    int error = 0;
    pr_info("Initializing module %s\n", THIS_MODULE->name);

    /*
     * Creates a kboject name SYSFS_DIR and attaches it to kernel_kobj.
     *
     * As a result, a DIRECTORY name /sys/kernel/SYSFS_DIR is created
     * in the sysfs file system
     * 
     * What is a kernel object?
     *   A kernel object (represented by kobject in the Linux kernel) is
     *   a fundamental building block used to manage and represent kernel
     *   data structures and resources in a structured and hierarchical
     *   way. It provides a unified way to expose kernel information and
     *   interact with kernel components, often via the sysfs virtual
     *   filesystem.
     */
    mymodule_kernobj = kobject_create_and_add(SYSFS_DIR, kernel_kobj);
    if (!mymodule_kernobj)
    {
        pr_info("Failed to initialize module %s\n", THIS_MODULE->name);
        return -ENOMEM;
    }

    pr_info("Successfully created dir: /sys/kernel/%s\n", SYSFS_DIR);
    pr_info("\tAttached new dir %s to kernel obj name %s\n", SYSFS_DIR, kernel_kobj->name);
    pr_info("\tNew kernel object created with name %s\n", mymodule_kernobj->name);

    /*
     * Creates a FILE named "myvariable" in sysfs associated with a
     * specific attribute of a kobject.
     * 
     * As a result this new file will be created under /sys/kernel/SYSFS_DIR.
     *
     * The file is linked to the myvariable_attribute object, which defines
     * how the file behaves.
     */
    error = sysfs_create_file(mymodule_kernobj, &myvariable_attribute.attr);
    if (error) {
        kobject_put(mymodule_kernobj);
        pr_info("failed to create the myvariable file in /sys/kernel/%s\n", SYSFS_DIR);
    }

    pr_info("Successfully created new file /sys/kernel/%s/%s associated with kernel object %s\n", SYSFS_DIR, myvariable_attribute.attr.name, mymodule_kernobj->name);

    return error;
}

static void __exit mymodule_exit(void)
{
    pr_info("%s: Exit success\n", THIS_MODULE->name);

    /*
     * Removes SYSFS_DIR directory and its associated resources
     */
    kobject_put(mymodule_kernobj);
}

module_init(mymodule_init);
module_exit(mymodule_exit);

MODULE_LICENSE("GPL");
