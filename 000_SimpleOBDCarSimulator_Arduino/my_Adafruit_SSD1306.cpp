/*!
 * @file Adafruit_SSD1306.cpp
 *
 * @mainpage Arduino library for monochrome OLEDs based on SSD1306 drivers.
 *
 * @section intro_sec Introduction
 *
 * This is documentation for Adafruit's SSD1306 library for monochrome
 * OLED displays: http://www.adafruit.com/category/63_98
 *
 * These displays use I2C or SPI to communicate. I2C requires 2 pins
 * (SCL+SDA) and optionally a RESET pin. SPI requires 4 pins (MOSI, SCK,
 * select, data/command) and optionally a reset pin. Hardware SPI or
 * 'bitbang' software SPI are both supported.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * @section dependencies Dependencies
 *
 * This library depends on <a
 * href="https://github.com/adafruit/Adafruit-GFX-Library"> Adafruit_GFX</a>
 * being present on your system. Please make sure you have installed the latest
 * version before using this library.
 *
 * @section author Author
 *
 * Written by Limor Fried/Ladyada for Adafruit Industries, with
 * contributions from the open source community.
 *
 * @section license License
 *
 * BSD license, all text above, and the splash screen included below,
 * must be included in any redistribution.
 *
 */

#include <avr/pgmspace.h>
#include <util/delay.h>

#include "my_Adafruit_SSD1306.h"

// SOME DEFINES AND STATIC VARIABLES USED INTERNALLY -----------------------

#if defined(I2C_BUFFER_LENGTH)
#define WIRE_MAX min(256, I2C_BUFFER_LENGTH) ///< Particle or similar Wire lib
#elif defined(BUFFER_LENGTH)
#define WIRE_MAX min(256, BUFFER_LENGTH) ///< AVR or similar Wire lib
#elif defined(SERIAL_BUFFER_SIZE)
#define WIRE_MAX                                                               \
  min(255, SERIAL_BUFFER_SIZE - 1) ///< Newer Wire uses RingBuffer
#else
#define WIRE_MAX 32 ///< Use common Arduino core default
#endif

#define ssd1306_swap(a, b)                                                     \
  (((a) ^= (b)), ((b) ^= (a)), ((a) ^= (b))) ///< No-temp-var swap operation

#define WIRE_WRITE wire->write ///< Wire write function in recent Arduino lib

#define SETWIRECLOCK wire->setClock(wireClk)    ///< Set before I2C transfer
#define RESWIRECLOCK wire->setClock(restoreClk) ///< Restore after I2C xfer

// The definition of 'transaction' is broadened a bit in the context of
// this library -- referring not just to SPI transactions (if supported
// in the version of the SPI library being used), but also chip select
// (if SPI is being used, whether hardware or soft), and also to the
// beginning and end of I2C transfers (the Wire clock may be sped up before
// issuing data to the display, then restored to the default rate afterward
// so other I2C device types still work).  All of these are encapsulated
// in the TRANSACTION_* macros.

// Check first if Wire, then hardware SPI, then soft SPI:
#define TRANSACTION_START                                                      \
    SETWIRECLOCK;
#define TRANSACTION_END                                                        \
    RESWIRECLOCK;

// CONSTRUCTORS, DESTRUCTOR ------------------------------------------------

