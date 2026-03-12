// set theme
#define THEME_DARK 0
#define THEME_LIGHT 1
#define THEME THEME_DARK

// color format is - 0xAARRGGBB
#define COLOR_PRIMARY_DARK 0xFFFFFFFF
#define COLOR_SECNDARY_DARK 0xFFFF0000
#define BOARD_BG_DARK 0xFF000000

// light theme colors definitions
#define COLOR_PRIMARY_LIGHT 0xFF000000
#define COLOR_SECNDARY_LIGHT 0xFFFF0000
#define BOARD_BG_LIGHT 0xFFFFFDD0

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
#define BOARD_BG BOARD_BG_LIGHT
#define BOARD_BG_INVERTED BOARD_BG_DARK
#define COLOR_PRIMARY COLOR_PRIMARY_LIGHT
#define COLOR_SECONDARY COLOR_SECNDARY_LIGHT

#else
#define BOARD_BG BOARD_BG_DARK
#define BOARD_BG_INVERTED BOARD_BG_LIGHT
#define COLOR_PRIMARY COLOR_PRIMARY_DARK
#define COLOR_SECONDARY COLOR_SECNDARY_DARK

#endif

#define A_MASK 0xFF000000
#define R_MASK 0x00FF0000
#define G_MASK 0x0000FF00
#define B_MASK 0x000000FF

#define CAIRO_R(color) ((double)(((color) & R_MASK) >> 16) / (double)0xFF)
#define CAIRO_G(color) ((double)(((color) & G_MASK) >> 8) / (double)0xFF)
#define CAIRO_B(color) ((double)((color) & B_MASK) / (double)0xFF)
#define CAIRO_A(color) ((double)(((color) & A_MASK) >> 24) / (double)0xFF)

#define BOARD_BG_CAIRO CAIRO_R(BOARD_BG), CAIRO_G(BOARD_BG), CAIRO_B(BOARD_BG), CAIRO_A(BOARD_BG)
#define BOARD_BG_INVERTED_CAIRO                                                                                        \
  CAIRO_R(BOARD_BG_INVERTED), CAIRO_G(BOARD_BG_INVERTED), CAIRO_B(BOARD_BG_INVERTED), CAIRO_A(BOARD_BG_INVERTED)
