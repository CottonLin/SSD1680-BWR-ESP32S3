idf_component_register(SRCS "main.c" "ssd1680.c" "epd_gui.c" "epd_font.c"
                    INCLUDE_DIRS "."
                    REQUIRES epd_driver epd_gui epd_font epd_tests unity)
