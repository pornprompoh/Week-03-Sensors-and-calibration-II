#include <stdio.h>
#include <math.h> // 1) เพิ่มไลบรารีนี้เพื่อใช้งานฟังก์ชันคณิตศาสตร์ exp()
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#define ADC_CHANNEL     ADC_CHANNEL_6    // GPIO34 สำหรับ ESP32
#define ADC_ATTEN       ADC_ATTEN_DB_12  // แก้ไขเป็น DB_12 ตามเวอร์ชันใหม่

void app_main(void)
{
    // 1) กำหนดค่าเริ่มต้นให้กับ ADC Unit 1
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    // 2) ตั้งค่า Channel (ความละเอียด และช่วงแรงดัน)
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL, &config));

    // 3) ตั้งค่า Calibration สำหรับแปลงค่า Raw เป็น Voltage (mV)
    adc_cali_handle_t cali_handle = NULL;
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    adc_cali_create_scheme_line_fitting(&cali_config, &cali_handle);

    while (1) {
        int raw = 0;
        int voltage = 0;

        // 4) อ่านค่า ADC แบบ Raw
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL, &raw));

        // 5) แปลงค่า Raw เป็นแรงดันไฟฟ้า (mV)
        if (cali_handle != NULL) {
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali_handle, raw, &voltage));
        }

        // 6) คำนวณค่า Lux จากสมการ Exponential: y = 0.8994 * e^(0.0017 * x)
        // ใช้ตัวแปร voltage แทนค่า x
        float lux = 0.8994 * exp(0.0017 * voltage);

        // แสดงผลลัพธ์ (แสดงทศนิยม 2 ตำแหน่งสำหรับค่า Lux)
        printf("ADC Raw = %d, Voltage = %d mV, Calculated Lux = %.2f\n", raw, voltage, lux);
        
        // หน่วงเวลา 500 ms (0.5 วินาที)
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}