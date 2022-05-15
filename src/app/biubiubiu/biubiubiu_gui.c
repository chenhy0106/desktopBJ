#include "biubiubiu_gui.h"
#include "loveheart_image.h"

#include "driver/lv_port_indev.h"
#include "lvgl.h"
#include "stdio.h"

#define GIF_MAX 8

LV_FONT_DECLARE(lv_font_ibmplex_115);
LV_FONT_DECLARE(ch_font20);

static char * texts[] = {
    "biubiubiu...",
    "Starting..."
};

static lv_obj_t *state_screen = NULL;
static lv_obj_t *biubiubiu_screen = NULL;
static lv_obj_t *LovingHeart_image = NULL;
static lv_obj_t *time_screen = NULL;
static lv_obj_t *clockLabel = NULL;
static lv_obj_t *dateLabel = NULL;
static lv_obj_t *stateLabel = NULL;

static lv_style_t default_style;
static lv_style_t numberBig_style;
static lv_style_t chFont_style;

const void *loveheartImg_Map[] = {
    &IMG00001, 
    &IMG00002,
    &IMG00003,
    &IMG00004,
    &IMG00005,
    &IMG00006,
    &IMG00007,
    &IMG00008
};

// const void *loveheartImg_Map[] = {&IMG00001, &IMG00002};

void biubiubiu_gui_init()
{
    lv_style_init(&default_style);
    lv_style_set_bg_color(&default_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_bg_color(&default_style, LV_STATE_PRESSED, LV_COLOR_GRAY);
    lv_style_set_bg_color(&default_style, LV_STATE_FOCUSED, LV_COLOR_BLACK);
    lv_style_set_bg_color(&default_style, LV_STATE_FOCUSED | LV_STATE_PRESSED, lv_color_hex(0xf88));
    lv_style_init(&numberBig_style);
    lv_style_set_text_opa(&numberBig_style, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_text_color(&numberBig_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_text_font(&numberBig_style, LV_STATE_DEFAULT, &lv_font_ibmplex_115);
    lv_style_init(&chFont_style);
    lv_style_set_text_opa(&chFont_style, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_text_color(&chFont_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_text_font(&chFont_style, LV_STATE_DEFAULT, &ch_font20);

    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    lv_obj_clean(act_obj);
}

void display_biubiubiu_init()
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == biubiubiu_screen)
        return;
    lv_obj_clean(act_obj);
    biubiubiu_gui_del();
    
    biubiubiu_screen = lv_obj_create(NULL, NULL);
    lv_obj_add_style(biubiubiu_screen, LV_BTN_PART_MAIN, &default_style);
    LovingHeart_image = lv_img_create(biubiubiu_screen, NULL);
    lv_img_set_src(LovingHeart_image, loveheartImg_Map[0]);
    lv_obj_align(LovingHeart_image, NULL, LV_ALIGN_CENTER, 0, 0);
}

void display_biubiubiu()
{
    display_biubiubiu_init();
    static int cur_pos = 0;
    lv_img_set_src(LovingHeart_image, loveheartImg_Map[cur_pos]);
    cur_pos = (cur_pos + 1) % GIF_MAX;

    lv_scr_load(biubiubiu_screen);
}

void biubiubiu_gui_del(void)
{
    if (NULL != biubiubiu_screen)
    {
        lv_obj_clean(biubiubiu_screen); // 清空此前页面
        lv_obj_clean(LovingHeart_image);
        biubiubiu_screen = NULL;
        LovingHeart_image = NULL;
    }

    if (NULL != time_screen) {
        lv_obj_clean(time_screen);
        lv_obj_clean(clockLabel);
        lv_obj_clean(dateLabel);
        time_screen = NULL;
        clockLabel = NULL;
        dateLabel = NULL;
    }

    if (NULL != state_screen) {
        lv_obj_clean(stateLabel);
        lv_obj_clean(state_screen);
        stateLabel = NULL;
        state_screen = NULL;

    }
}

void display_time_init() {
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == time_screen)
        return;
    lv_obj_clean(act_obj);
    biubiubiu_gui_del();

    time_screen = lv_obj_create(NULL, NULL);
    lv_obj_add_style(time_screen, LV_BTN_PART_MAIN, &default_style);
    clockLabel = lv_label_create(time_screen, NULL);
    lv_obj_add_style(clockLabel, LV_LABEL_PART_MAIN, &numberBig_style);
    lv_label_set_recolor(clockLabel, true);
    lv_obj_align(clockLabel, time_screen, LV_ALIGN_CENTER, -95, 5);
    dateLabel = lv_label_create(time_screen, NULL);
    lv_obj_add_style(dateLabel, LV_LABEL_PART_MAIN, &chFont_style);
    lv_label_set_recolor(dateLabel, true);
    lv_obj_align(dateLabel, time_screen, LV_ALIGN_CENTER, -40, 50);
}

void display_biubiubiu_time(struct TimeStr timeInfo) {
    display_time_init();

    lv_label_set_text_fmt(clockLabel, "%02d %02d", timeInfo.hour, timeInfo.minute);
    lv_label_set_text_fmt(dateLabel, "%d年%d月%d日", timeInfo.year, timeInfo.month, timeInfo.day);
    lv_scr_load(time_screen);
}

void display_state_init() {
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == state_screen)
        return;
    lv_obj_clean(act_obj);
    biubiubiu_gui_del();

    state_screen = lv_obj_create(NULL, NULL);
    lv_obj_add_style(state_screen, LV_BTN_PART_MAIN, &default_style);
    stateLabel = lv_label_create(state_screen, NULL);
    lv_obj_add_style(stateLabel, LV_LABEL_PART_MAIN, &chFont_style);
    lv_label_set_recolor(stateLabel, true);
    lv_obj_align(stateLabel, state_screen, LV_ALIGN_CENTER, -40, 50);
}

void display_state(int text_id) {
    display_state_init();

    lv_label_set_text_fmt(stateLabel, "%s", texts[text_id]);
    lv_scr_load(state_screen);
}