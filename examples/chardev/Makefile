MODULE_NAME_ONE = MyCharDevMod
MODULE_NAME_TWO = MyCharDevModNod

obj-m += $(MODULE_NAME_ONE).o
$(MODULE_NAME_ONE)-objs += mychardev_mod.o
$(MODULE_NAME_ONE)-objs += mychardev_common.o

obj-m += $(MODULE_NAME_TWO).o
$(MODULE_NAME_TWO)-objs += mychardevnod_mod.o
$(MODULE_NAME_TWO)-objs += mychardev_common.o

PWD := $(CURDIR)

ifeq ($(CONFIG_STATUS_CHECK_GCC),y)
CC=$(STATUS_CHECK_GCC)
ccflags-y += -fanalyzer
endif

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build CC=$(CC) M=$(PWD) clean
	$(RM) other/cat_noblock *.plist

indent:
	clang-format -i *.[ch]
	clang-format -i other/*.[ch]
