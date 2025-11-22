/*
 * ESP32 + E-Paper Display + LVGL Sample Project
 * 
 * This sample demonstrates best practices for integrating e-ink displays
 * with LVGL (Light and Versatile Graphics Library) on ESP32.
 * 
 * KEY CONCEPTS:
 * - E-ink displays are bistable (retain image without power)
 * - Slow refresh rates (2-5s full, 0.5-2s partial)
 * - Partial updates are faster but cause ghosting over time
 * - Manual refresh control saves power vs continuous polling
 * 
 * WHAT THIS DEMO SHOWS:
 * - Simple counter UI with chip information
 * - Hybrid refresh strategy (partial + periodic full refresh)
 * - Memory-efficient LVGL integration
 * - Manual refresh control for low power operation
 * 
 * HARDWARE:
 * - ESP32 Dev Module
 * - WeAct Studio 1.54" E-Paper Module (GDEH0154D67, 200x200, B/W)
 * - See docs/pinout.md for wiring details
 * 
 * LIBRARIES:
 * - GxEPD2: E-ink display driver
 * - LVGL: Graphics library
 * - Adafruit GFX: Graphics primitives
 */

#include "../version.h"
#include "eink_display.h"

// ===== UI OBJECTS =====
static lv_obj_t *label_counter;  // Only counter needs to be global (updated in loop)
static lv_obj_t *progress_bar;   // Progress bar (updated in loop)

// ===== APP STATE =====
unsigned long lastUpdate = 0;
const unsigned long UPDATE_INTERVAL = 2500; // Update every 2.5 seconds
uint32_t update_counter = 0;

// ===== CREATE UI =====
// Creates the LVGL user interface:
// 1. Title label at top
// 2. Chip info in center
// 3. Counter label (updated in loop)
// 4. Progress bar at bottom (updated in loop, cycles 0-10)
//
// LVGL BASICS:
// - All widgets are objects (lv_obj_t*)
// - Objects have parents (screen is root)
// - Objects deleted automatically when parent is deleted
// - For e-ink, always set colors explicitly (black/white)
void create_ui()
{
  Serial.println("[UI] Creating interface...");
  
  // Get active screen (root object for all widgets)
  lv_obj_t *scr = lv_screen_active();
  
  // Set screen background to white (important for e-ink!)
  lv_obj_set_style_bg_color(scr, lv_color_white(), 0);
  
  // Title label (local - never modified after creation)
  lv_obj_t *label_hello = lv_label_create(scr);
  lv_label_set_text(label_hello, "Hello LVGL!");
  lv_obj_set_style_text_color(label_hello, lv_color_black(), 0);
  lv_obj_set_style_text_font(label_hello, &lv_font_montserrat_32, 0); 
  lv_obj_align(label_hello, LV_ALIGN_TOP_MID, 0, 20);
  
  // Chip info label (local - never modified after creation)
  lv_obj_t *label_chip = lv_label_create(scr);
  String chip_info = String(ESP.getChipModel()) + "\n" + 
                     String(ESP.getCpuFreqMHz()) + " MHz";
  lv_label_set_text(label_chip, chip_info.c_str());
  lv_obj_set_style_text_color(label_chip, lv_color_black(), 0);
  lv_obj_set_style_text_align(label_chip, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(label_chip, LV_ALIGN_CENTER, 0, -20);
  
  // Counter label (moved up to make room for bar)
  label_counter = lv_label_create(scr);
  lv_label_set_text(label_counter, "Counter: 0");
  lv_obj_set_style_text_color(label_counter, lv_color_black(), 0);
  lv_obj_set_style_text_font(label_counter, &lv_font_montserrat_24, 0); 
  lv_obj_align(label_counter, LV_ALIGN_BOTTOM_MID, 0, -60);
  
  // Progress bar (black background, white indicator for e-ink)
  static lv_style_t style_bg;
  static lv_style_t style_indic;
  
  lv_style_init(&style_bg);
  lv_style_set_border_color(&style_bg, lv_color_black());
  lv_style_set_border_width(&style_bg, 2);
  lv_style_set_pad_all(&style_bg, 4);
  lv_style_set_radius(&style_bg, 4);
  lv_style_set_bg_color(&style_bg, lv_color_white());
  
  lv_style_init(&style_indic);
  lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);
  lv_style_set_bg_color(&style_indic, lv_color_black());
  lv_style_set_radius(&style_indic, 2);
  
  progress_bar = lv_bar_create(scr);
  lv_obj_remove_style_all(progress_bar);
  lv_obj_add_style(progress_bar, &style_bg, 0);
  lv_obj_add_style(progress_bar, &style_indic, LV_PART_INDICATOR);
  
  lv_obj_set_size(progress_bar, 180, 20);
  lv_obj_align(progress_bar, LV_ALIGN_BOTTOM_MID, 0, -20);
  lv_bar_set_range(progress_bar, 0, 10);
  lv_bar_set_value(progress_bar, 0, LV_ANIM_OFF); // No animation for e-ink
  
  Serial.println("[UI] Interface created");
}

