#include "ota_module.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"

static bool isValidFirmware(const esp_partition_t* partition) {
    esp_image_header_t header;
    if (esp_partition_read(partition, 0, &header, sizeof(header)) != ESP_OK) return false;
    return (header.magic == ESP_IMAGE_HEADER_MAGIC);
}

void switchToFactory() {
    const esp_partition_t* factory = esp_partition_find_first(
        ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY, nullptr
    );
    
    if (!factory || esp_ota_get_running_partition() == factory) return;
    if (!isValidFirmware(factory)) return;
    
    if (esp_ota_set_boot_partition(factory) == ESP_OK) {
        Serial.println("[OTA] → Factory. Restart...");
        delay(500);
        esp_restart();
    }
}