idf_component_register(
    SRCS "src/epd_gui.c"
    INCLUDE_DIRS "include"
    REQUIRES driver esp_log epd_driver epd_font
)
