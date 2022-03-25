// #include <assert.h>
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_spi_flash.h"
#include "esp_err.h"
// #include "esp_partition.h"


#define ECHO_TEST_TXD (CONFIG_EXAMPLE_UART_TXD)
#define ECHO_TEST_RXD (CONFIG_EXAMPLE_UART_RXD)
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)

#define ECHO_UART_PORT_NUM      (CONFIG_EXAMPLE_UART_PORT_NUM)
#define ECHO_UART_BAUD_RATE     (CONFIG_EXAMPLE_UART_BAUD_RATE)
#define ECHO_TASK_STACK_SIZE    (CONFIG_EXAMPLE_TASK_STACK_SIZE)
#define TEST_TASK_STACK_SIZE    (CONFIG_EXAMPLE_TEST_FLASH_SIZE)

#define BUF_SIZE        (100)
#define TEST_F_START    (0x40000)
// #define TEST_F_STOP     (0x40000)
// static const char *TAG = "TEST";
static int test_forever = 0;
static int test_break = 0;
static uint16_t sector_max = 1;
static uint16_t sector_num = 1;
static int32_t test_sector = 0;
static uint32_t test_round = 0;
static size_t size_flash_chip = 0;
static uint32_t flash_tested_round = 0;
static uint32_t sector_offset = 0;
static uint32_t addr_offset = TEST_F_START;

#define BLOCK_SIZE      SPI_FLASH_SEC_SIZE
static char test_data[BLOCK_SIZE];
static char test_buff[BLOCK_SIZE];

static IRAM_ATTR void test_task(void *arg)
{
    size_t size_flash_chip = spi_flash_get_chip_size();
    #ifdef TEST_F_STOP
    size_flash_chip = TEST_F_STOP;
    #endif
    sector_max = (size_flash_chip-TEST_F_START)/BLOCK_SIZE;
    // sector_max = (size_flash_chip)/BLOCK_SIZE;
    // char uart_info[BUF_SIZE];
    char *uart_info = (char *) malloc(BUF_SIZE);
    for (int i = 0; i < BLOCK_SIZE; i++) {
        test_data[i] = i%250 + 1;
    }
    memset(uart_info, 0x0, strlen(uart_info));
    sprintf(uart_info, "TEST FLASH [0x%x]-[%dKB]-[%d]",TEST_F_START,size_flash_chip /1024,sector_max);
    uart_write_bytes(ECHO_UART_PORT_NUM, uart_info, strlen(uart_info));
    while (1) {
        static uint16_t sector_cnt = sector_num;
        while(flash_tested_round<test_round) {
            while(sector_cnt--) {
                for (int i = 0; i < BLOCK_SIZE; i++) {
                    test_buff[i] = 0;
                }
                ESP_ERROR_CHECK(spi_flash_erase_range(addr_offset, BLOCK_SIZE));
                // Read back the data (should all now be 0xFF's)
                // ESP_ERROR_CHECK(esp_partition_read(partition, sector_offset, test_buff, sizeof(test_buff)));
                ESP_ERROR_CHECK(spi_flash_read(addr_offset, test_buff, sizeof(test_buff)));
                // assert(memcmp(store_data, read_data, sizeof(read_data)) == 0);
                for (int i = 0; i < sizeof(test_buff); i++) {
                    if(test_buff[i] != 0xFF){
                        // ESP_LOGI(TAG, "erase sector %d data err[%d/%d]: %d!=0xFF",test_sector, flash_tested_round,test_round , test_buff[i]);
                        memset(uart_info, 0x0, strlen(uart_info));
                        sprintf(uart_info, "[%d][0x%x] test %dc:erase err",test_sector,addr_offset,test_round);
                        // sprintf(uart_info, "test sector %d:erase err[%d/%d]",test_sector, flash_tested_round,test_round);
                        uart_write_bytes(ECHO_UART_PORT_NUM, uart_info, strlen(uart_info));
                        test_break = 1;
                        if(test_forever==0) break;
                    }
                }
                // memset(store_data, 0xaa, sizeof(store_data));
                // ESP_ERROR_CHECK(esp_partition_write(partition, sector_offset, test_data, sizeof(test_data)));
                ESP_ERROR_CHECK(spi_flash_write(addr_offset, test_data, sizeof(test_data)));
                // ESP_LOGI(TAG, "Written data: %s", store_data);

                // Read back the data, checking that read data and written data match
                // ESP_ERROR_CHECK(esp_partition_read(partition, sector_offset, test_buff, sizeof(test_buff)));
                ESP_ERROR_CHECK(spi_flash_read(addr_offset, test_buff, sizeof(test_buff)));
                for (int i = 0; i < sizeof(test_buff); i++) {
                    if(test_buff[i] != test_data[i]){
                        // ESP_LOGI(TAG, "write sector %d data err[%d/%d]: %d!=%d",test_sector, flash_tested_round,test_round , test_buff[i] , test_data[i]);
                        memset(uart_info, 0x0, strlen(uart_info));
                        sprintf(uart_info, "[%d][0x%x] test %dc:write err",test_sector,addr_offset,test_round);
                        // sprintf(uart_info, "test sector %d:write err[%d/%d]",test_sector, flash_tested_round,test_round);
                        uart_write_bytes(ECHO_UART_PORT_NUM, uart_info, strlen(uart_info));
                        test_break = 1;
                        if(test_forever==0) break;
                    }
                }
                flash_tested_round++;
                if(flash_tested_round%50==0){
                    vTaskDelay(2 / portTICK_PERIOD_MS);
                    if(test_forever) {
                        memset(uart_info, 0x0, strlen(uart_info));
                        sprintf(uart_info, "test all %d/%d:pass",flash_tested_round,test_round);
                        uart_write_bytes(ECHO_UART_PORT_NUM, uart_info, strlen(uart_info));
                    }
                }
            }
        }
        if(test_break==0) {
            memset(uart_info, 0x0, strlen(uart_info));
            sprintf(uart_info, "[%d][0x%x] tested:pass",test_sector,addr_offset,test_round);
            uart_write_bytes(ECHO_UART_PORT_NUM, uart_info, strlen(uart_info));
            else {
                test_sector++;
                sector_offset = test_sector*BLOCK_SIZE;
                addr_offset = TEST_F_START + sector_offset;
                flash_tested_round = 0;
            }
        }
        test_round = 0;
        flash_tested_round = 0;
        vTaskDelay(10 / portTICK_PERIOD_MS);

    }
}


