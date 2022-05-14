set(SOURCES lcd_ili9486_base.c lcd_ili9486_fontparser.c lcd_ili9486_primitives.c)
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS . ../../include)