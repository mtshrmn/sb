// set theme
#define THEME_DARK 0
#define THEME_LIGHT 1
#define THEME THEME_DARK

// color format is - 0xAARRGGBB
// board bg is in cario format
// dark theme colors definitions
#define _COLOR_PRIMARY_DARK 0xFFFFFFFF
#define _COLOR_SECNDARY_DARK 0xFFFF0000
#define _BOARD_BG_DARK 0, 0, 0, 1

// light theme colors definitions
#define _COLOR_PRIMARY_LIGHT 0xFF000000
#define _COLOR_SECNDARY_LIGHT 0xFFFF0000
#define _BOARD_BG_LIGHT 1, 1, 1, 1

// stroke widths
#define _STROKE_WIDTH_THIN 1.5
#define _STROKE_WIDTH_MEDIUM 3.0
#define _STROKE_WIDTH_THICK 6.0

// display toolbar
// #define USE_TOOLBAR

// --------------------------------------------
#if THEME
#define BOARD_BG _BOARD_BG_LIGHT
#define BOARD_BG_INVERTED _BOARD_BG_DARK
#define _COLOR_PRIMARY _COLOR_PRIMARY_LIGHT
#define _COLOR_SECONDARY _COLOR_SECNDARY_LIGHT

#else
#define BOARD_BG _BOARD_BG_DARK
#define BOARD_BG_INVERTED _BOARD_BG_LIGHT
#define _COLOR_PRIMARY _COLOR_PRIMARY_DARK
#define _COLOR_SECONDARY _COLOR_SECNDARY_DARK

#endif
