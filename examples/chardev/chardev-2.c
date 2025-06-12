#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/version.h> //For class_create()

#include "mychardev_common.h"

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johannes 4 GNU/Linux");
MODULE_DESCRIPTION("An example for registering device numbers for character devices");

// Device number assigned to this character device
static dev_t dev_num = MKDEV(MAJOR_NUM, MINOR_NUM);
static struct class *my_class;
// Character device object to register with the kernel
static struct cdev mychardev =
{
    .owner = THIS_MODULE,
};


// Define the file operations structure
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_close,
	.read = device_read,
	.write = device_write
};

static char *class_permissions_cb(const struct device *dev, umode_t *mode)
{
    if(mode != NULL)
    {
        *mode = 0666; // Set permissions to rw-rw-rw-
    }

    return NULL; // Return a string with the custom name of the device node, or NULL for the default device name
}

static int __init my_init(void)
{
    printk("---------------------- Mod Init ----------------------\n");
    printk("Initializing module: %s\n", THIS_MODULE->name);

    unsigned int num_of_dev = 1;

    // how many devices the cdev object should handle, starting from the dev_num base

    /****************************************************************************************
     * 1. Allocate/Register a range of device numbers. It does not deal with actual device
     * structure or file operations; its role is to associate a name with a device numbers
     * and register them with the kernel.
     *     - What does it mean to "register them with the kernel"?
     *       A: Informaing the Linux kernel about the existence of a particular range of
     *          device numbers that are associated with a char device. Basically tells
     *          the kernel "These device numbers (major/minor) are reserved for a specific
     *          driver".
     * 
     * register_chrdev_region(dev_t from, unsigned int count, const char *name):
     *      Use WHEN YOU KNOW the major number you want to assign to your char device.
     *      It allows you to register a fixed range of device numbers.
     * 
     *      dev_t from:
     *          The starting device number, which includes both the major and minor numbers.
     *          This is typically created using the MKDEV(major, minor) macro.
     *      unsigned int count:
     *          The number of consecutive device numbers to allocate. For example, if count
     *          is 5, you allocate minor numbers 0-4 for the given major number.
     *      const char *name:
     *          The name of the device, primarily for debugging purposes. This appears in
     *          /proc/devices.
     *      Limitations:
     *          You need to know the available major number in advance. If the major number
     *          is already in use, the function fails.
     *
     * alloc_chrdev_region():
     *      Use to DYNAMICALLY ALLOCATE a range of device numbers. The kernel assigns an
     *      available major number.
     * 
     *      dev_t *dev:
     *           A pointer to a dev_t variable where the allocated major and starting minor
     *           numbers are stored.
     *      unsigned int baseminor:
     *           The starting minor number. Usually set to 0 unless you need to allocate
     *           device numbers starting from a specific minor number.
     *      unsigned int count:
     *           The number of consecutive device numbers to allocate.
     *      const char *name:
     *            The name of the device, for debugging purposes.
     ****************************************************************************************/

    // Register the device. Creates new entry insidte file: /proc/devices
    // This will contain the driver name plus the Major Number
    if(register_chrdev_region(dev_num, RESERVED_CNT, DRIVER_NAME) < 0)
    {
		printk("%s - Error regiserting device number!\n", DRIVER_NAME);
		return -1;
	}

    /****************************************************************************
     * 2.0 cdev_init() + cdev_add()
     *     After these are called, a char device will be fully registerd and
     *     functional in the kernel however, there will be no corresponding device
     *     file in /dev (until device_create() or mknod are called), so user-space
     *     applications cannot acces or interact with the device.
     ****************************************************************************/

    /****************************************************************************
     * 2.1 Initialize a cdev object, linking it to the file operations for the device
     ****************************************************************************/

    // Setup the char device we want to use
    cdev_init(&mychardev, &fops);

    // num_of_dev should not exceed RESERVED_CNT
    if(num_of_dev > RESERVED_CNT)
    {
        printk("%s - Error attempting to handle more devices than reserved!\n", DRIVER_NAME);
        return -1;
    }

    /*****************************************************************************
     * 2.2 Register the cdev object with the kernel, associating it with the allocated
     * device numbers
     *****************************************************************************/

    if(cdev_add(&mychardev, dev_num, num_of_dev) < 0)
    {
        printk("Error: Could not add cdev\n");
        unregister_chrdev_region(dev_num, num_of_dev);
        return -1;
    }

    printk("Successfully registered device %s: Major-%u Minor-%u\n", DRIVER_NAME, MAJOR_NUM, MINOR_NUM);
    printk("\tCreated entry under: /proc/devices\n");

    /********************
     * 3.1 Create a sys class
     ********************/

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
        my_class = class_create(DRIVER_CLASS);
#else
        // Creates entry: /sys/class/DRIVER_CLASS
        my_class = class_create(THIS_MODULE, DRIVER_CLASS);
#endif

    // Set the devnode callback to set custom permissions
    my_class->devnode = class_permissions_cb;

    if(my_class == NULL)
    {
        printk("Error: Could not create class\n");
        cdev_del(&mychardev);
        unregister_chrdev_region(dev_num, num_of_dev);
        return -1;
    }
    printk("Successfully created class\n");
    printk("Created entry: /sys/class/%s\n", DRIVER_CLASS);

    /********************
     * 3.2 Create a node
     ********************/

    // Creates entry: /sys/class/DRIVER_CLASS/DRIVER_NODE.
    // User-space utilities like udev monitor sysfs to detect new devices and
    // dynamically create the corresponding /dev nodes: /dev/Mychar_Node
    device_create(my_class, NULL, dev_num, NULL, DRIVER_NODE);
    printk("Successfully created device node\n");
    printk("Created entry: /sys/class/%s/%s and /dev/%s\n", DRIVER_CLASS, DRIVER_NODE, DRIVER_NODE);

    printk("Successfully initialized module\n");
    printk("\tCreated entry under: /proc/modules\n");

    printk("------------------------------------------------------\n");
    return 0;   
}

static void __exit my_exit(void)
{
    printk("---------------------- Mod Exit ----------------------\n");
    printk("Removing module: %s\n", mychardev.owner->name);

    device_destroy(my_class, dev_num);
    class_destroy(my_class);

    // how many devices the cdev object should handle, starting from the dev_num base
    unsigned int num_of_dev = 1;

    unregister_chrdev_region(dev_num, num_of_dev);

    cdev_del(&mychardev);

    printk("Successfully un-registered Device number %u %u\n", MAJOR_NUM, MINOR_NUM);
    printk("------------------------------------------------------\n");
}

module_init(my_init);
module_exit(my_exit);

