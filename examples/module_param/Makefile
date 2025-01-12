MODULE_NAME = my_module_param
obj-m += $(MODULE_NAME).o

PWD := $(CURDIR)

ifeq ($(CONFIG_STATUS_CHECK_GCC),y)
CC=$(STATUS_CHECK_GCC)
ccflags-y += -fanalyzer
endif

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	echo $(PWD)


clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build CC=$(CC) M=$(PWD) clean
	$(RM) other/cat_noblock *.plist

install_mod:
	insmod $(PWD)/$(MODULE_NAME).ko

remove_mod:
	rmmod $(MODULE_NAME)

indent:
	clang-format -i *.[ch]
	clang-format -i other/*.[ch]
