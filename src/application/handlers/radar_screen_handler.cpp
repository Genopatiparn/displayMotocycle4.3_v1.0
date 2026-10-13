#include "radar_screen_handler.h"
#include "ui_radarmode/ui_Screen1.h"
#include "gps_sr1723.h"
#include <Arduino.h>
#include <algorithm>
#include <vector>

extern "C" {
extern lv_obj_t *radar_ui_Screen1;
extern void radar_ui_Screen1_screen_init(void);
extern lv_obj_t *radar_ui_Bar1;
extern lv_obj_t *radar_ui_Bar2;
extern lv_obj_t *radar_ui_Bar3;
extern lv_obj_t *radar_ui_Bar4;
extern lv_obj_t *radar_ui_Bar5;
extern lv_obj_t *radar_ui_Label4;  
extern lv_obj_t *radar_ui_Label5;
extern lv_obj_t *radar_ui_Label6;

// Create our own GPS icon on radar screen (don't use ui_Image3 - it's on wrong screen!)
static lv_obj_t *radar_gps_icon = NULL;

// GPS status icon images
extern const lv_image_dsc_t ui_img_gps_status_png;    // No signal (Bad)
extern const lv_image_dsc_t ui_img_gps_status_1_png;  // Moderate/Fair
extern const lv_image_dsc_t ui_img_gps_status_2_png;  // Good
extern const lv_image_dsc_t ui_img_gps_status_3_png;  // Excellent
}

static std::vector<lv_obj_t *> satellite_bars;
static std::vector<lv_obj_t *> prn_labels;
static std::vector<lv_obj_t *> snr_labels;
static int displayed_satellite_count = -1;
static bool original_bars_active = false;
static constexpr int ORIGINAL_BAR_COUNT = 5;

static uint32_t satellite_color(uint8_t constellation)
{
    switch (constellation) {
        case 1: return 0xFFFFFF; // ขาว      GPS
        case 2: return 0xFF8C00; // ส้ม      GLONASS
        case 3: return 0x0080FF; // น้ำเงิน  Galileo
        case 4: return 0x00FF00; // เขียว    BeiDou
        case 5: return 0xFFFF00; // เหลือง   QZSS
        case 6: return 0x00FFFF; // ฟ้า      SBAS
        default: return 0x660099; // ม่วง    Unknown
    }
}
static void hide_generated_bars(void)
{
    lv_obj_t *bars[] = {radar_ui_Bar1, radar_ui_Bar2, radar_ui_Bar3,
                        radar_ui_Bar4, radar_ui_Bar5};
    for (lv_obj_t *bar : bars) {
        if (bar) lv_obj_add_flag(bar, LV_OBJ_FLAG_HIDDEN);
    }
}

static void clear_dynamic_bars(void)
{
    if (!original_bars_active) {
        int count = 0;
        for (lv_obj_t *obj : satellite_bars) {
            if (obj) lv_obj_del(obj);
            if (++count % 5 == 0) yield(); 
        }
    }
    
    int count = 0;
    for (lv_obj_t *obj : prn_labels) {
        if (obj) lv_obj_del(obj);
        if (++count % 5 == 0) yield();
    }
    
    count = 0;
    for (lv_obj_t *obj : snr_labels) {
        if (obj) lv_obj_del(obj);
        if (++count % 5 == 0) yield();
    }
    
    satellite_bars.clear();
    prn_labels.clear();
    snr_labels.clear();
}

