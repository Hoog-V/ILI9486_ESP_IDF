set(srcs "main.c" 
          "lib/lcd_ili9486_base.c"
          "lib/lcd_ili9486_primitives.c"
          "lib/lcd_ili9486_fontparser.c"
          )

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "."
                    INCLUDE_DIRS "lib")
