// Setup_Oko_S3_TFT1.h – ESP32-S3 Dev Module + GC9A01 (TFT1)
// Sdílené piny pro SPI, samostatné CS/DC pro první displej

#define USER_SETUP_ID 901

// Driver
#define GC9A01_DRIVER

// Rozlišení
#define TFT_WIDTH  240
#define TFT_HEIGHT 240

// SPI piny ESP32-S3 -> TFT1
#define TFT_MOSI 11    // DIN
#define TFT_SCLK 10    // CLK
#define TFT_CS    9    // CS (TFT1)
#define TFT_DC    8    // D/C (TFT1)
#define TFT_RST  14    // RST společný
#define TFT_MISO -1    // nečteme z displeje

// Barvy a ofset pro GC9A01
#define TFT_RGB_ORDER TFT_BGR
#define CGRAM_OFFSET

// Fonty
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_GFXFF
#define SMOOTH_FONT

// SPI rychlosti
#define SPI_FREQUENCY       40000000   // 40 MHz – ověřeno, funguje
#define SPI_READ_FREQUENCY  20000000

// Transakce – hodí se kvůli SD kartě
#define SUPPORT_TRANSACTIONS
