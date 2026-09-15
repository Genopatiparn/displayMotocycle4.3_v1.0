SET(SOURCES ui_dragScreen.c
    ui.c
    ui_comp_hook.c
    ui_helpers.c
    fonts/ui_font_Antonio10.c
    fonts/ui_font_Antonio16.c
    fonts/ui_font_Antonio30.c
    fonts/ui_font_Antonio40.c
    fonts/ui_font_Antonio60.c
    fonts/ui_font_NexaB10.c
    fonts/ui_font_NexaB15.c
    fonts/ui_font_NexaB20.c
    fonts/ui_font_NexaB30.c
    fonts/ui_font_NexaB50.c
    fonts/ui_font_NexaB70.c)

add_library(ui ${SOURCES})
