#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <settings.h>
#include "esp_wpa2.h"

#define DEST_FS_USES_SPIFFS
#include <ESP32-targz.h>

// esp32 sdk imports
#include "esp_heap_caps.h"
#include "esp_log.h"

// epd
#include "epd_driver.h"
#include "epd_highlevel.h"

#include "esp32/rom/tjpgd.h"

// battery
#include <driver/adc.h>
#include "esp_adc_cal.h"

// deepsleep
#include "esp_sleep.h"

// font
#include "Firasans.h"

#define BATT_PIN 36

#define WAVEFORM EPD_BUILTIN_WAVEFORM

gpio_num_t FIRST_BTN_PIN = GPIO_NUM_39;
gpio_num_t SECOND_BTN_PIN = GPIO_NUM_34;

EpdiyHighlevelState hl;
// ambient temperature around device
int temperature = 20;
uint8_t *fb;
enum EpdDrawError err;

int vref = 1100;

void start_deep_sleep()
{
    Serial.println("Sending device to deepsleep");
    epd_poweroff();
    delay(400);
    esp_sleep_enable_ext0_wakeup(FIRST_BTN_PIN, 0);
    esp_deep_sleep_start();
}

double_t get_battery_percentage()
{
    // When reading the battery voltage, POWER_EN must be turned on
    epd_poweron();
    delay(50);

    //Serial.println(epd_ambient_temperature());

    uint16_t v = analogRead(BATT_PIN);
    //Serial.print("Battery analogRead value is");
    //Serial.println(v);
    double_t battery_voltage = ((double_t)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);

    // Better formula needed I suppose
    // experimental super simple percent estimate no lookup anything just divide by 100
    double_t percent_experiment = ((battery_voltage - 3.7) / 0.5) * 100;

    // cap out battery at 100%
    // on charging it spikes higher
    if (percent_experiment > 100) {
        percent_experiment = 100;
    }

    String voltage = "Battery Voltage :" + String(battery_voltage) + "V which is around " + String(percent_experiment) + "%";
    //Serial.println(voltage);

    epd_poweroff();
    delay(50);

    return percent_experiment;
}

/**
 * Correct the ADC reference voltage. Was in example of lilygo epd47 repository to calc battery percentage
*/
void correct_adc_reference()
{
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        //Serial.printf("eFuse Vref:%u mV", adc_chars.vref);
        vref = adc_chars.vref;
    }
}

void display_battery() {
    int battery_cursor_x = EPD_WIDTH - 30;
    int battery_cursor_y = EPD_HEIGHT - 20;

    EpdFontProperties battery_font_props = epd_font_properties_default();
    battery_font_props.flags = EPD_DRAW_ALIGN_RIGHT;
    String battery_text = String(get_battery_percentage());
    battery_text.concat("% Battery");
    epd_write_string(&FiraSans_12, battery_text.c_str(), &battery_cursor_x, &battery_cursor_y, fb, &battery_font_props);
}

int gz_ptr = 0;
bool GZ_CallBack(unsigned char *buff, size_t buffsize)
{
    memcpy(fb+gz_ptr, buff, buffsize);
    gz_ptr+=buffsize;
    return true;
}

void display_img(int img) {
    String server = "http://disp.azurewebsites.net/api/image?id="+String(img)+"&bin=1&gz=1";
      
    HTTPClient http;    
    http.begin(server);
    http.useHTTP10();
    http.setTimeout(15000);
    int httpResponseCode = http.GET();
    int size = http.getSize();
    int len = (960*540)/2;
    Serial.println("Image size: " + String(size) + ", saved " + String(100-(100*float(size))/len) + "%");
    if (httpResponseCode > 0) {
        WiFiClient * stream = http.getStreamPtr();
        
        gz_ptr = 0;
        GzUnpacker *GZUnpacker = new GzUnpacker();
        GZUnpacker->haltOnError( true );
        GZUnpacker->setGzProgressCallback( BaseUnpacker::targzNullProgressCallback );
        GZUnpacker->setLoggerCallback (BaseUnpacker::targzNullLoggerCallback );
        GZUnpacker->setStreamWriter(GZ_CallBack);

        Serial.println("Getting screen #" + String(img));
        if( !GZUnpacker->gzStreamExpander( stream, size ) ) {
            Serial.printf("tarGzStreamUpdater failed with return code #%d\n", GZUnpacker->tarGzGetError() );
        }
    }
    else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }
    http.end();
}

bool useEduroam = false;
void ConnectWifi()
{
    if ( useEduroam )
    {
        WiFi.disconnect(true);
        WiFi.mode(WIFI_STA);

        esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_ID, strlen(EAP_ID));
        esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_USERNAME, strlen(EAP_USERNAME));
        esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));
    
        WiFi.enableSTA(true);
        esp_wifi_sta_wpa2_ent_enable();

        WiFi.begin("eduroam"); 
    }
    else
    {
        WiFi.begin(ESP_WIFI_SSID, ESP_WIFI_PASSWORD);
    }

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");        
    }
}

void setup() {
    Serial.begin(115200);

    correct_adc_reference();

    epd_init(EPD_OPTIONS_DEFAULT);
    hl = epd_hl_init(WAVEFORM);
    epd_set_rotation(EPD_ROT_LANDSCAPE);
    fb = epd_hl_get_framebuffer(&hl);
    epd_clear();

    ConnectWifi();

    pinMode(SECOND_BTN_PIN, INPUT);
    attachInterrupt(SECOND_BTN_PIN, start_deep_sleep, HIGH);
}

int img = 0;
void loop()
{
    display_img(img);
    display_battery();
    epd_poweron();
    err = epd_hl_update_screen(&hl, MODE_GC16, temperature);
    delay(500);
    epd_poweroff();
    delay(1000);
    img += 1;
}