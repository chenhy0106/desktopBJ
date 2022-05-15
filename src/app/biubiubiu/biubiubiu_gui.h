#ifndef APP_BIUBIUBIU_GUI_H
#define APP_BIUBIUBIU_GUI_H

struct TimeStr
{
    int year;
    int month;
    int day;
    int hour;
    int minute;
};

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
#define ANIEND                      \
    while (lv_anim_count_running()) \
        lv_task_handler(); //等待动画完成

    void biubiubiu_gui_init(void);
    void display_biubiubiu_init(void);
    void display_biubiubiu();
    void display_state(int text_id);
    void display_biubiubiu_time(struct TimeStr timeInfo);
    void biubiubiu_gui_del(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
    extern const lv_img_dsc_t app_biubiubiu;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif