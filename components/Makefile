
PROJECT_NAME := hello_world

CC = xtensa-esp32s3-elf-gcc
AR = xtensa-esp32s3-elf-ar

LOCAL_CFLAGS = -mlongcalls -ffunction-sections -fdata-sections -Wall -Werror=all -ggdb \
                -fstrict-volatile-bitfields -fno-jump-tables -fno-tree-switch-conversion \
                -std=gnu99  -O2 \
                -I../include/ 
                

obj = hello_world.o

src = hello_world.c

hello_world.a: $(obj)
	$(AR) qc lib/libhello_world.a $(obj)
	rm *.o

$(obj):
	$(CC) $(LOCAL_CFLAGS) $(EXTRA_CFLAGS) -c $(src)

clean:
	rm lib/libhello_world.a

