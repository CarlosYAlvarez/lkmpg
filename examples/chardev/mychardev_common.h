#define MAJOR_NUM 74
#define MINOR_NUM 0
#define RESERVED_CNT 1                 // The number of consecutive device numbers to be reserved in the kernel
#define DRIVER_NAME "MyChar_Dev"       // Used by register_chrdev_region()
#define DRIVER_CLASS "MyChar_Class"      // Used by class_create()
#define DRIVER_NODE "MyChar_Node" // Used by device_create()

int device_open(struct inode* inode, struct file* file);
int device_close(struct inode* inode, struct file* file);
ssize_t device_read(struct file *file, char __user *user_buffer, size_t len, loff_t *off);
ssize_t device_write(struct file *file, const char __user *user_buffer, size_t len, loff_t *off);