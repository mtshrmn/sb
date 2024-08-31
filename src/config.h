// set theme
#define THEME_DARK 0
#define THEME_LIGHT 1
#define THEME THEME_DARK

// color format is - 0xAARRGGBB
#define _COLOR_PRIMARY_DARK 0xFFFFFFFF
#define _COLOR_SECNDARY_DARK 0xFFFF0000
#define _BOARD_BG_DARK 0xFF000000

// light theme colors definitions
#define _COLOR_PRIMARY_LIGHT 0xFF000000
#define _COLOR_SECNDARY_LIGHT 0xFFFF0000
#define _BOARD_BG_LIGHT 0xFFFFFFFF

// stroke widths
#define STROKE_WIDTH_THIN 1.5
#define STROKE_WIDTH_MEDIUM 3.0
#define STROKE_WIDTH_THICK 6.0
#define STROKE_WIDTH_THICKER 12.0
#define STROKE_WIDTH_THICKEST 24.0
#define STROKES_AMOUNT 5
#define COLORS_AMOUNT 2

#ifdef USER
#define SCREENSHOTS_PATH "/home/" USER "/pictures/sb/"
#else
#define SCREENSHOTS_PATH "./"
#endif

// --------------------------------------------
#if THEME
#define BOARD_BG _BOARD_BG_LIGHT
#define BOARD_BG_INVERTED _BOARD_BG_DARK
#define COLOR_PRIMARY _COLOR_PRIMARY_LIGHT
#define COLOR_SECONDARY _COLOR_SECNDARY_LIGHT

#else
#define BOARD_BG _BOARD_BG_DARK
#define BOARD_BG_INVERTED _BOARD_BG_LIGHT
#define COLOR_PRIMARY _COLOR_PRIMARY_DARK
#define COLOR_SECONDARY _COLOR_SECNDARY_DARK

#endif

#define A_MASK 0xFF000000
#define R_MASK 0x00FF0000
#define G_MASK 0x0000FF00
#define B_MASK 0x000000FF

#define BOARD_BG_CAIRO                                                                                                 \
  ((BOARD_BG & R_MASK) >> 16), ((BOARD_BG & G_MASK) >> 8), (BOARD_BG & B_MASK), ((BOARD_BG & A_MASK) >> 24)
#define BOARD_BG_INVERTED_CAIRO                                                                                        \
  ((BOARD_BG_INVERTED & R_MASK) >> 16), ((BOARD_BG_INVERTED & G_MASK) >> 8), (BOARD_BG_INVERTED & B_MASK),             \
      ((BOARD_BG_INVERTED & A_MASK) >> 24)
