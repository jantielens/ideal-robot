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
 * - Multi-screen architecture with lifecycle management
 * - Splash (5s) -> Arc (20s) -> Bar (20s) -> Arc (20s) -> Bar (20s)...
 * - SquareLine Studio compatible screen pattern
 * - MQTT-ready architecture (lifecycle hooks for future)
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
#include "screen_base.h"

// ===== SCREEN ENUM =====
typedef enum {
    SCREEN_SPLASH,
    SCREEN_ARC,
    SCREEN_BAR,
    SCREEN_COUNT  // Total number of screens
} screen_id_t;

// ===== SCREEN FUNCTION DECLARATIONS =====
// Splash Screen
void ui_Screen_Splash_init(void);
void ui_Screen_Splash_destroy(void);
lv_obj_t* ui_Screen_Splash_get_obj(void);

// Arc Screen
void ui_Screen_Arc_init(void);
void ui_Screen_Arc_destroy(void);
void ui_Screen_Arc_on_update(void);
lv_obj_t* ui_Screen_Arc_get_obj(void);

// Bar Screen
void ui_Screen_Bar_init(void);
void ui_Screen_Bar_destroy(void);
void ui_Screen_Bar_on_update(void);
lv_obj_t* ui_Screen_Bar_get_obj(void);

// ===== SCREEN REGISTRY =====
// Array of all screens with their lifecycle functions
// This table drives the screen manager
screen_t screens[SCREEN_COUNT] = {
    // Splash Screen (5 seconds, no updates, no MQTT)
    {
        .init = ui_Screen_Splash_init,
        .destroy = ui_Screen_Splash_destroy,
        .on_activate = NULL,
        .on_deactivate = NULL,
        .on_mqtt_message = NULL,
        .on_update = NULL,
        .screen_obj = NULL
    },
    // Arc Screen (20 seconds, periodic updates for animation)
    {
        .init = ui_Screen_Arc_init,
        .destroy = ui_Screen_Arc_destroy,
        .on_activate = NULL,
        .on_deactivate = NULL,
        .on_mqtt_message = NULL,
        .on_update = ui_Screen_Arc_on_update,
        .screen_obj = NULL
    },
    // Bar Screen (20 seconds, periodic updates)
    {
        .init = ui_Screen_Bar_init,
        .destroy = ui_Screen_Bar_destroy,
        .on_activate = NULL,
        .on_deactivate = NULL,
        .on_mqtt_message = NULL,
        .on_update = ui_Screen_Bar_on_update,
        .screen_obj = NULL
    }
};

// ===== SCREEN MANAGER STATE =====
screen_id_t current_screen_id = SCREEN_SPLASH;
unsigned long screen_start_time = 0;

// ===== APP STATE =====
unsigned long lastUpdate = 0;
const unsigned long UPDATE_INTERVAL = 5000; // Update every 5 seconds
const unsigned long SPLASH_DURATION = 5000; // Splash screen for 5 seconds
const unsigned long ARC_BAR_DURATION = 20000; // Arc and Bar screens for 20 seconds each

// ===== SCREEN MANAGEMENT =====
// Load a screen by ID
// 1. Deactivate current screen (unsubscribe MQTT topics)
// 2. Destroy current screen (free LVGL objects)
// 3. Initialize new screen (create LVGL objects)
// 4. Activate new screen (subscribe MQTT topics)
// 5. Load screen into LVGL display
void load_screen(screen_id_t screen_id)
{
    Serial.print("\n[SCREEN MGR] Transitioning from ");
    Serial.print(current_screen_id);
    Serial.print(" to ");
    Serial.println(screen_id);
    
    screen_t *old_screen = &screens[current_screen_id];
    screen_t *new_screen = &screens[screen_id];
    
    // Don't destroy if loading the same screen for the first time
    bool is_different_screen = (current_screen_id != screen_id) || (old_screen->screen_obj != NULL);
    
    // Deactivate old screen (unsubscribe MQTT)
    if(is_different_screen && old_screen->on_deactivate) {
        Serial.println("[SCREEN MGR] Deactivating old screen...");
        old_screen->on_deactivate();
    }
    
    // Initialize new screen (create LVGL objects)
    if(new_screen->init) {
        new_screen->init();
    }
    
    // Update screen_obj pointer
    if(screen_id == SCREEN_SPLASH) {
        new_screen->screen_obj = ui_Screen_Splash_get_obj();
    } else if(screen_id == SCREEN_ARC) {
        new_screen->screen_obj = ui_Screen_Arc_get_obj();
    } else if(screen_id == SCREEN_BAR) {
        new_screen->screen_obj = ui_Screen_Bar_get_obj();
    }
    
    // Load screen into LVGL (make new screen active BEFORE destroying old)
    if(new_screen->screen_obj) {
        lv_screen_load(new_screen->screen_obj);
        Serial.println("[SCREEN MGR] Screen loaded");
    }
    
    // Destroy old screen AFTER new screen is active (avoids LVGL warning)
    // Only if transitioning to a different screen OR reloading an existing screen
    if(is_different_screen && old_screen->destroy) {
        old_screen->destroy();
    }
    
    // Activate new screen (subscribe MQTT)
    if(new_screen->on_activate) {
        Serial.println("[SCREEN MGR] Activating new screen...");
        new_screen->on_activate();
    }
    
    // Update state
    current_screen_id = screen_id;
    screen_start_time = millis();
    
    Serial.println("[SCREEN MGR] Transition complete\n");
}

