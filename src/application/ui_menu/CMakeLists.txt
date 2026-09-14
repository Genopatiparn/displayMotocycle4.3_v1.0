SET(SOURCES ui_Screen1.c
    ui.c
    ui_comp_hook.c
    ui_helpers.c
    ui_img_background_menu_png.c
    ui_img_display_png.c
    ui_img_drag_png.c
    ui_img_gps_png.c
    ui_img_setting_png.c)

add_library(ui ${SOURCES})
