#include <linux/module.h>

#include "mychardev_common.h"

#define SUCCESS 0
#define FAILURE -1
#define BUF_LEN 80 /* Max length of the message from the device */

static char msg[BUF_LEN + 1]; /* The msg the device will give when asked */

#define BUFFER_SIZE 1024

static char device_buffer[BUFFER_SIZE]; // Internal buffer for the device
static size_t current_buffer_len = 0;   // Tracks the current size of data in the buffer
static size_t device_buffer_idx = 0;    // The last position that was written to the buffer

enum {
    CDEV_NOT_USED,
    CDEV_EXCLUSIVE_OPEN,
};
/* Is device open? Used to prevent multiple access to device */
static atomic_t already_open = ATOMIC_INIT(CDEV_NOT_USED);

/* Called when a process tries to open the device file, like
 * "sudo cat /dev/chardev"
 */
int device_open(struct inode* inode, struct file* file)
{
    static unsigned int counter = 0;

    // If file is not oppened (NOT_USED), set to EXCLUSIVE_OPEN
    // if arg1 == arg2; then set arg1 to arg3 AND Return arg2 else return arg1
    // Compare and Swap, prevent concurrent access to shared resources
    if (atomic_cmpxchg(&already_open, CDEV_NOT_USED, CDEV_EXCLUSIVE_OPEN))
        return -EBUSY;

    if(try_module_get(THIS_MODULE))
    {
        // printk("Module %s: Opening file %u times\n", THIS_MODULE->name, counter++);
        // printk("\tRef count: %u\n", THIS_MODULE->refcnt.counter);
        return SUCCESS;
    }

    return FAILURE;
}

/* Called when a process closes the device file. */
int device_close(struct inode* inode, struct file* file)
{
    /* We're now ready for our next caller */
    atomic_set(&already_open, CDEV_NOT_USED);
    
    /* Decrement the usage count, or else once you opened the file, you will
     * never get rid of the module.
     */
    module_put(THIS_MODULE);

    // printk("Module %s: Closing file\n", THIS_MODULE->name);
    // printk("\tRef count: %u\n", THIS_MODULE->refcnt.counter);

    return 0;
}
/**
 * @brief Read data out of the buffer
 * @param file Represents the open file instance on which the write operation is
 *             being performed
 * @param user_buf Pointer to the user-space buffere containing the data to be
 *                 written to the file
 * @param count Maximum number of bytes the user-space program is requesting to
 *              read from this device.
 *              Note: This number can be very large at the start, why? Tools like
 *              cat and the read() system call generally use a large buffer to
 *              minimize the number of system calls. This buffer size is typically
 *              4 KB (4096 bytes) on most systems.
 * @param pos Pointer to the current file offset, indicates where in the file the
 *            read operation should start. Does *pos = 0 (always) at the start?
 * @return This method should return the number of bytes read.
 *         Returning 0 signals to cat command that it has reached the end of file.
 *         When EOF is reached, cat stops calling device_read() and exits, printing
 *         the contents of the fil.
 * 
 *         In the first call, bytes_to_read will be returned. Because this is a non
 *         zero value, device_read() will be called again. In the second call, 0 will
 *         be returned (becuase pos was update during the first call).
 */
ssize_t device_read(struct file *file, char __user *user_buf, size_t count, loff_t *pos)
{
    size_t bytes_to_read;

    // Check if we've already read everything
    if (*pos >= current_buffer_len)
        return 0;  // cat will continue calling device_read() until it receives a 0, signaling EOF

    // Determine how many bytes to read
    bytes_to_read = min(count, (current_buffer_len - *pos) );

    // Copy data from kernel space buffer (device_buffer) to user space buffer (user_buf)
    if (copy_to_user(user_buf, &device_buffer[*pos], bytes_to_read))
    {
        pr_err("Failed to copy data from kernel space\n");
        return -EFAULT;
    }

    pr_info("%s: Read %zu bytes from the device\n", DRIVER_NAME, bytes_to_read);
    *pos += bytes_to_read;  // Update the file offset

    return bytes_to_read;  // Return the number of bytes read
}

/**
 * @brief Write data to buffer
 * @param file Represents the open file instance on which the write operation is
 *             being performed
 * @param user_buf Pointer to the user-space buffere containing the data to be
 *                 written to the file
 * @param count The number of bytes to write from the user buffer to the file
 * @param pos Pointer to the current file offset, indicates where in teh file the
 *            write operation should start
 * @return Returns the number of bytes written
 */
ssize_t device_write(struct file *file, const char __user *user_buf, size_t count, loff_t *pos)
{
    size_t bytes_to_write = 0;

    // Ensure we don't exceed the buffer size
    if (device_buffer_idx >= BUFFER_SIZE)
    {
        pr_err("Attempting to write at offset %lu, no more memory space left on buffer\n", device_buffer_idx);
        return -ENOMEM;  // No space left in the buffer
    }

    // Determine the number of bytes we can write.
    // Write either the full amount (count), or up to the space left in the buffer.
    bytes_to_write = min(count, (size_t)(BUFFER_SIZE - device_buffer_idx));

    // Copy data from user space buffer (user_buf) to kernel space buffer (device_buffer)
    if (copy_from_user( &device_buffer[device_buffer_idx], user_buf, bytes_to_write) )
    {
        pr_err("Failed to copy data from user space\n");
        return -EFAULT;  // Return error if copy fails
    }
    pr_info("Wrote %zu bytes to device buffer at offset %lld\n", bytes_to_write, device_buffer_idx);

    device_buffer_idx += bytes_to_write;  // Update the buffer offset. This is so that for consecutive writes the buffer isn't overwritten.
    current_buffer_len += bytes_to_write;

    return bytes_to_write;  // Return the number of bytes written
}