void setup()
{
  // Initialize serial communication
  Serial.begin(115200);
  delay(1000);
  
  // Print startup message
  Serial.println("\n=== ESP32 E-INK + LVGL Demo ===");
  printVersionInfo();
  Serial.println();
  Serial.print("Chip Model: ");
  Serial.println(ESP.getChipModel());
  Serial.print("Chip Revision: ");
  Serial.println(ESP.getChipRevision());
  Serial.print("CPU Frequency: ");
  Serial.print(ESP.getCpuFreqMHz());
  Serial.println(" MHz");
  Serial.print("Flash Size: ");
  Serial.print(ESP.getFlashChipSize() / (1024 * 1024));
  Serial.println(" MB");
  Serial.println("==================================\n");
  
  // Initialize e-ink display and LVGL
  eink_init();
  
  // Create UI
  create_ui();
  
  // Initial render
  Serial.println("[LVGL] Performing initial render...");
  lv_refr_now(eink_get_display());
  
  Serial.println("\n[READY] System initialized");
  Serial.print("[INFO] Updates every ");
  Serial.print(UPDATE_INTERVAL / 1000.0);
  Serial.println(" seconds with partial refresh");
  Serial.print("[INFO] Full refresh every ");
  Serial.print(MAX_PARTIAL_UPDATES);
  Serial.println(" e-ink flushes to clear ghosting");
  Serial.println("[INFO] (Multiple widgets = multiple flushes per cycle)\n");
  
  lastUpdate = millis();
}

void loop()
{
  unsigned long currentMillis = millis();
  
  // ===== MANUAL REFRESH CONTROL =====
  // We control when the display updates (not automatic LVGL timer).
  // This is KEY for e-ink power efficiency:
  // - Only refresh when content changes
  // - Avoid continuous polling (lv_task_handler)
  // - Display retains image without power between updates
  //
  // For battery operation, you could use deep sleep between updates
  // and achieve <50µA average current!
  if (currentMillis - lastUpdate >= UPDATE_INTERVAL) {
    update_counter++;
    
    // Use modulo to cycle 0-10
    uint32_t bar_value = update_counter % 11;
    
    // Update counter label
    String counter_text = "Counter: " + String(update_counter);
    lv_label_set_text(label_counter, counter_text.c_str());
    
    // Update progress bar
    lv_bar_set_value(progress_bar, bar_value, LV_ANIM_OFF);
    
    // Check if we need a full refresh to clear ghosting
    eink_force_full_refresh();
    
    // Trigger LVGL refresh
    Serial.print("[UPDATE] Refresh #");
    Serial.print(update_counter);
    Serial.print(" | Free Heap: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");
    
    lv_refr_now(eink_get_display());
    
    lastUpdate = currentMillis;
  }
  
  // LVGL task handler is NOT called here - we use manual refresh only
  // This is key for e-ink battery efficiency
  
  delay(100); // Small delay to prevent watchdog issues
}