idf_component_register(
    SRCS "src/epd_gui.c"
    INCLUDE_DIRS "include"
    REQUIRES driver epd_driver epd_font
)
