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

static const char *TAG = "TEST";
static uint16_t sector_max = 1;
static uint16_t sector_num = 1;
static int32_t test_sector = 0;
static uint32_t test_round =  0;
static uint32_t flash_tested_round = 0;

#define BUF_SIZE    (1024)
#define BLOCK_SIZE   SPI_FLASH_SEC_SIZE
static char test_data[BLOCK_SIZE];
static char test_buff[BLOCK_SIZE];

static void test_task(void *arg)
{
    // Find the partition map in the partition table
    const esp_partition_t *partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "storage");
    assert(partition != NULL);
    // size_t size_flash_chip = spi_flash_get_chip_size();
    size_t size_flash_chip = 0x200000;
    sector_max = (size_flash_chip-0x50000)/BLOCK_SIZE;
    char uart_info[30];
    for (int i = 0; i < BLOCK_SIZE; i++) {
        test_data[i] = i%250 + 1;
    }

    uint32_t sector_offset = test_sector*BLOCK_SIZE;
    ESP_LOGI(TAG, "size_flash_chip = %d bytes,%d sector\n", size_flash_chip,sector_num);
    while (1) {
        // xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
        // uint32_t notify_value;
        // xTaskNotifyWait(0, 0xFFFFFFFF, &notify_value, portMAX_DELAY);
        if(test_round && flash_tested_round>=test_round) {
            sector_num --;
            memset(uart_info, 0x0, sizeof(uart_info));
            sprintf(uart_info, "TEST sector[%d] round[%d] OK",test_sector,test_round);
            ESP_LOGI(TAG, "%s",uart_info);
            uart_write_bytes(ECHO_UART_PORT_NUM, uart_info, strlen(uart_info));
            if(sector_num==0) {
                test_round = 0;
                flash_tested_round = 0;
            }
            else {
                test_sector++;
                sector_offset = test_sector*BLOCK_SIZE;
                flash_tested_round = 0;
            }
        }
        if(test_sector>sector_max) test_sector = 0;
        while(flash_tested_round<test_round) {
            for (int i = 0; i < BLOCK_SIZE; i++) {
                test_buff[i] = 0;
            }
            ESP_ERROR_CHECK(esp_partition_erase_range(partition, sector_offset, SPI_FLASH_SEC_SIZE));
            // Read back the data (should all now be 0xFF's)
            ESP_ERROR_CHECK(esp_partition_read(partition, sector_offset, test_buff, sizeof(test_buff)));
            // assert(memcmp(store_data, read_data, sizeof(read_data)) == 0);
            for (int i = 0; i < sizeof(test_buff); i++) {
                if(test_buff[i] != 0xFF){
                    ESP_LOGI(TAG, "erase sector %d data err[%d/%d]: %d!=0xFF",test_sector, flash_tested_round,test_round , test_buff[i]);
                    memset(uart_info, 0x0, sizeof(uart_info));
                    sprintf(uart_info, "test sector %d:erase err[%d/%d]",test_sector, flash_tested_round,test_round);
                    uart_write_bytes(ECHO_UART_PORT_NUM, uart_info, strlen(uart_info));
                    test_round = 0;
                }
            }
            // memset(store_data, 0xaa, sizeof(store_data));
            ESP_ERROR_CHECK(esp_partition_write(partition, sector_offset, test_data, sizeof(test_data)));
            // ESP_LOGI(TAG, "Written data: %s", store_data);

            // Read back the data, checking that read data and written data match
            ESP_ERROR_CHECK(esp_partition_read(partition, sector_offset, test_buff, sizeof(test_buff)));
            for (int i = 0; i < sizeof(test_buff); i++) {
                if(test_buff[i] != test_data[i]){
                    ESP_LOGI(TAG, "write sector %d data err[%d/%d]: %d!=%d",test_sector, flash_tested_round,test_round , test_buff[i] , test_data[i]);
                    memset(uart_info, 0x0, sizeof(uart_info));
                    sprintf(uart_info, "test sector %d:write err[%d/%d]",test_sector, flash_tested_round,test_round);
                    uart_write_bytes(ECHO_UART_PORT_NUM, uart_info, strlen(uart_info));
                    test_round = 0;
                }
            }
            flash_tested_round++;
            if(flash_tested_round%100==19) vTaskDelay(2 / portTICK_PERIOD_MS);
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
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
    static char flag = 0;
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
        if (len>12) {
            data[len] = '\0';
            ESP_LOGI(TAG, "Recv Str[%d]: %s",len , (char *) data);

            if (memcmp((char *) data, "Flash sector ", 13)==0 || memcmp((char *) data, "flash sector ", 13)==0) {
                // ESP_LOGI(TAG, "Recv[%d][%d][%d]: %s",len , data[13] , data[15] , (char *) data);
                flag = 0;
                test_round = 0;
                test_sector = 0;
                flash_tested_round = 0;
                for (int i = 13; i < len; i++) {
                    if(flag<=1 && data[i]>='0' && data[i]<='9'){
                        test_sector *= 10;
                        test_sector += data[i]-'0';
                        flag = 1;
                        sector_num = 1;
                        // ESP_LOGI(TAG, "sector [%d][%d]",test_sector,i);
                    }
                    else if(flag<=1 && data[i]=='a'){
                        test_sector = 0;
                        sector_num = sector_max;
                        ESP_LOGI(TAG, "test sector all [%d]",test_sector);
                        flag = 2;
                    }
                    else if((flag==1 || flag==2) && data[i]==' '){
                        flag = 3;
                    }
                    else if(flag>=3 && data[i]>='0' && data[i]<='9'){
                        test_round *= 10;
                        test_round += data[i]-'0';
                        flag = 4;
                        // ESP_LOGI(TAG, "round [%d][%d]",test_round,i);
                    }
                    else if(flag==4){
                        flag = 0;
                        // ESP_LOGI(TAG, "input round [%d][%d]",test_round,i);
                        break;
                    }
                }
                if(test_round){
                    if(test_sector>0){
                        sprintf(uart_info, "ready test sector %d:%d ",test_sector,test_round);
                    }
                    else sprintf(uart_info, "ready test all sector:%d ",test_round);
                    ESP_LOGI(TAG,"%s",uart_info);
                    uart_write_bytes(ECHO_UART_PORT_NUM, uart_info, strlen(uart_info));
                }
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}



void app_main(void)
{
    xTaskCreate(uart_task, "uart_task", ECHO_TASK_STACK_SIZE, NULL, 5, NULL);
    xTaskCreate(test_task, "test_task", 4096, NULL, 5, NULL);
}