/*!
    @brief  Constructor for I2C-interfaced SSD1306 displays.
    @param  w
            Display width in pixels
    @param  h
            Display height in pixels
    @param  twi
            Pointer to an existing TwoWire instance (e.g. &Wire, the
            microcontroller's primary I2C bus).
    @param  rst_pin
            Reset pin (using Arduino pin numbering), or -1 if not used
            (some displays might be wired to share the microcontroller's
            reset pin).
    @param  clkDuring
            Speed (in Hz) for Wire transmissions in SSD1306 library calls.
            Defaults to 400000 (400 KHz), a known 'safe' value for most
            microcontrollers, and meets the SSD1306 datasheet spec.
            Some systems can operate I2C faster (800 KHz for ESP32, 1 MHz
            for many other 32-bit MCUs), and some (perhaps not all)
            SSD1306's can work with this -- so it's optionally be specified
            here and is not a default behavior. (Ignored if using pre-1.5.7
            Arduino software, which operates I2C at a fixed 100 KHz.)
    @param  clkAfter
            Speed (in Hz) for Wire transmissions following SSD1306 library
            calls. Defaults to 100000 (100 KHz), the default Arduino Wire
            speed. This is done rather than leaving it at the 'during' speed
            because other devices on the I2C bus might not be compatible
            with the faster rate. (Ignored if using pre-1.5.7 Arduino
            software, which operates I2C at a fixed 100 KHz.)
    @return Adafruit_SSD1306 object.
    @note   Call the object's begin() function before use -- buffer
            allocation is performed there!
*/
Adafruit_SSD1306::Adafruit_SSD1306(uint8_t w, uint8_t h, TwoWire *twi,
                                   int8_t rst_pin, uint32_t clkDuring,
                                   uint32_t clkAfter)
    : Adafruit_GFX(w, h), wire(twi ? twi : &Wire), buffer(NULL),
      rstPin(rst_pin)
      , wireClk(clkDuring), restoreClk(clkAfter)
{
}

/*!
    @brief  Destructor for Adafruit_SSD1306 object.
*/
Adafruit_SSD1306::~Adafruit_SSD1306(void) {
}

// LOW-LEVEL UTILS ---------------------------------------------------------

/*!
    @brief Issue single command to SSD1306, using I2C or hard/soft SPI as
   needed. Because command calls are often grouped, SPI transaction and
   selection must be started/ended in calling function for efficiency. This is a
   protected function, not exposed (see ssd1306_command() instead).

        @param c
                   the command character to send to the display.
                   Refer to ssd1306 data sheet for commands
    @return None (void).
    @note
*/
void Adafruit_SSD1306::ssd1306_command1(uint8_t c) {
    wire->beginTransmission(i2caddr);
    WIRE_WRITE((uint8_t)0x00); // Co = 0, D/C = 0
    WIRE_WRITE(c);
    wire->endTransmission();
}

/*!
    @brief Issue list of commands to SSD1306, same rules as above re:
   transactions. This is a protected function, not exposed.
        @param c
                   pointer to list of commands

        @param n
                   number of commands in the list

    @return None (void).
    @note
*/
void Adafruit_SSD1306::ssd1306_commandList(const uint8_t *c, uint8_t n) {
    wire->beginTransmission(i2caddr);
    WIRE_WRITE((uint8_t)0x00); // Co = 0, D/C = 0
    uint16_t bytesOut = 1;
    while (n--) {
      if (bytesOut >= WIRE_MAX) {
        wire->endTransmission();
        wire->beginTransmission(i2caddr);
        WIRE_WRITE((uint8_t)0x00); // Co = 0, D/C = 0
        bytesOut = 1;
      }
      WIRE_WRITE(pgm_read_byte(c++));
      bytesOut++;
    }
    wire->endTransmission();
}

// A public version of ssd1306_command1(), for existing user code that
// might rely on that function. This encapsulates the command transfer
// in a transaction start/end, similar to old library's handling of it.
/*!
    @brief  Issue a single low-level command directly to the SSD1306
            display, bypassing the library.
    @param  c
            Command to issue (0x00 to 0xFF, see datasheet).
    @return None (void).
*/
void Adafruit_SSD1306::ssd1306_command(uint8_t c) {
  TRANSACTION_START
  ssd1306_command1(c);
  TRANSACTION_END
}

// ALLOCATE & INIT DISPLAY -------------------------------------------------

