#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

struct ioctl_arg {
    unsigned int val;
};

/*
#define IOCTL_NAME _IO(type, number)
#define IOCTL_NAME _IOW(type, number, data_type) // Write Operation: User space sends data to the kernel.
#define IOCTL_NAME _IOR(type, number, data_type) // Read Operation: Kernel sends data to user space.

type (IOC_MAGIC):
    * A unique identifier (typically a single character like '\x66') to prevent conflicts between IOCTLs from different drivers.
    * It helps the kernel distinguish IOCTLs from your driver versus other drivers.

number:
    * An identifier for the specific IOCTL command. Each command gets a unique number.
    * In the macros, 0, 1, 2, and 3 are the unique command numbers.

data_type:
    * The type of data to be passed between user space and kernel space.
    * Examples:
        * struct ioctl_arg for complex data structures.
        *int for integers.
*/

#define IOC_MAGIC '\x66'
#define IOCTL_VALSET _IOW(IOC_MAGIC, 0, struct ioctl_arg) // Sets a value in the kernel using struct ioctl_arg
#define IOCTL_VALGET _IOR(IOC_MAGIC, 1, struct ioctl_arg) // Gets a value from the kernel into struct ioctl_arg
#define IOCTL_VALGET_NUM _IOR(IOC_MAGIC, 2, int) // Gets a plain integer from the kernel
#define IOCTL_VALSET_NUM _IOW(IOC_MAGIC, 3, int) // Sets an integer value in the kernel

int main() {
    int fd = open("/dev/ioctltest", O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    struct ioctl_arg arg = { .val = 0xAB };
    if (ioctl(fd, IOCTL_VALSET, &arg) == 0) {
        printf("Value set to: 0x%x\n", arg.val);
    }

    arg.val = 0;
    if (ioctl(fd, IOCTL_VALGET, &arg) == 0) {
        printf("Retrieved value: 0x%x\n", arg.val);
    }

    int num = 42;
    if (ioctl(fd, IOCTL_VALSET_NUM, num) == 0) {
        printf("Set ioctl_num to: %d\n", num);
    }

    if (ioctl(fd, IOCTL_VALGET_NUM, &num) == 0) {
        printf("Retrieved ioctl_num: %d\n", num);
    }

    close(fd);
    return 0;
}
