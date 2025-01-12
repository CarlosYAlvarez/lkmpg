#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define DEVICE_PATH "/dev/simple_mmap"
#define MAP_SIZE 4096  // Must match the kernel module's map size

int main()
{
    int fd;
    char *mapped;

    // Open the device file
    fd = open(DEVICE_PATH, O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("Failed to open device");
        return EXIT_FAILURE;
    }

    // Memory map the device
    mapped = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped == MAP_FAILED) {
        perror("Failed to mmap");
        close(fd);
        return EXIT_FAILURE;
    }

    // Write to the mapped memory
    sprintf(mapped, "Hello from user space!");
    printf("Message written: %s\n", mapped);

    // Cleanup
    munmap(mapped, MAP_SIZE);
    close(fd);
    return EXIT_SUCCESS;
}
