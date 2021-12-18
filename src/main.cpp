#define MJPEG_BUFFER_SIZE (320 * 240 * 2 / 4)

#include <WiFi.h>
#include <SD.h>
#include <SD_MMC.h>

#include <Arduino_GFX_Library.h>
#define TFT_BRIGHTNESS 128
#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_FIRE)
#define TFT_BL 32
#define SS 4
Arduino_HWSPI *bus = new Arduino_HWSPI(27 /* DC */, 14 /* CS */, SCK, MOSI, MISO);
Arduino_GFX *gfx = new Arduino_ILI9341_M5STACK(bus, 33 /* RST */, 1 /* rotation */);
#elif defined(ARDUINO_ODROID_ESP32)
#define TFT_BL 14
Arduino_HWSPI *bus = new Arduino_HWSPI(21 /* DC */, 5 /* CS */, SCK, MOSI, MISO);
// Arduino_GFX *gfx = new Arduino_ILI9341(bus, -1 /* RST */, 3 /* rotation */);
Arduino_GFX *gfx = new Arduino_ST7789(bus, -1 /* RST */, 1 /* rotation */, true /* IPS */);
#elif defined(ARDUINO_T) // TTGO T-Watch
#define TFT_BL 12
Arduino_HWSPI *bus = new Arduino_HWSPI(27 /* DC */, 5 /* CS */, SCK, MOSI, MISO);
Arduino_GFX *gfx = new Arduino_ST7789(bus, -1 /* RST */, 2 /* rotation */, true /* IPS */, 240, 240, 0, 80);
#else /* not a specific hardware */

// ST7789 Display
// Arduino_HWSPI *bus = new Arduino_HWSPI(15 /* DC */, 12 /* CS */, SCK, MOSI, MISO);
// Arduino_GFX *gfx = new Arduino_ST7789(bus, -1 /* RST */, 2 /* rotation */, true /* IPS */, 240 /* width */, 240 /* height */, 0 /* col offset 1 */, 80 /* row offset 1 */);
// ILI9225 Display
//Arduino_HWSPI *bus = new Arduino_HWSPI(27 /* DC */, 5 /* CS */, SCK, MOSI, MISO);
//Arduino_GFX *gfx = new Arduino_ILI9225(bus, 33 /* RST */, 3 /* rotation */);

Arduino_DataBus *bus = new Arduino_ESP32SPI(27 /* DC */, 12 /* CS */, 18 /* SCK */, 23 /* MOSI */, 19 /* MISO */);
Arduino_GFX *gfx = new Arduino_ILI9341(bus, 26 /* RST */, 1 /* rotation */);

#define TFT_BRIGHTNESS 128
#define TFT_BL 22

#endif /* not a specific hardware */

#include "MjpegClass.h"

static MjpegClass mjpeg;
uint8_t *mjpeg_buf;

File root;

void printDirectory(File dir)
{
  while (true)
  {
    File vFile = dir.openNextFile();
    if (!vFile)
    {
      return;
    }
  
    if (!mjpeg_buf)
    {
      Serial.println(F("mjpeg_buf malloc failed!"));
    }
    else
    {
      Serial.println(F("MJPEG video start"));
      Serial.println(vFile.name());
      mjpeg.setup(vFile, mjpeg_buf, (Arduino_TFT *)gfx, true);
      // Read video
      while (mjpeg.readMjpegBuf())
      {
        // Play video
        mjpeg.drawJpg();
      }
      Serial.println(F("MJPEG video end"));
      vFile.close();
    }
  
    vFile.close();
  }
}

void setup()
{
  mjpeg_buf = (uint8_t *)malloc(MJPEG_BUFFER_SIZE);

  WiFi.mode(WIFI_OFF);
  Serial.begin(9600);

  // Init Video
  gfx->begin();
  gfx->fillScreen(BLACK);

  ledcAttachPin(TFT_BL, 1);     // assign TFT_BL pin to channel 1
  ledcSetup(1, 12000, 8);       // 12 kHz PWM, 8-bit resolution
  ledcWrite(1, TFT_BRIGHTNESS); // brightness 0 - 255

  SPIClass spi = SPIClass(HSPI);
  spi.begin(14 /* SCK */, 2 /* MISO */, 4 /* MOSI */, 25 /* SS */);
  // Init SD card
  if (!SD.begin(25 /* SS */, spi, 80000000)) /* SPI bus mode */
  {
    Serial.println(F("ERROR: SD card mount failed!"));
    gfx->println(F("ERROR: SD card mount failed!"));
    return;
  }
  else
  {
    File root = SD.open("/data");
    if (!root)
    {
      Serial.println(F("ERROR: Failed to open root data for reading"));
      gfx->println(F("ERROR: Failed to open root data file for reading"));
      return;
    }
    else
    { 
      while (true)
      {
        printDirectory(root);
        root.rewindDirectory();
      }
    }
  } 
  free(mjpeg_buf);
}

void loop() {}
