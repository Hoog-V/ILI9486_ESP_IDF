set(srcs "main.c" 
          "lcd_ili9486_lib.c"
          )

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ".")
