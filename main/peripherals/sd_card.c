#include "sd_card.h"

#include "system_config.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"

/*********************************************************************
 * STATIC VARS
 *********************************************************************/

static const char *TAG = "SD CARD";

/*********************************************************************
 * PRIVATE FUNCTIONS
 *********************************************************************/

/* Test writing function */
esp_err_t s_example_write_file(const char *path, char *data)
{
    ESP_LOGI(TAG, "Opening file %s", path);
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    fprintf(f, data);
    fclose(f);
    ESP_LOGI(TAG, "File written");

    return ESP_OK;
}

/* Test reading function */
esp_err_t s_example_read_file(const char *path)
{
    ESP_LOGI(TAG, "Reading file %s", path);
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    char line[SD_MAX_CHAR_SIZE];
    fgets(line, sizeof(line), f);
    fclose(f);

    // strip newline
    char *pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);

    return ESP_OK;
}

/*********************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************/

static void init() {
    esp_err_t ret;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16*1024
    };

    sdmmc_card_t *card;
    const char *mount_point = SD_MOUNT_POINT;
    ESP_LOGI(TAG, "Initializing SD card");
    ESP_LOGI(TAG, "Using SPI peripheral");
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.max_freq_khz = SDMMC_FREQ_PROBING;
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = SD_SPI_MOSI_GPIO_NUM,
        .miso_io_num = SD_SPI_MISO_GPIO_NUM,
        .sclk_io_num = SD_SPI_CLK_GPIO_NUM,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };


    ret = spi_bus_initialize((spi_host_device_t)host.slot, &bus_cfg, 2);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return;
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = (gpio_num_t)SD_SPI_CS_GPIO_NUM;
    slot_config.host_id = (spi_host_device_t)host.slot;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                        "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                        "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }
    ESP_LOGI(TAG, "Filesystem mounted");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);

    // Read test file
    const char *file_version = "/version.txt";
    char *file_path = (char*)malloc(strlen(SD_MOUNT_POINT) + strlen(file_version));
    strcpy(file_path, SD_MOUNT_POINT);
    strcat(file_path, file_version);
    // const char *file_version = strcat(SD_MOUNT_POINT, "/version.txt");
    // const char *file_version = MOUNT_POINT"/version.txt";
    ret = s_example_read_file(file_path);
    if (ret != ESP_OK) {
        return;
    }
}

/*********************************************************************
 * PUBLIC INTERFACE
 *********************************************************************/

const struct SdCard sd_card = {
    .init = init
};
