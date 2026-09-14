SET(SOURCES ui_Screen1.c
    ui.c
    ui_comp_hook.c
    ui_helpers.c
    ui_img_background_main_png.c
    ui_img_arrow_1_png.c
    ui_img_background_1_png.c
    fonts/ui_font_Antonio10.c
    fonts/ui_font_Antonio30.c
    fonts/ui_font_Antonio60.c
    fonts/ui_font_NexaB10.c
    fonts/ui_font_NexaB15.c
    fonts/ui_font_NexaB20.c)

add_library(ui ${SOURCES})