void setup()
{
  // Initialize serial communication
  Serial.begin(115200);
  delay(1000);
  
  // Print startup message
  Serial.println("\n=== ESP32 E-INK + LVGL Multi-Screen Demo ===");
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
  Serial.println("================================================\n");
  
  // Initialize e-ink display and LVGL
  eink_init();
  
  // Load initial screen (Splash)
  load_screen(SCREEN_SPLASH);
  
  // Initial render
  Serial.println("[LVGL] Performing initial render...");
  lv_refr_now(eink_get_display());
  eink_full_refresh_now();
  
  Serial.println("\n[READY] System initialized");
  Serial.println("[INFO] Screen sequence: Splash (5s) -> Arc (20s) -> Bar (20s) -> repeat");
  Serial.print("[INFO] Updates every ");
  Serial.print(UPDATE_INTERVAL / 1000.0);
  Serial.println(" seconds with partial refresh");
  Serial.print("[INFO] Full refresh every ");
  Serial.print(MAX_PARTIAL_UPDATES);
  Serial.println(" e-ink flushes to clear ghosting");
  Serial.println("[INFO] (Multiple widgets = multiple flushes per cycle)\n");
  
  lastUpdate = millis();
  screen_start_time = millis();
}

void loop()
{
  unsigned long currentMillis = millis();
  
  // ===== SCREEN TRANSITION LOGIC =====
  // Check if we need to transition screens
  if (current_screen_id == SCREEN_SPLASH) {
    // Splash -> Arc after 5 seconds
    if (currentMillis - screen_start_time >= SPLASH_DURATION) {
      Serial.println("\n[APP] Splash timeout - transitioning to Arc screen");
      load_screen(SCREEN_ARC);
      
      // Force full screen refresh after transition
      lv_refr_now(eink_get_display());
      eink_full_refresh_now();
      
      lastUpdate = currentMillis;
    }
  }
  else if (current_screen_id == SCREEN_ARC) {
    // Arc -> Bar after 20 seconds
    if (currentMillis - screen_start_time >= ARC_BAR_DURATION) {
      Serial.println("\n[APP] Arc timeout - transitioning to Bar screen");
      load_screen(SCREEN_BAR);
      
      // Force full screen refresh after transition
      lv_refr_now(eink_get_display());
      eink_full_refresh_now();
      
      lastUpdate = currentMillis;
    }
  }
  else if (current_screen_id == SCREEN_BAR) {
    // Bar -> Arc after 20 seconds (rotate forever)
    if (currentMillis - screen_start_time >= ARC_BAR_DURATION) {
      Serial.println("\n[APP] Bar timeout - transitioning to Arc screen");
      load_screen(SCREEN_ARC);
      
      // Force full screen refresh after transition
      lv_refr_now(eink_get_display());
      eink_full_refresh_now();
      
      lastUpdate = currentMillis;
    }
  }
  
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
    // Call current screen's update function (if it has one)
    screen_t *current_screen = &screens[current_screen_id];
    if (current_screen->on_update) {
      current_screen->on_update();
      
      // Check if we need a full refresh to clear ghosting
      eink_force_full_refresh();
      
      // Trigger LVGL refresh
      lv_refr_now(eink_get_display());
      
      // Power off panel voltages to prevent fading
      eink_poweroff();
    }
    
    lastUpdate = currentMillis;
  }
  
  // LVGL task handler is NOT called here - we use manual refresh only
  // This is key for e-ink battery efficiency
  
  delay(100); // Small delay to prevent watchdog issues
}