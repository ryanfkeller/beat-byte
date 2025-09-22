#include "esp_log.h"

#include "gui.h"
#include "ui.h"
#include "sd_card.h"


static const char *TAG = "MAIN";

// /*********************************************************************
//  * MAIN APPLICATION
//  *********************************************************************/

void app_main(void) {
    ESP_LOGI(TAG, "*** Beat-Byte Main Starting ***");
    gui.init();
    create_ui();
    sd_card.init();
}


