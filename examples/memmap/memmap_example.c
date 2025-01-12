#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/io.h>

#define DEVICE_NAME "simple_mmap"
#define DEVICE_MAJOR 94
#define MAP_SIZE PAGE_SIZE  // Size of memory mapping (one page)

// Pointer for memory to be mapped
static char *mapped_memory;

// mmap function
static int mmap_example_mmap(struct file *filp, struct vm_area_struct *vma)
{
    unsigned long pfn;
    unsigned long length = vma->vm_end - vma->vm_start;

    if (length > MAP_SIZE) {
        printk(KERN_ERR "Requested memory size exceeds limit\n");
        return -EINVAL;
    }

    // Get the page frame number (PFN) of the memory to map
    pfn = virt_to_phys(mapped_memory) >> PAGE_SHIFT;

    // Remap the memory to user space
    if (remap_pfn_range(vma, vma->vm_start, pfn, length, vma->vm_page_prot)) {
        printk(KERN_ERR "Failed to map memory to user space\n");
        return -EAGAIN;
    }

    printk(KERN_INFO "Memory mapped to user space\n");
    return 0;
}

// Open function (for completeness, though not mandatory here)
static int mmap_example_open(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "Device opened\n");
    return 0;
}

// Release function (for completeness)
static int mmap_example_release(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "Device closed\n");
    return 0;
}

// File operations
static const struct file_operations mmap_example_fops = {
    .owner = THIS_MODULE,
    .open = mmap_example_open,
    .release = mmap_example_release,
    .mmap = mmap_example_mmap,
};

// Module initialization
static int __init mmap_example_init(void)
{
    int ret;

    // Allocate memory to be mapped
    mapped_memory = (char *)kmalloc(MAP_SIZE, GFP_KERNEL);
    if (!mapped_memory) {
        printk(KERN_ERR "Failed to allocate memory\n");
        return -ENOMEM;
    }

    memset(mapped_memory, 0, MAP_SIZE);

    // Register character device
    ret = register_chrdev(DEVICE_MAJOR, DEVICE_NAME, &mmap_example_fops);
    if (ret < 0) {
        printk(KERN_ERR "Failed to register device\n");
        kfree(mapped_memory);
        return ret;
    }

    printk(KERN_INFO "Module loaded: /dev/%s with major %d\n", DEVICE_NAME, DEVICE_MAJOR);
    return 0;
}

// Module cleanup
static void __exit mmap_example_exit(void)
{
    unregister_chrdev(DEVICE_MAJOR, DEVICE_NAME);
    kfree(mapped_memory);
    printk(KERN_INFO "Module unloaded\n");
}

module_init(mmap_example_init);
module_exit(mmap_example_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple mmap example kernel module");
