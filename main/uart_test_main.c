/* UART Echo Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_partition.h"



/**
 * This is an example which echos any data it receives on configured UART back to the sender,
 * with hardware flow control turned off. It does not use UART driver event queue.
 *
 * - Port: configured UART
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: off
 * - Pin assignment: see defines below (See Kconfig)
 */

#define ECHO_TEST_TXD (CONFIG_EXAMPLE_UART_TXD)
#define ECHO_TEST_RXD (CONFIG_EXAMPLE_UART_RXD)
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)

#define ECHO_UART_PORT_NUM      (CONFIG_EXAMPLE_UART_PORT_NUM)
#define ECHO_UART_BAUD_RATE     (CONFIG_EXAMPLE_UART_BAUD_RATE)
#define ECHO_TASK_STACK_SIZE    (CONFIG_EXAMPLE_TASK_STACK_SIZE)

static const char *TAG = "UART TEST";

// static uint32_t test_sector = 1;
static uint32_t test_round =  0;
static uint32_t flash_tested_round = 0;

#define BUF_SIZE    (1024)
// #define BLOCK_SIZE  128


static void test_task(void *arg)
{
    /*
    * This example uses the partition table from ../partitions_example.csv. For reference, its contents are as follows:
    *
    *  nvs,        data, nvs,      0x9000,  0x6000,
    *  phy_init,   data, phy,      0xf000,  0x1000,
    *  factory,    app,  factory,  0x10000, 1M,
    *  storage,    data, ,             , 0x40000,
    */

    // Find the partition map in the partition table
    const esp_partition_t *partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "storage");
    assert(partition != NULL);

    // static char store_data[] = "ESP Partition Operations Test (Read, Erase, Write)";
    // static char read_data[BLOCK_SIZE];
    static char store_data[] = "ESP-IDF Partition Operations Test (Read, Erase, Write)";
    static char read_data[sizeof(store_data)];
    char uart_info[30];
    while (1) {
        // Erase entire partition
        // memset(read_data, 0xFF, sizeof(read_data));
        // ESP_ERROR_CHECK(esp_partition_erase_range(partition, 0, partition->size));
        // Erase the area where the data was written. Erase size shoud be a multiple of SPI_FLASH_SEC_SIZE
        // and also be SPI_FLASH_SEC_SIZE aligned
        if(flash_tested_round<test_round) {
            ESP_ERROR_CHECK(esp_partition_erase_range(partition, 0, SPI_FLASH_SEC_SIZE));
            // Read back the data (should all now be 0xFF's)
            memset(store_data, 0xFF, sizeof(read_data));
            ESP_ERROR_CHECK(esp_partition_read(partition, 0, read_data, sizeof(read_data)));
            // assert(memcmp(store_data, read_data, sizeof(read_data)) == 0);
            if (!memcmp(store_data, read_data, sizeof(read_data))) {
                ESP_LOGI(TAG, "Erase data err[%d/%d]: %s", flash_tested_round,test_round , read_data);
                sprintf(uart_info, "test erase err:%d/%d", flash_tested_round,test_round);
                uart_write_bytes(ECHO_UART_PORT_NUM, read_data, strlen(uart_info));
                test_round = 0;
            }

            // Write the data, starting from the beginning of the partition
            // memset(store_data, 0xaa, sizeof(store_data));
            ESP_ERROR_CHECK(esp_partition_write(partition, 0, store_data, sizeof(store_data)));
            // ESP_LOGI(TAG, "Written data: %s", store_data);

            // Read back the data, checking that read data and written data match
            ESP_ERROR_CHECK(esp_partition_read(partition, 0, read_data, sizeof(read_data)));
            if (!memcmp(store_data, read_data, sizeof(read_data))) {
                ESP_LOGI(TAG, "test write err[%d/%d]: %s", flash_tested_round,test_round , read_data);
                sprintf(uart_info, "test write err:%d/%d", flash_tested_round,test_round);
                uart_write_bytes(ECHO_UART_PORT_NUM, read_data, strlen(uart_info));
                test_round = 0;
            }
            // assert(memcmp(store_data, read_data, sizeof(read_data)) == 0);
            // ESP_LOGI(TAG, "Erased data");
            flash_tested_round++;
        }
    }
}


static void uart_task(void *arg)
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = ECHO_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif
    char uart_info[30];
    ESP_ERROR_CHECK(uart_driver_install(ECHO_UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(ECHO_UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(ECHO_UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS));

    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

    while (1) {
        // Read data from the UART
        int len = uart_read_bytes(ECHO_UART_PORT_NUM, data, (BUF_SIZE - 1), 20 / portTICK_PERIOD_MS);
        if (len) {
            data[len] = '\0';
            ESP_LOGI(TAG, "Recv Str[%d]: %s",len, (char *) data);
            if (memcmp((char *) data, "Flash sector ", 13)) {
                test_round = 0;
                flash_tested_round = 0;
                for (int i = 13; i < len; i++) {
                    if(data[i]>='0' && data[i]<='9'){
                        test_round *= 10;
                        test_round += data[i]-'0';
                    }
                }
                sprintf(uart_info, "ready test round:%d",test_round);
                uart_write_bytes(ECHO_UART_PORT_NUM, uart_info, strlen(uart_info));
            }
        }
    }
}



void app_main(void)
{
    xTaskCreate(uart_task, "uart_task", 2048, NULL, 5, NULL);
    xTaskCreate(test_task, "test_task", 4096, NULL, 5, NULL);
}
