
PROJECT_NAME := hello_world_lib

CC = xtensa-esp32s3-elf-gcc
AR = xtensa-esp32s3-elf-ar

LOCAL_CFLAGS = -mlongcalls -ffunction-sections -fdata-sections -Wall -Werror=all -ggdb -O2  \
                -fstrict-volatile-bitfields -fno-jump-tables -fno-tree-switch-conversion -std=gnu99 \
                -I../include/ \
                -I../build/config/ \
               -I$(IDF_PATH)/components/newlib/platform_include/ \
               -I$(IDF_PATH)/components/freertos/include/ \
               -I$(IDF_PATH)/components/freertos/include/esp_additions/freertos/ \
               -I$(IDF_PATH)/components/freertos/port/xtensa/include/ \
               -I$(IDF_PATH)/components/esp_hw_support/include/ \
               -I$(IDF_PATH)/components/heap/include/ \
               -I$(IDF_PATH)/components/soc/include/ \
               -I$(IDF_PATH)/components/soc/esp32s3/include/ \
               -I$(IDF_PATH)/components/hal/include/ \
               -I$(IDF_PATH)/components/hal/esp32s3/include/ \
               -I$(IDF_PATH)/components/esp_rom/include/ \
               -I$(IDF_PATH)/components/esp_common/include/ \
               -I$(IDF_PATH)/components/esp_system/include/ \
               -I$(IDF_PATH)/components/xtensa/include/ \
               -I$(IDF_PATH)/components/xtensa/esp32s3/include/ \
               -I$(IDF_PATH)/components/esp_timer/include/ 
                


obj = hello_world_lib.o

src = hello_world_lib.c

hello_world_lib.a: $(obj)
	$(AR) qc libhello_world_lib.a $(obj)
	rm *.o

$(obj):
	$(CC) $(LOCAL_CFLAGS) $(EXTRA_CFLAGS) -c $(src)

clean:
	rm hello_world_lib.a

