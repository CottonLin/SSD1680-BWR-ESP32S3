idf_component_register(
    SRCS "src/epd_tests.c"
    INCLUDE_DIRS "."
    REQUIRES epd_driver epd_gui epd_font unity
    PRIVACY_REQUIRES esp_timer
)
