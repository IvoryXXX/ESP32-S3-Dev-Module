#pragma once

#define USER_SETUP_ID 901

// Driver
#define GC9A01_DRIVER

// Rozlišení
#define TFT_WIDTH  240
#define TFT_HEIGHT 240

// SPI piny ESP32-S3 -> TFT1
#define TFT_MOSI 11
#define TFT_SCLK 10
#define TFT_CS    9
#define TFT_DC    8
#define TFT_RST  14
#define TFT_MISO -1

// Barvy a offset
#define TFT_RGB_ORDER TFT_BGR
#define CGRAM_OFFSET

// Fonty
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_GFXFF
#define SMOOTH_FONT

// SPI rychlosti
#define SPI_FREQUENCY       40000000
#define SPI_READ_FREQUENCY  20000000

#define SUPPORT_TRANSACTIONS
