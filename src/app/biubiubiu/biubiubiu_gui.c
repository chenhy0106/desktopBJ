#include "biubiubiu_gui.h"
#include "loveheart_image.h"

#include "driver/lv_port_indev.h"
#include "lvgl.h"
#include "stdio.h"

#define GIF_MAX 8

lv_obj_t *biubiubiu_screen = NULL;
lv_obj_t *LovingHeart_image = NULL;

static lv_style_t default_style;

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
        biubiubiu_screen = NULL;
    }

    if (NULL != LovingHeart_image) {
        lv_obj_clean(LovingHeart_image);
        LovingHeart_image = NULL;
    }
}