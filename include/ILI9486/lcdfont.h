#pragma once

#include <stdbool.h>

#define VARIABLE_WIDTH true
#define CONSTANT_WIDTH false
#define FLIPPED true
#define NORMAL false

struct lcdfont
{
  const char** addr;
  const char *width;
  uint8_t font_size;
  uint8_t line_size;
  uint8_t letter_spacing;
  bool variable_width;
  bool font_flipped;
};
