/*
 * ioctl.c
 */
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/version.h>

struct ioctl_arg {
    unsigned int val;
};

/* Documentation/userspace-api/ioctl/ioctl-number.rst */
#define IOC_MAGIC '\x66'

#define IOCTL_VALSET _IOW(IOC_MAGIC, 0, struct ioctl_arg)
#define IOCTL_VALGET _IOR(IOC_MAGIC, 1, struct ioctl_arg)
#define IOCTL_VALGET_NUM _IOR(IOC_MAGIC, 2, int)
#define IOCTL_VALSET_NUM _IOW(IOC_MAGIC, 3, int)

#define IOCTL_VAL_MAXNR 3
#define DRIVER_NAME "ioctltest"

static unsigned int MAJOR_NUM = 0;
static unsigned int RESERVED_CNT = 1; // The number of consecutive device numbers to be reserved in the kernel
static struct cdev test_ioctl_cdev;
static int ioctl_num = 0;

struct test_ioctl_data {
    unsigned char val;
    rwlock_t lock;
};

/*
 * Provides custom IOCTL commands for user-space interaction.
 * 
 * @param cmd - The ioctl number. This encodes the major device number, the type of the ioctl,
 *              the command, and the type of the parameter
 */
static long test_ioctl_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct test_ioctl_data *ioctl_data = filp->private_data;
    int retval = 0;
    unsigned char val;
    struct ioctl_arg data;
    memset(&data, 0, sizeof(data));

    switch (cmd) {
    case IOCTL_VALSET:
        if (copy_from_user(&data, (int __user *)arg, sizeof(data))) {
            retval = -EFAULT;
            goto done;
        }

        pr_alert("IOCTL set val:%x .\n", data.val);
        write_lock(&ioctl_data->lock);
        ioctl_data->val = data.val;
        write_unlock(&ioctl_data->lock);
        break;

    case IOCTL_VALGET:
        read_lock(&ioctl_data->lock);
        val = ioctl_data->val;
        read_unlock(&ioctl_data->lock);
        data.val = val;

        if (copy_to_user((int __user *)arg, &data, sizeof(data))) {
            retval = -EFAULT;
            goto done;
        }

        break;

    case IOCTL_VALGET_NUM:
        retval = __put_user(ioctl_num, (int __user *)arg);
        break;

    case IOCTL_VALSET_NUM:
        ioctl_num = arg;
        break;

    default:
        retval = -ENOTTY;
    }

done:
    return retval;
}

/*
 * The test_ioctl_read function is called when a user-space application attempts to read data from
 * the character device associated with the driver. Specifically, it is invoked when the read()
 * system call is used on the device file (e.g., /dev/ioctltest).
 */
static ssize_t test_ioctl_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    pr_alert("%s call.\n", __func__);
    struct test_ioctl_data *ioctl_data = filp->private_data;
    unsigned char val;
    int retval;
    int i = 0;

    read_lock(&ioctl_data->lock);
    val = ioctl_data->val;
    read_unlock(&ioctl_data->lock);

    for (; i < count; i++) {
        if (copy_to_user(&buf[i], &val, 1)) {
            retval = -EFAULT;
            goto out;
        }
    }

    retval = count;
out:
    return retval;
}

static int test_ioctl_close(struct inode *inode, struct file *filp)
{
    pr_alert("%s call.\n", __func__);

    if (filp->private_data) {
        kfree(filp->private_data);
        filp->private_data = NULL;
    }

    return 0;
}

static int test_ioctl_open(struct inode *inode, struct file *filp)
{
    struct test_ioctl_data *ioctl_data;

    pr_alert("%s call.\n", __func__);
    ioctl_data = kmalloc(sizeof(struct test_ioctl_data), GFP_KERNEL);

    if (ioctl_data == NULL)
        return -ENOMEM;

    rwlock_init(&ioctl_data->lock);
    ioctl_data->val = 0xFF;
    filp->private_data = ioctl_data;

    return 0;
}

static struct file_operations fops = {
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 4, 0)
    .owner = THIS_MODULE,
#endif
    .open = test_ioctl_open,
    .release = test_ioctl_close,
    .read = test_ioctl_read,
    .unlocked_ioctl = test_ioctl_ioctl,
};

static int __init ioctl_init(void)
{
    printk("---------------------- Mod Init ----------------------\n");
    printk("Initializing module: %s\n", THIS_MODULE->name);

    dev_t dev;
    int alloc_ret = -1;
    int cdev_ret = -1;
    
    /************************************
     * Allocate a range of device numbers
     ************************************/

    // Register the device, Creates new entry insidte file: /proc/devices
    alloc_ret = alloc_chrdev_region(&dev, 0, RESERVED_CNT, DRIVER_NAME);

    if (alloc_ret)
        goto error;

    MAJOR_NUM = MAJOR(dev);

    /****************************************************************************
     * Initialize a cdev object, linking it to the file operations for the device
     ****************************************************************************/

    // Setup the char device we want to use
    cdev_init(&test_ioctl_cdev, &fops);

    /*****************************************************************************
     * Register the cdev object with the kernel, associating it with the allocated
     * device numbers
     *****************************************************************************/
    cdev_ret = cdev_add(&test_ioctl_cdev, dev, RESERVED_CNT);

    if (cdev_ret)
        goto error;

    printk("Successfully registered device %s: Major-%u Minor-%u\n", DRIVER_NAME, MAJOR(dev), MINOR(dev));
    printk("\tCreated %s entry under: /proc/devices\n", DRIVER_NAME);

    printk("Successfully initialized module\n");
    printk("\tCreated entry under: /proc/modules\n\n");

    printk("Manually create a device file: sudo mknod /dev/%s c %u %u\n", DRIVER_NAME, MAJOR(dev), MINOR(dev));

    printk("------------------------------------------------------\n");

    return 0;

error:
    if (cdev_ret == 0)
        cdev_del(&test_ioctl_cdev);
    if (alloc_ret == 0)
        unregister_chrdev_region(dev, RESERVED_CNT);

    printk("------------------------------------------------------\n");
    return -1;
}

static void __exit ioctl_exit(void)
{
    dev_t dev = MKDEV(MAJOR_NUM, 0);

    cdev_del(&test_ioctl_cdev);
    unregister_chrdev_region(dev, RESERVED_CNT);
    pr_alert("%s driver removed.\n", DRIVER_NAME);
}

module_init(ioctl_init);
module_exit(ioctl_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("This is test_ioctl module");
