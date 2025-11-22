# Extending the E-Paper Sample Project

This guide provides practical tips, code patterns, and examples for customizing the ESP32 E-Paper + LVGL sample project.

## Table of Contents

- [LVGL UI Customization](#lvgl-ui-customization)
- [Sensor Integration](#sensor-integration)
- [Power Management](#power-management)
- [Network Connectivity](#network-connectivity)
- [Different Display Sizes](#different-display-sizes)
- [Advanced LVGL Features](#advanced-lvgl-features)
- [Performance Optimization](#performance-optimization)

---

## LVGL UI Customization

### Creating Custom Layouts

LVGL uses a widget-based approach. Here are common patterns:

#### Multi-Column Layout

```cpp
void create_ui()
{
  lv_obj_t *scr = lv_screen_active();
  lv_obj_set_style_bg_color(scr, lv_color_white(), 0);
  
  // Left column
  lv_obj_t *label_left = lv_label_create(scr);
  lv_label_set_text(label_left, "Temperature:");
  lv_obj_set_style_text_color(label_left, lv_color_black(), 0);
  lv_obj_align(label_left, LV_ALIGN_LEFT_MID, 10, -20);
  
  lv_obj_t *label_temp_value = lv_label_create(scr);
  lv_label_set_text(label_temp_value, "23.5 C");
  lv_obj_set_style_text_color(label_temp_value, lv_color_black(), 0);
  lv_obj_align(label_temp_value, LV_ALIGN_LEFT_MID, 10, 0);
  
  // Right column
  lv_obj_t *label_right = lv_label_create(scr);
  lv_label_set_text(label_right, "Humidity:");
  lv_obj_set_style_text_color(label_right, lv_color_black(), 0);
  lv_obj_align(label_right, LV_ALIGN_RIGHT_MID, -10, -20);
  
  lv_obj_t *label_humid_value = lv_label_create(scr);
  lv_label_set_text(label_humid_value, "65%");
  lv_obj_set_style_text_color(label_humid_value, lv_color_black(), 0);
  lv_obj_align(label_humid_value, LV_ALIGN_RIGHT_MID, -10, 0);
}
```

#### Using Flexbox Layout

```cpp
void create_ui()
{
  lv_obj_t *scr = lv_screen_active();
  
  // Container with flex layout
  lv_obj_t *container = lv_obj_create(scr);
  lv_obj_set_size(container, 180, 150);
  lv_obj_center(container);
  lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  
  // Add items to container - they'll stack vertically
  lv_obj_t *label1 = lv_label_create(container);
  lv_label_set_text(label1, "Item 1");
  
  lv_obj_t *label2 = lv_label_create(container);
  lv_label_set_text(label2, "Item 2");
  
  lv_obj_t *label3 = lv_label_create(container);
  lv_label_set_text(label3, "Item 3");
}
```

#### Progress Bar

```cpp
// Create progress bar (global or in create_ui)
lv_obj_t *bar;

void create_ui()
{
  lv_obj_t *scr = lv_screen_active();
  
  bar = lv_bar_create(scr);
  lv_obj_set_size(bar, 150, 20);
  lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, -10);
  lv_bar_set_value(bar, 0, LV_ANIM_OFF);  // Start at 0%
}

void loop()
{
  if (update time) {
    // Update progress (0-100)
    int progress = calculate_progress();
    lv_bar_set_value(bar, progress, LV_ANIM_OFF);
    
    lv_refr_now(eink_get_display());
  }
}
```

#### Custom Fonts

To use different font sizes, enable them in `lv_conf.h`:

```cpp
// In lv_conf.h
#define LV_FONT_MONTSERRAT_24 1  // Enable 24pt font
```

Then use in code:

```cpp
lv_obj_t *big_label = lv_label_create(scr);
lv_label_set_text(big_label, "Big Text!");
lv_obj_set_style_text_font(big_label, &lv_font_montserrat_24, 0);
```

---

## Sensor Integration

### Temperature and Humidity (BME280)

```cpp
#include <Adafruit_BME280.h>

Adafruit_BME280 bme;
lv_obj_t *label_temp;
lv_obj_t *label_humid;

void setup()
{
  // ... existing setup ...
  
  // Initialize sensor
  if (!bme.begin(0x76)) {
    Serial.println("BME280 sensor not found!");
  }
  
  eink_init();
  create_ui();
  
  // Initial reading
  update_sensor_display();
}

void create_ui()
{
  lv_obj_t *scr = lv_screen_active();
  
  // Temperature display
  lv_obj_t *label_temp_title = lv_label_create(scr);
  lv_label_set_text(label_temp_title, "Temperature");
  lv_obj_align(label_temp_title, LV_ALIGN_TOP_MID, 0, 20);
  
  label_temp = lv_label_create(scr);
  lv_label_set_text(label_temp, "--.- C");
  lv_obj_set_style_text_font(label_temp, &lv_font_montserrat_20, 0);
  lv_obj_align(label_temp, LV_ALIGN_CENTER, 0, -20);
  
  // Humidity display
  label_humid = lv_label_create(scr);
  lv_label_set_text(label_humid, "Humidity: --%");
  lv_obj_align(label_humid, LV_ALIGN_CENTER, 0, 20);
}

void update_sensor_display()
{
  float temp = bme.readTemperature();
  float humid = bme.readHumidity();
  
  String temp_str = String(temp, 1) + " C";
  String humid_str = "Humidity: " + String((int)humid) + "%";
  
  lv_label_set_text(label_temp, temp_str.c_str());
  lv_label_set_text(label_humid, humid_str.c_str());
}

void loop()
{
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastUpdate >= UPDATE_INTERVAL) {
    update_sensor_display();
    eink_force_full_refresh();
    lv_refr_now(eink_get_display());
    
    lastUpdate = currentMillis;
  }
  
  delay(100);
}
```

### Battery Voltage Monitoring

```cpp
// ESP32 ADC pin for battery monitoring (voltage divider)
#define BATTERY_PIN 34  // Use ADC1 pins (32-39) for WiFi compatibility

float read_battery_voltage()
{
  // Read ADC value (0-4095 for 12-bit ADC)
  int adc_value = analogRead(BATTERY_PIN);
  
  // Convert to voltage (assuming 2:1 voltage divider)
  // Max ADC voltage = 3.3V, so max battery = 6.6V
  float voltage = (adc_value / 4095.0) * 3.3 * 2.0;
  
  return voltage;
}

void create_ui()
{
  // ... other UI elements ...
  
  // Battery indicator in corner
  lv_obj_t *label_battery = lv_label_create(scr);
  float battery_v = read_battery_voltage();
  String battery_str = String(battery_v, 2) + "V";
  lv_label_set_text(label_battery, battery_str.c_str());
  lv_obj_align(label_battery, LV_ALIGN_TOP_RIGHT, -5, 5);
  lv_obj_set_style_text_font(label_battery, &lv_font_montserrat_12, 0);
}
```

---

## Power Management

### Deep Sleep Between Updates

For battery-powered projects, use ESP32 deep sleep:

```cpp
#include <esp_sleep.h>

// Configuration
#define SLEEP_TIME_SECONDS 300  // Sleep for 5 minutes
#define uS_TO_S_FACTOR 1000000ULL

void setup()
{
  Serial.begin(115200);
  
  // Print wakeup reason
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
    Serial.println("Woke up from timer");
  } else {
    Serial.println("First boot");
  }
  
  // Initialize and update display
  eink_init();
  create_ui();
  update_display_content();
  
  // Refresh display
  lv_refr_now(eink_get_display());
  
  // Wait for display to finish
  delay(5000);
  
  // Configure timer wakeup
  esp_sleep_enable_timer_wakeup(SLEEP_TIME_SECONDS * uS_TO_S_FACTOR);
  
  Serial.println("Going to sleep...");
  Serial.flush();
  
  // Enter deep sleep
  esp_deep_sleep_start();
}

void loop()
{
  // Never reached - ESP32 resets on wakeup
}
```

**Power consumption with deep sleep:**
- Active (display update): ~50mA for 5 seconds
- Deep sleep: ~10µA (practically zero)
- Average with 5-minute intervals: ~15µA (months on battery!)

### RTC Memory for Persistent Data

Store data across sleep cycles:

```cpp
RTC_DATA_ATTR int boot_count = 0;  // Survives deep sleep
RTC_DATA_ATTR float last_temperature = 0.0;

void setup()
{
  boot_count++;
  Serial.print("Boot number: ");
  Serial.println(boot_count);
  
  // Read sensor
  float current_temp = bme.readTemperature();
  
  // Display change from last reading
  float temp_change = current_temp - last_temperature;
  String change_str = (temp_change > 0 ? "+" : "") + String(temp_change, 1) + " C";
  
  // Update for next cycle
  last_temperature = current_temp;
  
  // ... display and sleep ...
}
```

---

## Network Connectivity

### WiFi Connection

```cpp
#include <WiFi.h>

const char* ssid = "YourSSID";
const char* password = "YourPassword";

void setup()
{
  Serial.begin(115200);
  
  // Initialize display
  eink_init();
  create_ui();
  
  // Show "Connecting..." message
  lv_obj_t *status_label = lv_label_create(lv_screen_active());
  lv_label_set_text(status_label, "Connecting WiFi...");
  lv_obj_align(status_label, LV_ALIGN_CENTER, 0, 0);
  lv_refr_now(eink_get_display());
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    
    // Update display with IP
    String ip_str = "IP: " + WiFi.localIP().toString();
    lv_label_set_text(status_label, ip_str.c_str());
  } else {
    Serial.println("\nWiFi connection failed");
    lv_label_set_text(status_label, "WiFi Failed");
  }
  
  lv_refr_now(eink_get_display());
}
```

### HTTP Request Example

```cpp
#include <HTTPClient.h>

String fetch_weather()
{
  if (WiFi.status() != WL_CONNECTED) {
    return "No WiFi";
  }
  
  HTTPClient http;
  http.begin("http://api.openweathermap.org/data/2.5/weather?q=Brussels&appid=YOUR_API_KEY&units=metric");
  
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    String payload = http.getString();
    // Parse JSON (use ArduinoJson library)
    // Extract temperature, weather, etc.
    return payload;
  } else {
    Serial.print("HTTP Error: ");
    Serial.println(httpCode);
    return "Error";
  }
  
  http.end();
}
```

---

## Different Display Sizes

### Adapting to 2.13" Display (122×250)

1. **Update display constructor** in `eink_display.ino`:

```cpp
// Change from:
static GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(
  GxEPD2_154_D67(EINK_CS, EINK_DC, EINK_RST, EINK_BUSY)
);

// To:
static GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT> display(
  GxEPD2_213_BN(EINK_CS, EINK_DC, EINK_RST, EINK_BUSY)
);
```

2. **Update screen dimensions** in `eink_display.h`:

```cpp
#define SCREEN_WIDTH  122
#define SCREEN_HEIGHT 250
```

3. **Adjust UI layout** in `app.ino` for different aspect ratio:

```cpp
void create_ui()
{
  // 2.13" is portrait (tall), adjust positions accordingly
  lv_obj_align(label_hello, LV_ALIGN_TOP_MID, 0, 30);
  lv_obj_align(label_chip, LV_ALIGN_CENTER, 0, 0);
  lv_obj_align(label_counter, LV_ALIGN_BOTTOM_MID, 0, -30);
}
```

### Common Display Models

| Display Model | Resolution | GxEPD2 Class |
|---------------|------------|--------------|
| 1.54" B/W | 200×200 | `GxEPD2_154_D67` |
| 2.13" B/W | 122×250 | `GxEPD2_213_BN` |
| 2.9" B/W | 128×296 | `GxEPD2_290_BS` |
| 4.2" B/W | 400×300 | `GxEPD2_420` |

See [GxEPD2 examples](https://github.com/ZinggJM/GxEPD2/tree/master/examples) for complete list.

---

## Advanced LVGL Features

### Creating a Clock Display

```cpp
#include <time.h>

lv_obj_t *label_time;
lv_obj_t *label_date;

void setup_time()
{
  // Configure NTP
  configTime(3600, 3600, "pool.ntp.org");  // GMT+1 with DST
  
  // Wait for time sync
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  
  Serial.println(&timeinfo, "Time: %A, %B %d %Y %H:%M:%S");
}

void create_ui()
{
  lv_obj_t *scr = lv_screen_active();
  
  // Time (large font)
  label_time = lv_label_create(scr);
  lv_label_set_text(label_time, "00:00");
  lv_obj_set_style_text_font(label_time, &lv_font_montserrat_24, 0);
  lv_obj_align(label_time, LV_ALIGN_CENTER, 0, -20);
  
  // Date (small font)
  label_date = lv_label_create(scr);
  lv_label_set_text(label_date, "Mon, Jan 1");
  lv_obj_align(label_date, LV_ALIGN_CENTER, 0, 20);
}

void update_time_display()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }
  
  char time_str[6];
  strftime(time_str, sizeof(time_str), "%H:%M", &timeinfo);
  lv_label_set_text(label_time, time_str);
  
  char date_str[20];
  strftime(date_str, sizeof(date_str), "%a, %b %d", &timeinfo);
  lv_label_set_text(label_date, date_str);
}
```

### Drawing Lines and Shapes

```cpp
void create_ui()
{
  lv_obj_t *scr = lv_screen_active();
  
  // Horizontal line separator
  lv_obj_t *line1 = lv_line_create(scr);
  static lv_point_t line_points[] = {{10, 100}, {190, 100}};
  lv_line_set_points(line1, line_points, 2);
  lv_obj_set_style_line_color(line1, lv_color_black(), 0);
  lv_obj_set_style_line_width(line1, 2, 0);
}
```

---

## Performance Optimization

### Minimize Redraws

Only invalidate changed areas:

```cpp
void loop()
{
  if (update time) {
    // Update only the counter label
    lv_label_set_text(label_counter, new_text);
    
    // LVGL will only redraw the changed area
    lv_refr_now(eink_get_display());
  }
}
```

### Reduce Memory Usage

If running low on RAM:

1. **Reduce LVGL buffer** in `eink_display.ino`:
```cpp
static lv_color_t lv_disp_buf[SCREEN_WIDTH * 30] __attribute__((aligned(4)));
```

2. **Reduce LVGL heap** in `lv_conf.h`:
```cpp
#define LV_MEM_SIZE (32U * 1024U)  // 32KB instead of 64KB
```

3. **Disable unused widgets** in `lv_conf.h`:
```cpp
#define LV_USE_CHART 0
#define LV_USE_TABLE 0
// etc.
```

### Faster Partial Updates

Experiment with display driver settings in `eink_display.ino`:

```cpp
// Reduce waveform steps for faster updates (may increase ghosting)
display.init(115200, true, 2, false);  // Last param = false for fast mode
```

Or adjust partial window size for smaller updates.

---

## Best Practices

### 1. Always Set Colors Explicitly

E-ink needs explicit black/white:

```cpp
lv_obj_set_style_bg_color(obj, lv_color_white(), 0);
lv_obj_set_style_text_color(obj, lv_color_black(), 0);
```

### 2. Test Refresh Strategies

Different use cases need different strategies:

- **Clock/Temperature**: Partial updates every minute, full every hour
- **Status Display**: Full refresh on every change
- **Data Logger**: Partial for values, full when clearing screen

### 3. Monitor Memory Usage

```cpp
Serial.print("Free Heap: ");
Serial.println(ESP.getFreeHeap());

Serial.print("LVGL Memory: ");
lv_mem_monitor_t mon;
lv_mem_monitor(&mon);
Serial.println(mon.used_pct);
```

### 4. Use Serial Debugging

Add diagnostic output:

```cpp
#define DEBUG_PRINT(x) Serial.println(x)

void loop() {
  DEBUG_PRINT("[DEBUG] Updating display...");
  // ... code ...
}
```

### 5. Handle Init Failures Gracefully

```cpp
if (!sensor.begin()) {
  Serial.println("Sensor failed - using dummy data");
  // Display error message or use fallback
}
```

---

## Additional Resources

- **[LVGL Documentation](https://docs.lvgl.io/)** - Complete API reference
- **[LVGL Examples](https://github.com/lvgl/lvgl/tree/master/examples)** - Official examples
- **[GxEPD2 Library](https://github.com/ZinggJM/GxEPD2)** - Display driver documentation
- **[ESP32 Deep Sleep Guide](https://randomnerdtutorials.com/esp32-deep-sleep-arduino-ide-wake-up-sources/)** - Power management tutorial

---

**Happy coding!** If you create something cool, consider sharing it with the community! 🚀