/*!
    @brief  Allocate RAM for image buffer, initialize peripherals and pins.
    @param  vcs
            VCC selection. Pass SSD1306_SWITCHCAPVCC to generate the display
            voltage (step up) from the 3.3V source, or SSD1306_EXTERNALVCC
            otherwise. Most situations with Adafruit SSD1306 breakouts will
            want SSD1306_SWITCHCAPVCC.
    @param  addr
            I2C address of corresponding SSD1306 display (or pass 0 to use
            default of 0x3C for 128x32 display, 0x3D for all others).
            SPI displays (hardware or software) do not use addresses, but
            this argument is still required (pass 0 or any value really,
            it will simply be ignored). Default if unspecified is 0.
    @param  reset
            If true, and if the reset pin passed to the constructor is
            valid, a hard reset will be performed before initializing the
            display. If using multiple SSD1306 displays on the same bus, and
            if they all share the same reset pin, you should only pass true
            on the first display being initialized, false on all others,
            else the already-initialized displays would be reset. Default if
            unspecified is true.
    @param  periphBegin
            If true, and if a hardware peripheral is being used (I2C or SPI,
            but not software SPI), call that peripheral's begin() function,
            else (false) it has already been done in one's sketch code.
            Cases where false might be used include multiple displays or
            other devices sharing a common bus, or situations on some
            platforms where a nonstandard begin() function is available
            (e.g. a TwoWire interface on non-default pins, as can be done
            on the ESP8266 and perhaps others).
    @return true on successful allocation/init, false otherwise.
            Well-behaved code should check the return value before
            proceeding.
    @note   MUST call this function before any drawing or updates!
*/
bool Adafruit_SSD1306::begin(uint8_t addr, bool reset, bool periphBegin) {

  if ((!buffer) && !(buffer = (uint8_t *)malloc(WIDTH * ((HEIGHT + 7) / 8))))
    return false;

  clearDisplay();

  // Setup pin directions
  // If I2C address is unspecified, use default
  // (0x3C for 32-pixel-tall displays, 0x3D for all others).

  //i2caddr = addr ? addr : ((HEIGHT == 32) ? 0x3C : 0x3C);
  i2caddr = 0x3C;

  // TwoWire begin() function might be already performed by the calling
  // function if it has unusual circumstances (e.g. TWI variants that
  // can accept different SDA/SCL pins, or if two SSD1306 instances
  // with different addresses -- only a single begin() is needed).
  if (periphBegin)
    wire->begin();

  // Reset SSD1306 if requested and reset pin specified in constructor
  if (reset && (rstPin >= 0)) {
    pinMode(rstPin, OUTPUT);
    digitalWrite(rstPin, HIGH);
    delay(1);                   // VDD goes high at start, pause for 1 ms
    digitalWrite(rstPin, LOW);  // Bring reset low
    delay(10);                  // Wait 10 ms
    digitalWrite(rstPin, HIGH); // Bring out of reset
  }

  TRANSACTION_START

  // Init sequence
  static const uint8_t PROGMEM init1[] = {SSD1306_DISPLAYOFF,         // 0xAE
                                          SSD1306_SETDISPLAYCLOCKDIV, // 0xD5
                                          0x80, // the suggested ratio 0x80
                                          SSD1306_SETMULTIPLEX}; // 0xA8
  ssd1306_commandList(init1, sizeof(init1));
  ssd1306_command1(HEIGHT - 1);

  static const uint8_t PROGMEM init2[] = {SSD1306_SETDISPLAYOFFSET, // 0xD3
                                          0x0,                      // no offset
                                          SSD1306_SETSTARTLINE | 0x0, // line #0
                                          SSD1306_CHARGEPUMP};        // 0x8D
  ssd1306_commandList(init2, sizeof(init2));

  ssd1306_command1(0x14);

  static const uint8_t PROGMEM init3[] = {SSD1306_MEMORYMODE, // 0x20
                                          0x00, // 0x0 act like ks0108
                                          SSD1306_SEGREMAP | 0x1,
                                          SSD1306_COMSCANDEC};
  ssd1306_commandList(init3, sizeof(init3));

  uint8_t comPins = 0x02;
  contrast = 0x8F;

  //if ((WIDTH == 128) && (HEIGHT == 32)) {
    comPins = 0x02;
    contrast = 0x8F;
  // } else if ((WIDTH == 128) && (HEIGHT == 64)) {
  //   comPins = 0x12;
  //   contrast = 0xCF;
  // } else if ((WIDTH == 96) && (HEIGHT == 16)) {
  //   comPins = 0x2; // ada x12
  //   contrast = 0xAF;
  // } else {
  //   // Other screen varieties -- TBD
  // }

  ssd1306_command1(SSD1306_SETCOMPINS);
  ssd1306_command1(comPins);
  ssd1306_command1(SSD1306_SETCONTRAST);
  ssd1306_command1(contrast);

  ssd1306_command1(SSD1306_SETPRECHARGE); // 0xd9
  ssd1306_command1(0xF1);
  static const uint8_t PROGMEM init5[] = {
      SSD1306_SETVCOMDETECT, // 0xDB
      0x40,
      SSD1306_DISPLAYALLON_RESUME, // 0xA4
      SSD1306_NORMALDISPLAY,       // 0xA6
      SSD1306_DEACTIVATE_SCROLL,
      SSD1306_DISPLAYON}; // Main screen turn on
  ssd1306_commandList(init5, sizeof(init5));

  TRANSACTION_END

  return true; // Success
}

