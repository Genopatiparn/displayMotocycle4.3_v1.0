SET(SOURCES ui_comp_rpm.c
    ui_comp.c
    ui_Screen1.c
    ui.c
    ui_comp_hook.c
    ui_helpers.c
    fonts/ui_font_Antonio10.c
    fonts/ui_font_Antonio30.c
    fonts/ui_font_Antonio60.c
    fonts/ui_font_NexaB10.c
    fonts/ui_font_NexaB15.c
    fonts/ui_font_NexaB20.c)

add_library(ui ${SOURCES})
