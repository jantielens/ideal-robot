/*
 * Arc Screen (Loading Animation)
 * 
 * Demonstrates a waiting/loading animation suitable for e-ink displays.
 * Shows a large arc that fills in 5 chunks (0% -> 20% -> 40% -> 60% -> 80% -> 100%).
 * 
 * LIFECYCLE:
 * - Shown after splash screen, alternates with bar screen
 * - Updates every 2.5 seconds to advance animation
 * - Each update advances by 72 degrees (360°/5 chunks)
 * - Screen duration: 20 seconds
 * 
 * E-INK CONSIDERATIONS:
 * - Chunked animation (not smooth) to minimize updates
 * - Black arc on white background for maximum contrast
 * - Large arc (160px diameter) for visibility
 */

#include "screen_base.h"

// SquareLine Studio compatible globals
lv_obj_t *ui_Screen_Arc = NULL;
lv_obj_t *ui_Arc_Loading = NULL;
lv_obj_t *ui_Label_Arc_Title = NULL;

// Screen-local state
static uint8_t arc_step = 0;  // 0-5 (0=empty, 5=full)

// Styles (must be static to persist after init)
static lv_style_t style_arc_main;
static lv_style_t style_arc_indicator;

// Screen initialization (SquareLine Studio pattern)
void ui_Screen_Arc_init(void)
{
    Serial.println("[SCREEN] Creating Arc screen...");
    
    // Create screen
    ui_Screen_Arc = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Screen_Arc, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_Screen_Arc, lv_color_white(), 0);
    
    // Arc styles for e-ink (black on white)
    lv_style_init(&style_arc_main);
    lv_style_set_arc_color(&style_arc_main, lv_color_make(0xE0, 0xE0, 0xE0));  // Light gray background
    lv_style_set_arc_width(&style_arc_main, 20);
    lv_style_set_arc_rounded(&style_arc_main, false);
    
    lv_style_init(&style_arc_indicator);
    lv_style_set_arc_color(&style_arc_indicator, lv_color_black());  // Black indicator
    lv_style_set_arc_width(&style_arc_indicator, 20);
    lv_style_set_arc_rounded(&style_arc_indicator, false);
    
    // Title label (at top)
    ui_Label_Arc_Title = lv_label_create(ui_Screen_Arc);
    lv_label_set_text(ui_Label_Arc_Title, "Loading...");
    lv_obj_set_style_text_color(ui_Label_Arc_Title, lv_color_black(), 0);
    lv_obj_set_style_text_font(ui_Label_Arc_Title, &lv_font_montserrat_20, 0);
    lv_obj_align(ui_Label_Arc_Title, LV_ALIGN_TOP_MID, 0, 20);
    
    // Create arc widget (positioned below label)
    ui_Arc_Loading = lv_arc_create(ui_Screen_Arc);
    
    // Remove default knob (we just want an arc, not interactive)
    lv_obj_remove_style(ui_Arc_Loading, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(ui_Arc_Loading, LV_OBJ_FLAG_CLICKABLE);
    
    // Apply styles
    lv_obj_remove_style_all(ui_Arc_Loading);
    lv_obj_add_style(ui_Arc_Loading, &style_arc_main, LV_PART_MAIN);
    lv_obj_add_style(ui_Arc_Loading, &style_arc_indicator, LV_PART_INDICATOR);
    
    // Set size and position AFTER applying styles
    lv_obj_set_size(ui_Arc_Loading, 140, 140);
    lv_obj_align(ui_Arc_Loading, LV_ALIGN_CENTER, 0, 30);
    
    // Configure arc range (0-100%)
    lv_arc_set_range(ui_Arc_Loading, 0, 100);
    lv_arc_set_bg_angles(ui_Arc_Loading, 0, 360);  // Full circle background
    
    // Start at 0% (empty)
    arc_step = 0;
    lv_arc_set_value(ui_Arc_Loading, 0);
    
    Serial.println("[SCREEN] Arc screen created");
}

// Screen destruction (SquareLine Studio pattern)
void ui_Screen_Arc_destroy(void)
{
    Serial.println("[SCREEN] Destroying Arc screen");
    
    if(ui_Screen_Arc) {
        lv_obj_del(ui_Screen_Arc);
    }
    
    // NULL all screen variables
    ui_Screen_Arc = NULL;
    ui_Arc_Loading = NULL;
    ui_Label_Arc_Title = NULL;
}

// Periodic update (called from app.ino loop)
// Advances arc by 1 chunk each update (5 chunks total)
void ui_Screen_Arc_on_update(void)
{
    // Advance to next chunk
    arc_step++;
    if (arc_step > 5) {
        arc_step = 0;  // Reset to empty and restart animation
    }
    
    // Calculate percentage (0%, 20%, 40%, 60%, 80%, 100%)
    uint8_t percentage = arc_step * 20;
    
    // Update arc
    lv_arc_set_value(ui_Arc_Loading, percentage);
    
    Serial.print("[UPDATE] Arc screen step ");
    Serial.print(arc_step);
    Serial.print("/5 (");
    Serial.print(percentage);
    Serial.println("%)");
}

// Get screen object (for screen manager)
lv_obj_t* ui_Screen_Arc_get_obj(void)
{
    return ui_Screen_Arc;
}