// DRAWING FUNCTIONS -------------------------------------------------------

/*!
    @brief  Set/clear/invert a single pixel. This is also invoked by the
            Adafruit_GFX library in generating many higher-level graphics
            primitives.
    @param  x
            Column of display -- 0 at left to (screen width - 1) at right.
    @param  y
            Row of display -- 0 at top to (screen height -1) at bottom.
    @param  color
            Pixel color, one of: SSD1306_BLACK, SSD1306_WHITE or
            SSD1306_INVERSE.
    @return None (void).
    @note   Changes buffer contents only, no immediate effect on display.
            Follow up with a call to display(), or with other graphics
            commands as needed by one's own application.
*/
void Adafruit_SSD1306::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x >= 0) && (x < width()) && (y >= 0) && (y < height())) {
    //rotate 180
    x = WIDTH - x - 1;
    y = HEIGHT - y - 1;

    switch (color) {
    case SSD1306_WHITE:
      buffer[x + (y / 8) * WIDTH] |= (1 << (y & 7));
      break;
    case SSD1306_BLACK:
      buffer[x + (y / 8) * WIDTH] &= ~(1 << (y & 7));
      break;
    //case SSD1306_INVERSE:
    //  buffer[x + (y / 8) * WIDTH] ^= (1 << (y & 7));
    //  break;
    }
  }
}

/*!
    @brief  Clear contents of display buffer (set all pixels to off).
    @return None (void).
    @note   Changes buffer contents only, no immediate effect on display.
            Follow up with a call to display(), or with other graphics
            commands as needed by one's own application.
*/
void Adafruit_SSD1306::clearDisplay(void) {
  memset(buffer, 0, WIDTH * ((HEIGHT + 7) / 8));
}

/*!
    @brief  Draw a horizontal line. This is also invoked by the Adafruit_GFX
            library in generating many higher-level graphics primitives.
    @param  x
            Leftmost column -- 0 at left to (screen width - 1) at right.
    @param  y
            Row of display -- 0 at top to (screen height -1) at bottom.
    @param  w
            Width of line, in pixels.
    @param  color
            Line color, one of: SSD1306_BLACK, SSD1306_WHITE or SSD1306_INVERSE.
    @return None (void).
    @note   Changes buffer contents only, no immediate effect on display.
            Follow up with a call to display(), or with other graphics
            commands as needed by one's own application.
*/
void Adafruit_SSD1306::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    //rotate 180 degrees
    x = WIDTH - x - 1;
    y = HEIGHT - y - 1;
    x -= (w - 1);

    drawFastHLineInternal(x, y, w, color);
}

/*!
    @brief  Draw a horizontal line with a width and color. Used by public
   methods drawFastHLine,drawFastVLine
        @param x
                   Leftmost column -- 0 at left to (screen width - 1) at right.
        @param y
                   Row of display -- 0 at top to (screen height -1) at bottom.
        @param w
                   Width of line, in pixels.
        @param color
               Line color, one of: SSD1306_BLACK, SSD1306_WHITE or
   SSD1306_INVERSE.
    @return None (void).
    @note   Changes buffer contents only, no immediate effect on display.
            Follow up with a call to display(), or with other graphics
            commands as needed by one's own application.
*/
void Adafruit_SSD1306::drawFastHLineInternal(int16_t x, int16_t y, int16_t w, uint16_t color) {

  if ((y >= 0) && (y < HEIGHT)) { // Y coord in bounds?
    if (x < 0) {                  // Clip left
      w += x;
      x = 0;
    }
    if ((x + w) > WIDTH) { // Clip right
      w = (WIDTH - x);
    }
    if (w > 0) { // Proceed only if width is positive
      uint8_t *pBuf = &buffer[(y / 8) * WIDTH + x], mask = 1 << (y & 7);
      switch (color) {
      case SSD1306_WHITE:
        while (w--) {
          *pBuf++ |= mask;
        };
        break;
      case SSD1306_BLACK:
        mask = ~mask;
        while (w--) {
          *pBuf++ &= mask;
        };
        break;
      //case SSD1306_INVERSE:
      //  while (w--) {
      //    *pBuf++ ^= mask;
      //  };
      //  break;
      }
    }
  }
}

