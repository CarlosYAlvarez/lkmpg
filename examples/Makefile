# The format of this should be:
#     obj-m += <module_name>.o # Create a module named <module_name>.ko using the following files:
#     <module_name>-objs += fileOne.o
#     <module_name>-objs += fileTwo.o
#     etc....
#
# Can also just do:
#     obj-m += <file_name>.o # Create a module with the same name as the file using the specified file.

obj-m += startstop.o
startstop-objs := start.o stop.o
obj-m += chardev.o
obj-m += procfs1.o
obj-m += procfs2.o
obj-m += procfs3.o
obj-m += procfs4.o
obj-m += hello-sysfs.o
obj-m += sleep.o
obj-m += print_string.o
obj-m += kbleds.o
obj-m += sched.o
obj-m += chardev2.o
obj-m += syscall-steal.o
obj-m += intrpt.o
obj-m += completions.o
obj-m += example_tasklet.o
obj-m += devicemodel.o
obj-m += example_spinlock.o
obj-m += example_rwlock.o
obj-m += example_atomic.o
obj-m += example_mutex.o
obj-m += bottomhalf.o
obj-m += bh_threaded.o
obj-m += ioctl.o
obj-m += vinput.o
obj-m += vkbd.o
obj-m += static_key.o

PWD := $(CURDIR)

ifeq ($(CONFIG_STATUS_CHECK_GCC),y)
CC=$(STATUS_CHECK_GCC)
ccflags-y += -fanalyzer
endif

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build CC=$(CC) M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build CC=$(CC) M=$(PWD) clean
	$(RM) other/cat_noblock *.plist

indent:
	clang-format -i *.[ch]
	clang-format -i other/*.[ch]
