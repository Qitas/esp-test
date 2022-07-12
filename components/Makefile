
PROJECT_NAME := hello_world

CC = xtensa-esp32s3-elf-gcc
AR = xtensa-esp32s3-elf-ar

LOCAL_CFLAGS = -mlongcalls -ffunction-sections -fdata-sections -Wall -Werror=all -ggdb \
                -fstrict-volatile-bitfields -fno-jump-tables -fno-tree-switch-conversion \
                -std=gnu99  -O2 \
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
                

obj = hello_world.o

src = hello_world.c

hello_world.a: $(obj)
	$(AR) qc lib/libhello_world.a $(obj)
	rm *.o

$(obj):
	$(CC) $(LOCAL_CFLAGS) $(EXTRA_CFLAGS) -c $(src)

clean:
	rm lib/libhello_world.a