/*!
    @brief  Draw a vertical line. This is also invoked by the Adafruit_GFX
            library in generating many higher-level graphics primitives.
    @param  x
            Column of display -- 0 at left to (screen width -1) at right.
    @param  y
            Topmost row -- 0 at top to (screen height - 1) at bottom.
    @param  h
            Height of line, in pixels.
    @param  color
            Line color, one of: SSD1306_BLACK, SSD1306_WHITE or SSD1306_INVERSE.
    @return None (void).
    @note   Changes buffer contents only, no immediate effect on display.
            Follow up with a call to display(), or with other graphics
            commands as needed by one's own application.
*/
void Adafruit_SSD1306::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    drawFastVLineInternal(x, y, h, color);
}

/*!
    @brief  Draw a vertical line with a width and color. Used by public method
   drawFastHLine,drawFastVLine
        @param x
                   Leftmost column -- 0 at left to (screen width - 1) at right.
        @param __y
                   Row of display -- 0 at top to (screen height -1) at bottom.
        @param __h height of the line in pixels
        @param color
                   Line color, one of: SSD1306_BLACK, SSD1306_WHITE or
   SSD1306_INVERSE.
    @return None (void).
    @note   Changes buffer contents only, no immediate effect on display.
            Follow up with a call to display(), or with other graphics
            commands as needed by one's own application.
*/
void Adafruit_SSD1306::drawFastVLineInternal(int16_t x, int16_t __y, int16_t __h, uint16_t color) {

  if ((x >= 0) && (x < WIDTH)) { // X coord in bounds?
    if (__y < 0) {               // Clip top
      __h += __y;
      __y = 0;
    }
    if ((__y + __h) > HEIGHT) { // Clip bottom
      __h = (HEIGHT - __y);
    }
    if (__h > 0) { // Proceed only if height is now positive
      // this display doesn't need ints for coordinates,
      // use local byte registers for faster juggling
      uint8_t y = __y, h = __h;
      uint8_t *pBuf = &buffer[(y / 8) * WIDTH + x];

      // do the first partial byte, if necessary - this requires some masking
      uint8_t mod = (y & 7);
      if (mod) {
        // mask off the high n bits we want to set
        mod = 8 - mod;
        // note - lookup table results in a nearly 10% performance
        // improvement in fill* functions
        // uint8_t mask = ~(0xFF >> mod);
        static const uint8_t PROGMEM premask[8] = {0x00, 0x80, 0xC0, 0xE0,
                                                   0xF0, 0xF8, 0xFC, 0xFE};
        uint8_t mask = pgm_read_byte(&premask[mod]);
        // adjust the mask if we're not going to reach the end of this byte
        if (h < mod)
          mask &= (0XFF >> (mod - h));

        switch (color) {
        case SSD1306_WHITE:
          *pBuf |= mask;
          break;
        case SSD1306_BLACK:
          *pBuf &= ~mask;
          break;
        //case SSD1306_INVERSE:
        //  *pBuf ^= mask;
        //  break;
        }
        pBuf += WIDTH;
      }

      if (h >= mod) { // More to go?
        h -= mod;
        // Write solid bytes while we can - effectively 8 rows at a time
        if (h >= 8) {
          if (color == SSD1306_INVERSE) {
            //// separate copy of the code so we don't impact performance of
            //// black/white write version with an extra comparison per loop
            //do {
            //  *pBuf ^= 0xFF; // Invert byte
            //  pBuf += WIDTH; // Advance pointer 8 rows
            //  h -= 8;        // Subtract 8 rows from height
            //} while (h >= 8);
          } else {
            // store a local value to work with
            uint8_t val = (color != SSD1306_BLACK) ? 255 : 0;
            do {
              *pBuf = val;   // Set byte
              pBuf += WIDTH; // Advance pointer 8 rows
              h -= 8;        // Subtract 8 rows from height
            } while (h >= 8);
          }
        }

        if (h) { // Do the final partial byte, if necessary
          mod = h & 7;
          // this time we want to mask the low bits of the byte,
          // vs the high bits we did above
          // uint8_t mask = (1 << mod) - 1;
          // note - lookup table results in a nearly 10% performance
          // improvement in fill* functions
          static const uint8_t PROGMEM postmask[8] = {0x00, 0x01, 0x03, 0x07,
                                                      0x0F, 0x1F, 0x3F, 0x7F};
          uint8_t mask = pgm_read_byte(&postmask[mod]);
          switch (color) {
          case SSD1306_WHITE:
            *pBuf |= mask;
            break;
          case SSD1306_BLACK:
            *pBuf &= ~mask;
            break;
          //case SSD1306_INVERSE:
          //  *pBuf ^= mask;
          //  break;
          }
        }
      }
    } // endif positive height
  }   // endif x in bounds
}

