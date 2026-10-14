#include <Arduino.h>
#include <esp_display_panel.hpp>
#include <esp_err.h>
#include <lvgl.h>
#include "esp_lv_adapter_arduino.h"
#include "ui.h" // Your SquareLine export
#include <ui_Screen1.h>
#include "display_helper/display_helper.h"

using namespace esp_panel::drivers;
using namespace esp_panel::board;

DisplayVars GlobalDisplayData;

// update display with current data
void setDisplay() {
    lv_arc_set_value(ui_SpeedDial, GlobalDisplayData.Speed);
    lv_label_set_text_fmt(ui_SpeedText, "%d",GlobalDisplayData.Speed);
    
    lv_bar_set_value(ui_BatteryBar, GlobalDisplayData.Battery, LV_ANIM_OFF);
    lv_bar_set_value(ui_SOCText, GlobalDisplayData.Soc, LV_ANIM_OFF);

    lv_label_set_text_fmt(ui_Battery_Text, "%.1f %%", GlobalDisplayData.Battery);
    lv_label_set_text_fmt(ui_SOCText, "%.1f %%", GlobalDisplayData.Soc);
    lv_label_set_text_fmt(ui_TimeText, "%s", GlobalDisplayData.Time.c_str());
    lv_label_set_text_fmt(ui_MinCellText, "%.2f V", GlobalDisplayData.Min_Cell);
    lv_label_set_text_fmt(ui_MaxCellText, "%.2f V", GlobalDisplayData.Max_Cell);
    lv_label_set_text_fmt(ui_MinPackText, "%.2f V", GlobalDisplayData.Min_Pack);
    lv_label_set_text_fmt(ui_MaxPackText, "%.2f V", GlobalDisplayData.Max_Pack);
    lv_label_set_text_fmt(ui_PackText, "%.2f V", GlobalDisplayData.Pack);
    lv_label_set_text_fmt(ui_MotorText, " %.2f V", GlobalDisplayData.Motor);
    lv_label_set_text_fmt(ui_SolarText, "%.2f V", GlobalDisplayData.Solar);
    lv_label_set_text_fmt(ui_SpeedText, "%d", GlobalDisplayData.Speed);
}

void setup() {
    Serial.begin(115200);
    Serial.println("Booting SquareLine UI with ESP Panel...");

    Board *board = new Board();
    if ((board == nullptr) || !board->init()) {
        Serial.println("Board init failed");
        while (true) delay(1000);
    }

    const esp_lv_adapter_rotation_t rotation = ESP_LV_ADAPTER_ROTATE_0;
    const esp_lv_adapter_tear_avoid_mode_t tear_mode = ESP_LV_ADAPTER_TEAR_AVOID_MODE_DEFAULT_RGB;
    const uint8_t frame_buffer_count = esp_lv_adapter_get_required_frame_buffer_count(tear_mode, rotation);

    LCD *lcd = board->getLCD();
    if (lcd == nullptr) {
        Serial.println("LCD device is not available");
        while (true) delay(1000);
    }
    auto *lcd_bus = lcd->getBus();
    if (lcd_bus->getBasicAttributes().type == ESP_PANEL_BUS_TYPE_RGB) {
        lcd->configFrameBufferNumber(frame_buffer_count);
        static_cast<BusRGB *>(lcd_bus)->configRGB_BounceBufferSize(lcd->getFrameWidth() * 10);
    }

    board->begin();

    esp_lv_adapter_config_t adapter_config = ESP_LV_ADAPTER_DEFAULT_CONFIG();
    adapter_config.task_stack_size = 12 * 1024;
    adapter_config.task_priority = 2;
    adapter_config.task_core_id = ARDUINO_RUNNING_CORE;
    ESP_ERROR_CHECK(esp_lv_adapter_init(&adapter_config));

    esp_lv_adapter_display_config_t disp_config = ESP_LV_ADAPTER_DISPLAY_RGB_DEFAULT_CONFIG(
        lcd, lcd->getFrameWidth(), lcd->getFrameHeight(), rotation
    );
    disp_config.profile.use_psram = true;

    lv_display_t *disp = esp_lv_adapter_register_display(&disp_config);

    if (board->getTouch() != nullptr) {
        esp_lv_adapter_touch_config_t touch_config = ESP_LV_ADAPTER_TOUCH_DEFAULT_CONFIG(disp, board->getTouch());
        esp_lv_adapter_register_touch(&touch_config);
    }

    ESP_ERROR_CHECK(esp_lv_adapter_start());

    // INJECT SQUARELINE UI HERE
    ESP_ERROR_CHECK(esp_lv_adapter_lock(-1));
    ui_init();
    esp_lv_adapter_unlock();

    Serial.println("SquareLine UI Ready!");
}

void loop()
{
    // 1. Generate a fake ticking clock for the Time string
    static int minutes = 12;
    static int seconds = 0;
    seconds = (seconds + 1) % 60;
    if (seconds == 0) minutes = (minutes + 1) % 24;
    char timeBuf[10];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", minutes, seconds);
    GlobalDisplayData.Time = std::string(timeBuf);

    // 2. Increment and wrap Floats
    // Sweeps Battery from 10.0V to 14.0V
    GlobalDisplayData.Battery = (GlobalDisplayData.Battery > 14.0f) ? 10.0f : GlobalDisplayData.Battery + 0.1f;
    // Sweeps SOC from 0% to 100%
    GlobalDisplayData.Soc = (GlobalDisplayData.Soc > 100.0f) ? 0.0f : GlobalDisplayData.Soc + 1.0f;

    // 3. Increment and wrap Doubles
    // Sweeps Cell Voltages from 3.0V to 4.2V
    GlobalDisplayData.Min_Cell = (GlobalDisplayData.Min_Cell > 4.2) ? 3.0 : GlobalDisplayData.Min_Cell + 0.02;
    GlobalDisplayData.Max_Cell = (GlobalDisplayData.Max_Cell > 4.2) ? 3.0 : GlobalDisplayData.Max_Cell + 0.02;

    // Sweeps Pack Voltages from 80.0V to 120.0V
    GlobalDisplayData.Min_Pack = (GlobalDisplayData.Min_Pack > 120.0) ? 80.0 : GlobalDisplayData.Min_Pack + 0.5;
    GlobalDisplayData.Max_Pack = (GlobalDisplayData.Max_Pack > 120.0) ? 80.0 : GlobalDisplayData.Max_Pack + 0.5;
    GlobalDisplayData.Pack = (GlobalDisplayData.Pack > 120.0) ? 80.0 : GlobalDisplayData.Pack + 0.5;

    // Sweeps Motor Temp from 20C to 80C
    GlobalDisplayData.Motor = (GlobalDisplayData.Motor > 80.0) ? 20.0 : GlobalDisplayData.Motor + 1.5;

    // Sweeps Solar output from 0W to 500W
    GlobalDisplayData.Solar = (GlobalDisplayData.Solar > 500.0) ? 0.0 : GlobalDisplayData.Solar + 5.0;

    // 4. Increment and wrap Integer
    // Sweeps Speed from 0 to 100
    GlobalDisplayData.Speed = (GlobalDisplayData.Speed + 1) % 105;
    setDisplay();
    delay(200);
}