static void create_dynamic_bars(int count)
{
    clear_dynamic_bars();
    if (count <= 0) {
        if (original_bars_active) hide_generated_bars();
        return;
    }

    if (count <= ORIGINAL_BAR_COUNT) {
        original_bars_active = true;
        lv_obj_t *original_bars[ORIGINAL_BAR_COUNT] = {
            radar_ui_Bar1, radar_ui_Bar2, radar_ui_Bar3, radar_ui_Bar4, radar_ui_Bar5};
        hide_generated_bars();
        for (int i = 0; i < count; i++) {
            lv_obj_t *bar = original_bars[i];
            if (!bar) continue;
            lv_obj_remove_flag(bar, LV_OBJ_FLAG_HIDDEN);
            satellite_bars.push_back(bar);

            lv_obj_t *prn = lv_label_create(radar_ui_Screen1);
            lv_obj_t *snr = lv_label_create(radar_ui_Screen1);
            lv_obj_set_style_text_font(prn, &lv_font_montserrat_10, LV_PART_MAIN);
            lv_obj_set_style_text_font(snr, &lv_font_montserrat_10, LV_PART_MAIN);
            lv_obj_set_style_text_align(prn, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
            lv_obj_set_style_text_align(snr, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
            prn_labels.push_back(prn);
            snr_labels.push_back(snr); 
        }
        return;
    }

    original_bars_active = false;
    hide_generated_bars();

    constexpr lv_coord_t screen_width = 480;
    constexpr lv_coord_t screen_center_x = 240;
    constexpr lv_coord_t area_y = 113;  
    constexpr lv_coord_t area_height = 148;
    constexpr lv_coord_t margin = 19;  
    constexpr lv_coord_t usable_width = screen_width - (margin * 2);  

    lv_coord_t bar_width;
    lv_coord_t gap;
    
    if (count <= 20) {
        bar_width = 18;
        gap = 6;
    } else if (count <= 27) {
        bar_width = 14;
        gap = 4;
    } else {
        bar_width = 12;
        gap = 2;
    }
    
    const lv_coord_t total_width = count * bar_width + (count - 1) * gap;

    if (total_width > usable_width) {
        const lv_coord_t spacing = usable_width / count; 
        bar_width = spacing * 3 / 4;
        gap = spacing - bar_width;  
        if (bar_width < 8) bar_width = 8;  
        if (gap < 1) gap = 1;        
    }
    
    // Always start at Bar1 position (x=19)
    const lv_coord_t bar1_x = margin;  // Start at left margin (x=19)
    
    // Choose font size based on bar width
    const lv_font_t* label_font;
    if (bar_width >= 16) {
        label_font = &lv_font_montserrat_10;  // Normal font for wide bars
    } else if (bar_width >= 12) {
        label_font = &lv_font_montserrat_8;   // Smaller font for medium bars
    } else {
        label_font = &lv_font_montserrat_8;   // Tiny font for narrow bars
    }
    
    const bool show_labels = true;

    for (int i = 0; i < count; i++) {
        const lv_coord_t x = bar1_x + i * (bar_width + gap);
        lv_obj_t *bar = lv_bar_create(radar_ui_Screen1);
        lv_obj_set_pos(bar, x, area_y);
        lv_obj_set_size(bar, bar_width > 0 ? bar_width : 1, area_height);
        lv_bar_set_range(bar, 0, 100);
        lv_bar_set_value(bar, 0, LV_ANIM_OFF);
        lv_obj_set_style_radius(bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(bar, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_INDICATOR | LV_STATE_DEFAULT);
        satellite_bars.push_back(bar);

        lv_obj_t *prn = nullptr;
        lv_obj_t *snr = nullptr;
        if (show_labels) {
            prn = lv_label_create(radar_ui_Screen1);
            snr = lv_label_create(radar_ui_Screen1);
            lv_obj_set_style_text_font(prn, label_font, LV_PART_MAIN);
            lv_obj_set_style_text_font(snr, label_font, LV_PART_MAIN);
            lv_obj_set_style_text_align(prn, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
            lv_obj_set_style_text_align(snr, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        }
        prn_labels.push_back(prn);
        snr_labels.push_back(snr);
        
        // Yield every 5 satellites to prevent watchdog timeout during creation
        if ((i + 1) % 5 == 0) {
            yield();
        }
    }
}

void RadarScreenHandler_Init(void)
{
    if (!radar_ui_Screen1) radar_ui_Screen1_screen_init();
    hide_generated_bars();
    
    // Create GPS status icon on radar screen
    if (!radar_gps_icon && radar_ui_Screen1) {
        radar_gps_icon = lv_image_create(radar_ui_Screen1);
        lv_image_set_src(radar_gps_icon, &ui_img_gps_status_png);  // Start with "Bad" icon
        lv_obj_set_width(radar_gps_icon, LV_SIZE_CONTENT);
        lv_obj_set_height(radar_gps_icon, LV_SIZE_CONTENT);
        lv_obj_set_x(radar_gps_icon, -224);   // Left side
        lv_obj_set_y(radar_gps_icon, -118);   // Top side
        lv_obj_set_align(radar_gps_icon, LV_ALIGN_CENTER);
        lv_obj_add_flag(radar_gps_icon, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_remove_flag(radar_gps_icon, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_move_foreground(radar_gps_icon);  // Bring to front
    }
}

lv_obj_t *RadarScreenHandler_GetScreen(void)
{
    return radar_ui_Screen1;
}

void RadarScreenHandler_UpdateGPS(void)
{
    if (!radar_ui_Screen1 || !radar_ui_Label5 || !radar_ui_Label6) return;

    // Get real GPS data
    const char *quality = GPS_GetQualityStatus();
    bool has_fix = GPS_HasFix();

    // Update GPS icon on radar screen
    lv_obj_t *gps_icon = radar_gps_icon;
    
    if (gps_icon) {
        static const char *last_quality = nullptr;
        
        // Only update if quality changed
        if (last_quality != quality) {
            if (!has_fix || strcmp(quality, "Bad") == 0) {
                lv_image_set_src(gps_icon, &ui_img_gps_status_png);  // No bars
            } else if (strcmp(quality, "Fair") == 0 || strcmp(quality, "Moderate") == 0) {
                lv_image_set_src(gps_icon, &ui_img_gps_status_1_png);  // 1 bar
            } else if (strcmp(quality, "Good") == 0) {
                lv_image_set_src(gps_icon, &ui_img_gps_status_2_png);  // 2 bars
            } else if (strcmp(quality, "Excellent") == 0) {
                lv_image_set_src(gps_icon, &ui_img_gps_status_3_png);  // 3 bars
            }
            
            // Force refresh
            lv_obj_invalidate(gps_icon);
            
            last_quality = quality;
        }
    }

    gps_satellite_t satellites[GPS_MAX_SATELLITES];
    const uint8_t satellite_count = GPS_GetSatelliteInfo(satellites);
    gps_satellite_t visible[GPS_MAX_SATELLITES];
    int visible_count = 0;

    for (int i = 0; i < satellite_count && visible_count < GPS_MAX_SATELLITES; i++) {
        if (satellites[i].snr > 0) visible[visible_count++] = satellites[i];
    }

    std::sort(visible, visible + visible_count,
              [](const gps_satellite_t &left, const gps_satellite_t &right) {
                  if (left.constellation != right.constellation) {
                      return left.constellation < right.constellation;
                  }
                  if (left.snr != right.snr) return left.snr > right.snr;
                  return left.prn < right.prn;
              });
    
    // Find max SNR for auto-scaling (normalize strongest signal to 60% bar height)
    uint8_t max_snr = 0;
    for (int i = 0; i < visible_count; i++) {
        if (visible[i].snr > max_snr) max_snr = visible[i].snr;
    }

    lv_label_set_text_fmt(radar_ui_Label5, "%d", visible_count);
    lv_label_set_text(radar_ui_Label6, quality);
    lv_obj_set_style_text_color(radar_ui_Label6,
                                lv_color_hex(strcmp(quality, "Bad") == 0 ? 0xFF0000 : 0x00FF00),
                                LV_PART_MAIN);

    
    if (radar_ui_Label4) {
        gps_debug_info_t gps_info;
        GPS_GetDebugInfo(&gps_info);
        
        if (gps_info.utc_time[0] != '\0') {

            int hour = 0, minute = 0, second = 0;
            if (sscanf(gps_info.utc_time, "%d:%d:%d", &hour, &minute, &second) == 3) {

                char local_time[8];
                snprintf(local_time, sizeof(local_time), "%02d:%02d", hour, minute);
                lv_label_set_text(radar_ui_Label4, local_time);
            } else {

                lv_label_set_text(radar_ui_Label4, "--:--");
            }
        } else {
            lv_label_set_text(radar_ui_Label4, "--:--");
        }
    }

    if (visible_count != displayed_satellite_count) {
        create_dynamic_bars(visible_count);
        displayed_satellite_count = visible_count;

        yield();
    }

    for (int i = 0; i < visible_count; i++) {
        const gps_satellite_t &satellite = visible[i];

        // Auto-scale: strongest signal (max_snr) = 60% bar height
        // Other signals scale proportionally
        int bar_value;
        if (max_snr > 0) {
            bar_value = (satellite.snr * 60) / max_snr;  // Scale to 60% max
            if (bar_value > 60) bar_value = 60;  // Cap at 60%
        } else {
            bar_value = 0;
        }
        
        const uint32_t color = satellite_color(satellite.constellation);
        lv_obj_t *bar = satellite_bars[i];
        lv_bar_set_value(bar, bar_value, LV_ANIM_OFF);
        lv_obj_set_style_bg_color(bar, lv_color_hex(color), LV_PART_INDICATOR);

        if (prn_labels[i] && snr_labels[i]) {
            const lv_coord_t x = lv_obj_get_x(bar);
            const lv_coord_t width = lv_obj_get_width(bar);
            lv_label_set_text_fmt(prn_labels[i], "%02u", satellite.prn);
            lv_label_set_text_fmt(snr_labels[i], "%u", satellite.snr);
            lv_obj_set_style_text_color(prn_labels[i], lv_color_hex(color), LV_PART_MAIN);
            lv_obj_set_width(prn_labels[i], width);
            lv_obj_set_width(snr_labels[i], width);
            lv_obj_update_layout(prn_labels[i]);
            lv_obj_update_layout(snr_labels[i]);
            const lv_coord_t bar_y = lv_obj_get_y(bar);
            const lv_coord_t bar_height = lv_obj_get_height(bar);
            lv_obj_set_pos(prn_labels[i], x, bar_y + bar_height + 1);
            lv_obj_set_pos(snr_labels[i], x,
                           bar_y + bar_height - (bar_height * bar_value / 100) - 13);
        }
        if (i % 10 == 0) yield();
    }
}
