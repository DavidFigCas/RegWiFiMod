#define ASCII_TAB '\t' //!< Horizontal tab
#define ASCII_LF '\n'  //!< Line feed
#define ASCII_FF '\f'  //!< Form feed
#define ASCII_CR '\r'  //!< Carriage return
#define ASCII_DC2 18   //!< Device control 2
#define ASCII_ESC 27   //!< Escape
#define ASCII_FS 28    //!< Field separator
#define ASCII_GS 29    //!< Group separator

// === Character commands ===
#define FONT_MASK (1 << 0) //!< Select character font A or B
#define INVERSE_MASK                                                           \
  (1 << 1) //!< Turn on/off white/black reverse printing mode. Not in 2.6.8
           //!< firmware (see inverseOn())
#define UPDOWN_MASK (1 << 2)        //!< Turn on/off upside-down printing mode
#define BOLD_MASK (1 << 3)          //!< Turn on/off bold printing mode
#define DOUBLE_HEIGHT_MASK (1 << 4) //!< Turn on/off double-height printing mode
#define DOUBLE_WIDTH_MASK (1 << 5)  //!< Turn on/off double-width printing mode
#define STRIKE_MASK (1 << 6)        //!< Turn on/off deleteline mode

void adjustCharValues(uint8_t printMode) {
  uint8_t charWidth;
  if (printMode & FONT_MASK) {
    // FontB
    charHeight = 17;
    charWidth = 9;
  } else {
    // FontA
    charHeight = 24;
    charWidth = 12;
  }
  // Double Width Mode
  if (printMode & DOUBLE_WIDTH_MASK) {
    maxColumn /= 2;
    charWidth *= 2;
  }
  // Double Height Mode
  if (printMode & DOUBLE_HEIGHT_MASK) {
    charHeight *= 2;
  }
  maxColumn = (384 / charWidth);
}

void writePrintMode() {
  writeBytes(ASCII_ESC, '!', printMode);
}

void setPrintMode(uint8_t mask) {
  printMode |= mask;
  writePrintMode();
  adjustCharValues(printMode);
  // charHeight = (printMode & DOUBLE_HEIGHT_MASK) ? 48 : 24;
  // maxColumn = (printMode & DOUBLE_WIDTH_MASK) ? 16 : 32;
}

void unsetPrintMode(uint8_t mask) {
  printMode &= ~mask;
  writePrintMode();
  adjustCharValues(printMode);
  // charHeight = (printMode & DOUBLE_HEIGHT_MASK) ? 48 : 24;
  // maxColumn = (printMode & DOUBLE_WIDTH_MASK) ? 16 : 32;
}

void doubleHeightOn() { setPrintMode(DOUBLE_HEIGHT_MASK); }

void doubleHeightOff() { unsetPrintMode(DOUBLE_HEIGHT_MASK); }

void doubleWidthOn() { setPrintMode(DOUBLE_WIDTH_MASK); }

void doubleWidthOff() { unsetPrintMode(DOUBLE_WIDTH_MASK); }

void setSize(char value) {
  uint8_t size;

  switch (toupper(value)) {
  default: // Small: standard width and height
    // size = 0x00;
    // charHeight = 24;
    // maxColumn = 32;
    doubleWidthOff();
    doubleHeightOff();
    break;
  case 'M': // Medium: double height
    // size = 0x01;
    // charHeight = 48;
    // maxColumn = 32;
    doubleHeightOn();
    doubleWidthOff();
    break;
  case 'L': // Large: double width and height
    // size = 0x11;
    // charHeight = 48;
    // maxColumn = 16;
    doubleHeightOn();
    doubleWidthOn();
    break;
  }

  // writeBytes(ASCII_GS, '!', size);
  // prevByte = '\n'; // Setting the size adds a linefeed
}

void Adafruit_Thermal::timeoutSet(unsigned long x) {
  if (!dtrEnabled)
    resumeTime = micros() + x;
}


void writeBytes(uint8_t a) {
  timeoutWait();
  stream->write(a);
  timeoutSet(BYTE_TIME);
}

void writeBytes(uint8_t a, uint8_t b) {
  timeoutWait();
  stream->write(a);
  stream->write(b);
  timeoutSet(2 * BYTE_TIME);
}

void writeBytes(uint8_t a, uint8_t b, uint8_t c) {
  timeoutWait();
  stream->write(a);
  stream->write(b);
  stream->write(c);
  timeoutSet(3 * BYTE_TIME);
}

void writeBytes(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
  timeoutWait();
  stream->write(a);
  stream->write(b);
  stream->write(c);
  stream->write(d);
  timeoutSet(4 * BYTE_TIME);
}

// The underlying method for all high-level printing (e.g. println()).
// The inherited Print class handles the rest!
size_t write(uint8_t c) {

  if (c != 13) { // Strip carriage returns
    timeoutWait();
    stream->write(c);
    unsigned long d = BYTE_TIME;
    if ((c == '\n') || (column == maxColumn)) { // If newline or wrap
      d += (prevByte == '\n') ? ((charHeight + lineSpacing) * dotFeedTime)
                              : // Feed line
               ((charHeight * dotPrintTime) +
                (lineSpacing * dotFeedTime)); // Text line
      column = 0;
      c = '\n'; // Treat wrap as newline on next pass
    } else {
      column++;
    }
    timeoutSet(d);
    prevByte = c;
  }

  return 1;
}

void wake() {
  timeoutSet(0);   // Reset timeout counter
  writeBytes(255); // Wake
  if (firmware >= 264) {
    delay(50);
    writeBytes(ASCII_ESC, '8', 0, 0); // Sleep off (important!)
  } else {
    // Datasheet recommends a 50 mS delay before issuing further commands,
    // but in practice this alone isn't sufficient (e.g. text size/style
    // commands may still be misinterpreted on wake).  A slightly longer
    // delay, interspersed with NUL chars (no-ops) seems to help.
    for (uint8_t i = 0; i < 10; i++) {
      writeBytes(0);
      timeoutSet(10000L);
    }
  }
}

void reset() {
  writeBytes(ASCII_ESC, '@'); // Init command
  prevByte = '\n';            // Treat as if prior line is blank
  column = 0;
  maxColumn = 32;
  charHeight = 24;
  lineSpacing = 6;
  barcodeHeight = 50;

  if (firmware >= 264) {
    // Configure tab stops on recent printers
    writeBytes(ASCII_ESC, 'D'); // Set tab stops...
    writeBytes(4, 8, 12, 16);   // ...every 4 columns,
    writeBytes(20, 24, 28, 0);  // 0 marks end-of-list.
  }
}

void setHeatConfig(uint8_t dots, uint8_t time, uint8_t interval) {
  writeBytes(ASCII_ESC, '7');       // Esc 7 (print settings)
  writeBytes(dots, time, interval); // Heating dots, heat time, heat interval
}

void PrinterBegin(uint16_t version) {

  firmware = version;

  // The printer can't start receiving data immediately upon power up --
  // it needs a moment to cold boot and initialize.  Allow at least 1/2
  // sec of uptime before printer can receive data.
  timeoutSet(500000L);

  wake();
  reset();

  setHeatConfig();

  // Enable DTR pin if requested
  if (dtrPin < 255) {
    pinMode(dtrPin, INPUT_PULLUP);
    writeBytes(ASCII_GS, 'a', (1 << 5));
    dtrEnabled = true;
  }

  dotPrintTime = 30000; // See comments near top of file for
  dotFeedTime = 2100;   // an explanation of these values.
  maxChunkHeight = 255;
}
