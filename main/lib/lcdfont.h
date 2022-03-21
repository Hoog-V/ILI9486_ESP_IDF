// Font structures for newer Adafruit_GFX (1.1 and later).
// Example fonts are included in 'Fonts' directory.
// To use a font in your Arduino sketch, #include the corresponding .h
// file and pass address of GFXfont struct to setFont().  Pass NULL to
// revert to 'classic' fixed-space bitmap font.

#pragma once


struct lcdfont
{
  const char** char_addr;
  const char *width;
  uint8_t font_size;
  uint8_t line_size;
};
