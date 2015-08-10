#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <ads7843.h>
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A7 // LCD Read goes to Analog 0
#define DCLK     2
#define CSS       A4
#define DIN      A0
#define DOUT     A5
#define IRQ      A6
#define SD_CS 10     // Set the chip select line to whatever you use (10 doesnt conflict with the library)
Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, A7);
ADS7843 touch(CSS, DCLK, DIN, DOUT, IRQ);
Point p;
int t_x, t_y;
uint32_t flag;
unsigned long time1 ;
void setup()
{
  Serial.begin(115200);
  tft.reset();
  delay(1000);
  uint16_t identifier = 0x9341;
  tft.begin(0x9341);
  if (!SD.begin(SD_CS)) {
    return;
  }
  tft.setRotation(3);
  touch.begin();
  bmpDraw("cmmc.bmp", 0, 0); time1 = millis();
}

void loop()
{
HOME:
  while (1) {
stap1:
    while (1) {
      p = touch.getpos(&flag) ;
      if (flag && p.x > 0 && p.y < 4095) { //
        touch_test();
        if (t_x > 0 && t_x < 50 && t_y > 0 && t_y < 50) {
          bmpDraw("menu.bmp", 0, 0);
          goto menu;
        }
      }
      if(millis()>time1+1000){ bmpDraw("tamp.bmp", 0, 0);time1 = millis();goto stap2; }
    }
stap2:
while(1){
       p = touch.getpos(&flag) ;
      if (flag && p.x > 0 && p.y < 4095) { //
        touch_test();
        if (t_x > 0 && t_x < 50 && t_y > 0 && t_y < 50) {
          bmpDraw("menu.bmp", 0, 0);
          goto menu;
        }
      }
      if(millis()>time1+1000){ bmpDraw("ph.bmp", 0, 0);time1 = millis();goto stap3; }
}
stap3:
while(1){
       p = touch.getpos(&flag) ;
      if (flag && p.x > 0 && p.y < 4095) { //
        touch_test();
        if (t_x > 0 && t_x < 50 && t_y > 0 && t_y < 50) {
          bmpDraw("menu.bmp", 0, 0);
          goto menu;
        }
      }
      if(millis()>time1+1000){ bmpDraw("cmmc.bmp", 0, 0);time1 = millis();goto stap1; }
}
  
}
menu:
while (1) {
    p = touch.getpos(&flag) ;
    if (flag && p.x > 0 && p.y < 4095) { //
      touch_test();
      if (t_x > 0 && t_x < 50 && t_y > 0 && t_y < 50) {
        //bmpDraw("cmmc.bmp", 0, 0);
        bmpDraw("cmmc.bmp", 0, 0); time1 = millis();
        goto HOME;
      }
    }
  }

}
void touch_test() {
  t_x = p.x;
  t_y = p.y;
  t_y = 4095 - t_y;
  if (t_x < 200)t_x = 200;
  if (t_y < 230)t_y = 230;
  if (t_y > 3800)t_y = 3800;
  if (t_x > 3800)t_x = 3800;

  t_x = map(t_x, 200, 3800, 0, 320);
  t_y = map(t_y, 230, 3800, 0, 240);
  //t_x = 320 - t_x;
  Serial.print(t_x);
  Serial.print("   ");
  Serial.print(t_y);
  Serial.println(" ");
}
#define BUFFPIXEL 20

void bmpDraw(char *filename, int x, int y) {

  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3 * BUFFPIXEL]; // pixel in buffer (R+G+B per pixel)
  uint16_t lcdbuffer[BUFFPIXEL];  // pixel out buffer (16-bit per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();
  uint8_t  lcdidx = 0;
  boolean  first = true;

  if ((x >= tft.width()) || (y >= tft.height())) return;
  if ((bmpFile = SD.open(filename)) == NULL) {
    return;
  }

  // Parse BMP header
  if (read16(bmpFile) == 0x4D42) { // BMP signature
    read32(bmpFile);
    (void)read32(bmpFile);
    bmpImageoffset = read32(bmpFile);
    read32(bmpFile);
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if (read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      if ((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed
        goodBmp = true; // Supported BMP format -- proceed!
        rowSize = (bmpWidth * 3 + 3) & ~3;
        if (bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if ((x + w - 1) >= tft.width())  w = tft.width()  - x;
        if ((y + h - 1) >= tft.height()) h = tft.height() - y;

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x + w - 1, y + h - 1);

        for (row = 0; row < h; row++) { // For each scanline...
          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if (flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if (bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col = 0; col < w; col++) { // For each column...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) { // Indeed
              // Push LCD buffer to the display first
              if (lcdidx > 0) {
                tft.pushColors(lcdbuffer, lcdidx, first);
                lcdidx = 0;
                first  = false;
              }
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            lcdbuffer[lcdidx++] = tft.color565(r, g, b);
          } // end pixel
        } // end scanline
        // Write any remaining data to LCD
        if (lcdidx > 0) {
          tft.pushColors(lcdbuffer, lcdidx, first);
        }
        Serial.print(F("Loaded in "));
        Serial.print(millis() - startTime);
        Serial.println(" ms");
      } // end goodBmp
    }
  }

  bmpFile.close();
  if (!goodBmp) Serial.println(F("BMP format not recognized."));
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

