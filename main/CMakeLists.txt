set(srcs "main.c" )

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "."
                    INCLUDE_DIRS ../include/ILI9486)
