#include <iostream>
#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "driver/gpio.h"

#include "esp_firebase/app.h"
#include "esp_firebase/rtdb.h"

#include "wifi_utils.h"
#include "firebase_config.h"

using namespace ESPFirebase;

// ==========================================
// CONFIGURATION
// ==========================================
#define DEVICE_ID "ESP32_04" 

// Output Pins
#define RELAY_GPIO GPIO_NUM_4   // Relay (0 = ON, 1 = OFF)
#define LED_GPIO   GPIO_NUM_2   // Internal LED

// Input Pins (Sensors)
#define IR1_GPIO GPIO_NUM_27
#define IR2_GPIO GPIO_NUM_26
#define IR3_GPIO GPIO_NUM_25
#define IR4_GPIO GPIO_NUM_33
#define IR5_GPIO GPIO_NUM_32
#define IR6_GPIO GPIO_NUM_14
#define FLAME_GPIO GPIO_NUM_13 

static const char *TAG = "EV_STATION_INTEGRATED";

/* ------------ Hardware Initialization ------------ */
void hardware_init()
{
    // Configure Sensors as Inputs
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << IR1_GPIO) | (1ULL << IR2_GPIO) | 
                           (1ULL << IR3_GPIO) | (1ULL << IR4_GPIO) | 
                           (1ULL << IR5_GPIO) | (1ULL << IR6_GPIO) | 
                           (1ULL << FLAME_GPIO);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;   
    gpio_config(&io_conf);

    // Configure Relay and LED as Outputs
    gpio_reset_pin(RELAY_GPIO);
    gpio_set_direction(RELAY_GPIO, GPIO_MODE_OUTPUT);
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    // Initial States
    gpio_set_level(RELAY_GPIO, 0); // Default Relay ON
    gpio_set_level(LED_GPIO, 0);   // Default LED OFF
}

/* ------------ Main Application ------------ */
extern "C" void app_main(void)
{
    // 1. Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. WiFi Connect
    wifiInit(SSID, PASSWORD);
    ESP_LOGI(TAG, "WiFi connected. Device ID: %s", DEVICE_ID);

    // 3. Firebase Login
    user_account_t account = {USER_EMAIL, USER_PASSWORD};
    FirebaseApp app(API_KEY);
    app.loginUserAccount(account);
    RTDB db(&app, DATABASE_URL);

    // 4. Setup Hardware
    hardware_init();
    
    std::string path = "/devices/" + std::string(DEVICE_ID);

    while (true)
    {
        // --- STEP A: Fetch Control Data from Firebase ---
        Json::Value device_node = db.getData(path.c_str());

        // Control LED via "Sensor" key (1 = LED ON)
        // Check for both Integer and String formats to prevent errors
        if (device_node["Sensor"].asInt() == 1 || device_node["Sensor"].asString() == "1") {
            gpio_set_level(LED_GPIO, 1);
        } else {
            gpio_set_level(LED_GPIO, 0);
        }

        // Control Relay via "Shutdown" key (1 = Relay OFF)
        int shutdown_val = 0;
        if (device_node["Shutdown"].asInt() == 1 || device_node["Shutdown"].asString() == "1") {
            shutdown_val = 1;
            gpio_set_level(RELAY_GPIO, 1); // Logic HIGH turns Relay OFF
        } else {
            shutdown_val = 0;
            gpio_set_level(RELAY_GPIO, 0); // Logic LOW turns Relay ON
        }

        // --- STEP B: Read Sensor Data ---
        Json::Value sync_data;
        sync_data["ir1"] = (gpio_get_level(IR1_GPIO) == 0);
        sync_data["ir2"] = (gpio_get_level(IR2_GPIO) == 0);
        sync_data["ir3"] = (gpio_get_level(IR3_GPIO) == 0);
        sync_data["ir4"] = (gpio_get_level(IR4_GPIO) == 0);
        sync_data["ir5"] = (gpio_get_level(IR5_GPIO) == 0);
        sync_data["ir6"] = (gpio_get_level(IR6_GPIO) == 0);
        sync_data["flame_detected"] = (gpio_get_level(FLAME_GPIO) == 0);
        
        // --- STEP C: Maintain Controls in Firebase ---
        // We include these in putData so our manual dashboard changes aren't overwritten by the ESP32
        sync_data["Sensor"] = device_node["Sensor"];
        sync_data["Shutdown"] = shutdown_val;

        // --- STEP D: Sync to Firebase ---
        db.putData(path.c_str(), sync_data);

        ESP_LOGI(TAG, "Sync: LED=%d, Shutdown=%d, Path=%s", 
                 device_node["Sensor"].asInt(), shutdown_val, path.c_str());

        vTaskDelay(pdMS_TO_TICKS(3000)); 
    }
}