static IRAM_ATTR void uart_task(void *arg)
{
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
    ESP_ERROR_CHECK(uart_driver_install(ECHO_UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(ECHO_UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(ECHO_UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS));

    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
    char *uart_info = (char *) malloc(BUF_SIZE);
    // char uart_info[BUF_SIZE];
    while (1) {
        // Read data from the UART
        int len = uart_read_bytes(ECHO_UART_PORT_NUM, data, (BUF_SIZE - 1), 20 / portTICK_PERIOD_MS);
        if (len>11) {
            data[len] = '\0';
            memset(uart_info, 0x0, strlen(uart_info));
            // ESP_LOGI(TAG, "Recv Str[%d]: %s",len , (char *) data);
            sprintf(uart_info, "RecvStr[%d]:%s",len , (char *) data);
            // ESP_LOGI(TAG,"%s",uart_info);
            uart_write_bytes(ECHO_UART_PORT_NUM, uart_info, strlen(uart_info));
            if (memcmp((char *) data, "test sector ", 12)==0) {
                // ESP_LOGI(TAG, "Recv[%d][%d][%d]: %s",len , data[13] , data[15] , (char *) data);
                flag = 0;
                test_round = 0;
                test_sector = 0;
                flash_tested_round = 0;
                for (int i = 12; i < len; i++) {
                    if(flag<=1 && data[i]>='0' && data[i]<='9'){
                        test_sector *= 10;
                        test_sector += data[i]-'0';
                        flag = 1;
                        sector_num = 1;
                        // ESP_LOGI(TAG, "sector [%d][%d]",test_sector,i);
                    }
                    else if(flag<=1 && (data[i]=='a' || data[i]=='A')){
                        test_sector = 0;
                        sector_num = sector_max;
                        // ESP_LOGI(TAG, "test sector all [%d]",test_sector);
                        flag = 2;
                    }
                    else if((flag==1 || flag==2) && data[i]==' '){
                        if(flag==1){
                            sector_offset = test_sector*BLOCK_SIZE;
                            addr_offset = TEST_F_START + sector_offset;
                        }
                        else {
                            sector_offset = 0;
                            addr_offset = TEST_F_START ;
                        }
                        flag = 3;
                    }
                    else if(flag>=3 && data[i]>='0' && data[i]<='9'){
                        test_round *= 10;
                        test_round += data[i]-'0';
                        flag = 4;
                        test_forever = 0;
                        // ESP_LOGI(TAG, "round [%d][%d]",test_round,i);
                    }
                    else if(data[i]=='a' && data[i+1]=='u' && data[i+2]=='t' && data[i+3]=='o'){
                        test_forever = 1;
                        sector_offset = 0;
                        addr_offset = TEST_F_START ;
                        test_round = 10000000;
                        break;
                    }
                    else if(flag==4){
                        // flag = 0;
                        // ESP_LOGI(TAG, "input round [%d][%d]",test_round,i);
                        break;
                    }
                }
                if(test_round){
                    if(test_sector>=(size_flash_chip-TEST_F_START)/BLOCK_SIZE){
                        sprintf(uart_info, "input sector %d invalid",test_sector);
                        test_sector = 0;
                        test_round = 0;
                        sector_num = 0;
                    }
                    else if(sector_num==1){
                        sprintf(uart_info, "ready test sector [%d][0x%x]:%dc",test_sector,addr_offset,test_round);
                    }
                    else sprintf(uart_info, "ready test all sector:%dc",test_round);
                    // ESP_LOGI(TAG,"%s",uart_info);
                    uart_write_bytes(ECHO_UART_PORT_NUM, uart_info, strlen(uart_info));
                }
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}



IRAM_ATTR void app_main(void)
{
    xTaskCreate(uart_task, "uart_task", ECHO_TASK_STACK_SIZE, NULL, 5, NULL);
    xTaskCreate(test_task, "test_task", 4096, NULL, 5, NULL);
}