// ! call before begin()
void Adafruit_SSD1306::setExternalBuffer(uint8_t* buff)
{
    buffer = buff;
}

// REFRESH DISPLAY ---------------------------------------------------------

/*!
    @brief  Push data currently in RAM to SSD1306 display.
    @return None (void).
    @note   Drawing operations are not visible until this function is
            called. Call after each graphics command, or after a whole set
            of graphics commands, as best needed by one's own application.
*/
void Adafruit_SSD1306::display(void) {
  TRANSACTION_START
  static const uint8_t PROGMEM dlist1[] = {
      SSD1306_PAGEADDR,
      0,                      // Page start address
      0xFF,                   // Page end (not really, but works here)
      SSD1306_COLUMNADDR, 0}; // Column start address
  ssd1306_commandList(dlist1, sizeof(dlist1));
  ssd1306_command1(WIDTH - 1); // Column end address

#if defined(ESP8266)
  // ESP8266 needs a periodic yield() call to avoid watchdog reset.
  // With the limited size of SSD1306 displays, and the fast bitrate
  // being used (1 MHz or more), I think one yield() immediately before
  // a screen write and one immediately after should cover it.  But if
  // not, if this becomes a problem, yields() might be added in the
  // 32-byte transfer condition below.
  yield();
#endif
  uint16_t count = WIDTH * ((HEIGHT + 7) / 8);
  uint8_t *ptr = buffer;
    wire->beginTransmission(i2caddr);
    WIRE_WRITE((uint8_t)0x40);
    uint16_t bytesOut = 1;
    while (count--) {
      if (bytesOut >= WIRE_MAX) {
        wire->endTransmission();
        wire->beginTransmission(i2caddr);
        WIRE_WRITE((uint8_t)0x40);
        bytesOut = 1;
      }
      WIRE_WRITE(*ptr++);
      bytesOut++;
    }
    wire->endTransmission();
  TRANSACTION_END
#if defined(ESP8266)
  yield();
#endif
}

// OTHER HARDWARE SETTINGS -------------------------------------------------

/*!
    @brief  Enable or disable display invert mode (white-on-black vs
            black-on-white).
    @param  i
            If true, switch to invert mode (black-on-white), else normal
            mode (white-on-black).
    @return None (void).
    @note   This has an immediate effect on the display, no need to call the
            display() function -- buffer contents are not changed, rather a
            different pixel mode of the display hardware is used. When
            enabled, drawing SSD1306_BLACK (value 0) pixels will actually draw
   white, SSD1306_WHITE (value 1) will draw black.
*/
void Adafruit_SSD1306::invertDisplay(bool i) {
  TRANSACTION_START
  ssd1306_command1(i ? SSD1306_INVERTDISPLAY : SSD1306_NORMALDISPLAY);
  TRANSACTION_END
}

/*!
    @brief  Dim the display.
    @param  dim
            true to enable lower brightness mode, false for full brightness.
    @return None (void).
    @note   This has an immediate effect on the display, no need to call the
            display() function -- buffer contents are not changed.
*/
void Adafruit_SSD1306::dim(bool dim) {
  // the range of contrast to too small to be really useful
  // it is useful to dim the display
  TRANSACTION_START
  ssd1306_command1(SSD1306_SETCONTRAST);
  ssd1306_command1(dim ? 0 : contrast);
  TRANSACTION_END
}
