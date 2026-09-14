/*****************************************************************************
  | File        :   LVGL_Driver.cpp
******************************************************************************/
#include "LVGL_Driver.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <string.h>

static lv_display_t *s_disp       = NULL;
static TaskHandle_t  s_lvgl_task  = NULL;
static QueueHandle_t s_flush_queue = NULL;

void* buf1 = NULL;
void* buf2 = NULL;

void Lvgl_Set_Task_Handle(TaskHandle_t h) { s_lvgl_task = h; }

typedef struct {
    int           x1, y1, x2, y2;
    uint8_t      *px_map;
    lv_display_t *disp;
} flush_job_t;

/* Runs on core 0 — calls draw_bitmap while LVGL renders on core 1 */
static void flush_task(void *arg)
{
    flush_job_t job;
    while (1) {
        if (xQueueReceive(s_flush_queue, &job, portMAX_DELAY) == pdTRUE) {
            esp_lcd_panel_draw_bitmap(rgb_panel_handle,
                                      job.x1, job.y1, job.x2 + 1, job.y2 + 1,
                                      job.px_map);
            lv_display_flush_ready(job.disp);
            if (s_lvgl_task) {
                xTaskNotifyGive(s_lvgl_task);
            }
        }
    }
}

/* Called by LVGL when a strip is ready — sends to flush_task and returns */
void Lvgl_Display_LCD(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    flush_job_t job = {
        .x1     = area->x1,
        .y1     = area->y1,
        .x2     = area->x2,
        .y2     = area->y2,
        .px_map = px_map,
        .disp   = disp,
    };
    xQueueSend(s_flush_queue, &job, portMAX_DELAY);
    // lv_display_flush_ready called from flush_task after draw_bitmap completes
}

/* Called by LVGL while it waits for the previous flush to finish */
static void lvgl_wait_cb(lv_display_t *disp)
{
    ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1));
}

void Lvgl_Touchpad_Read(lv_indev_t *indev, lv_indev_data_t *data)
{
    data->state = LV_INDEV_STATE_RELEASED;
}

void Lvgl_print(const char *buf) {}

void example_increase_lvgl_tick(void *arg)
{
    lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

void Lvgl_Init(void)
{
    lv_init();

    s_flush_queue = xQueueCreate(4, sizeof(flush_job_t));
    xTaskCreatePinnedToCore(flush_task, "lvgl_flush", 4096, NULL, configMAX_PRIORITIES - 1, NULL, 0);

    buf1 = heap_caps_malloc(LVGL_BUF_LEN, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    buf2 = heap_caps_malloc(LVGL_BUF_LEN, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    printf("[LVGL] buf1=%p buf2=%p\n", buf1, buf2);

    s_disp = lv_display_create(LVGL_WIDTH, LVGL_HEIGHT);
    lv_display_set_color_format(s_disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(s_disp, Lvgl_Display_LCD);
    lv_display_set_flush_wait_cb(s_disp, lvgl_wait_cb);
    lv_display_set_buffers(s_disp, buf1, buf2, LVGL_BUF_LEN,
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_user_data(s_disp, rgb_panel_handle);

    // Clear the PSRAM framebuffer to black — without this, DMA streams PSRAM garbage
    // (leftover from previous boots) until LVGL completes its first full render pass.
    static uint16_t blank_line[LVGL_WIDTH];
    memset(blank_line, 0x00, sizeof(blank_line));
    for (int y = 0; y < LVGL_HEIGHT; y++) {
        esp_lcd_panel_draw_bitmap(rgb_panel_handle, 0, y, LVGL_WIDTH, y + 1, blank_line);
    }

    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, Lvgl_Touchpad_Read);

    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Hello Arduino and LVGL!");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &example_increase_lvgl_tick,
        .name     = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer);
    esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000);
}

void Lvgl_Loop(void)
{
    lv_timer_handler();
}
