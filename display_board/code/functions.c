 void setFullWindow()
    {
      _using_partial_mode = false;
      _pw_x = 0;
      _pw_y = 0;
      _pw_w = GxEPD2_Type::WIDTH;
      _pw_h = HEIGHT;
    }
 void firstPage()
    {
      fillScreen(GxEPD_WHITE);
      _current_page = 0;
      _second_phase = false;
    }
    
    bool nextPage()
    {
      if (1 == _pages)
      {
        if (_using_partial_mode)
        {
          uint32_t offset = _reverse ? (HEIGHT - _pw_h) * _pw_w / 8 : 0;
          epd2.writeImage(_buffer + offset, _pw_x, _pw_y, _pw_w, _pw_h);
          epd2.refresh(_pw_x, _pw_y, _pw_w, _pw_h);
          if (epd2.hasFastPartialUpdate)
          {
            epd2.writeImageAgain(_buffer + offset, _pw_x, _pw_y, _pw_w, _pw_h);
            //epd2.refresh(_pw_x, _pw_y, _pw_w, _pw_h); // not needed
          }
        }
        else // full update
        {
          epd2.writeImageForFullRefresh(_buffer, 0, 0, GxEPD2_Type::WIDTH, HEIGHT);
          epd2.refresh(false);
          if (epd2.hasFastPartialUpdate)
          {
            epd2.writeImageAgain(_buffer, 0, 0, GxEPD2_Type::WIDTH, HEIGHT);
            //epd2.refresh(true); // not needed
          }
          epd2.powerOff();
        }
        return false;
      }
      uint16_t page_ys = _current_page * _page_height;
      if (_using_partial_mode)
      {
        //Serial.print("  nextPage("); Serial.print(_pw_x); Serial.print(", "); Serial.print(_pw_y); Serial.print(", ");
        //Serial.print(_pw_w); Serial.print(", "); Serial.print(_pw_h); Serial.print(") P"); Serial.println(_current_page);
        uint16_t page_ye = _current_page < int16_t(_pages - 1) ? page_ys + _page_height : HEIGHT;
        uint16_t dest_ys = _pw_y + page_ys; // transposed
        uint16_t dest_ye = gx_uint16_min(_pw_y + _pw_h, _pw_y + page_ye);
        if (dest_ye > dest_ys)
        {
          //Serial.print("writeImage("); Serial.print(_pw_x); Serial.print(", "); Serial.print(dest_ys); Serial.print(", ");
          //Serial.print(_pw_w); Serial.print(", "); Serial.print(dest_ye - dest_ys); Serial.println(")");
          uint32_t offset = _reverse ? (_page_height - (dest_ye - dest_ys)) * _pw_w / 8 : 0;
          if (!_second_phase) epd2.writeImage(_buffer + offset, _pw_x, dest_ys, _pw_w, dest_ye - dest_ys);
          else epd2.writeImageAgain(_buffer + offset, _pw_x, dest_ys, _pw_w, dest_ye - dest_ys);
        }
        else
        {
          //Serial.print("writeImage("); Serial.print(_pw_x); Serial.print(", "); Serial.print(dest_ys); Serial.print(", ");
          //Serial.print(_pw_w); Serial.print(", "); Serial.print(dest_ye - dest_ys); Serial.print(") skipped ");
          //Serial.print(dest_ys); Serial.print(".."); Serial.println(dest_ye);
        }
        _current_page++;
        if (_current_page == int16_t(_pages))
        {
          _current_page = 0;
          if (!_second_phase)
          {
            epd2.refresh(_pw_x, _pw_y, _pw_w, _pw_h);
            if (epd2.hasFastPartialUpdate)
            {
              _second_phase = true;
              fillScreen(GxEPD_WHITE);
              return true;
            }
          }
          return false;
        }
        fillScreen(GxEPD_WHITE);
        return true;
      }
      else // full update
      {
        if (!_second_phase) epd2.writeImageForFullRefresh(_buffer, 0, page_ys, GxEPD2_Type::WIDTH, gx_uint16_min(_page_height, HEIGHT - page_ys));
        else epd2.writeImageAgain(_buffer, 0, page_ys, GxEPD2_Type::WIDTH, gx_uint16_min(_page_height, HEIGHT - page_ys));
        _current_page++;
        if (_current_page == int16_t(_pages))
        {
          _current_page = 0;
          if (epd2.hasFastPartialUpdate)
          {
            if (!_second_phase)
            {
              epd2.refresh(false); // full update after first phase
              _second_phase = true;
              fillScreen(GxEPD_WHITE);
              return true;
            }
            //else epd2.refresh(true); // partial update after second phase
          } else epd2.refresh(false); // full update after only phase
          epd2.powerOff();
          return false;
        }
        fillScreen(GxEPD_WHITE);
        return true;
      }
    }
    
void init(uint32_t serial_diag_bitrate, bool initial, uint16_t reset_duration = 10, bool pulldown_rst_mode = false)
    {
      epd2.init(serial_diag_bitrate, initial, reset_duration, pulldown_rst_mode);
      _using_partial_mode = false;
      _current_page = 0;
      setFullWindow();
    }
void fillScreen(uint16_t color) // 0x0 black, >0x0 white, to buffer
    {
      uint8_t data = (color == GxEPD_BLACK) ? 0x00 : 0xFF;
      for (uint16_t x = 0; x < sizeof(_buffer); x++)
      {
        _buffer[x] = data;
      }
    }
    
clearScreen(uint8_t value)
{
  writeScreenBuffer(value);
  refresh(true);
  writeScreenBuffer(value);
}

void GxEPD2_750_YT7::writeScreenBuffer(uint8_t value)
{
  if (!_using_partial_mode) _Init_Part();
  _writeScreenBuffer(0x13, value); // set current
  if (_initial_refresh) _writeScreenBuffer(0x10, 0x00); // set previous
  _initial_write = false; // initial full screen buffer clean done
}

void GxEPD2_750_YT7::_writeScreenBuffer(uint8_t command, uint8_t value)
{
  _writeCommand(command);
  _startTransfer();
  for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 8; i++)
  {
    _transfer(value);
  }
  _endTransfer();
}

void GxEPD2_750_YT7::refresh(bool partial_update_mode)
{
  if (partial_update_mode) refresh(0, 0, WIDTH, HEIGHT);
  else
  {
    if (_using_partial_mode) _Init_Full();
    _Update_Full();
    _initial_refresh = false; // initial full update done
  }
}

void GxEPD2_750_YT7::refresh(int16_t x, int16_t y, int16_t w, int16_t h)
{
  if (_initial_refresh) return refresh(false); // initial update needs be full update
  // intersection with screen
  int16_t w1 = x < 0 ? w + x : w; // reduce
  int16_t h1 = y < 0 ? h + y : h; // reduce
  int16_t x1 = x < 0 ? 0 : x; // limit
  int16_t y1 = y < 0 ? 0 : y; // limit
  w1 = x1 + w1 < int16_t(WIDTH) ? w1 : int16_t(WIDTH) - x1; // limit
  h1 = y1 + h1 < int16_t(HEIGHT) ? h1 : int16_t(HEIGHT) - y1; // limit
  if ((w1 <= 0) || (h1 <= 0)) return;
  // make x1, w1 multiple of 8
  w1 += x1 % 8;
  if (w1 % 8 > 0) w1 += 8 - w1 % 8;
  x1 -= x1 % 8;
  if (!_using_partial_mode) _Init_Part();
  if (usePartialUpdateWindow) _writeCommand(0x91); // partial in
  _setPartialRamArea(x1, y1, w1, h1);
  _Update_Part();
  if (usePartialUpdateWindow) _writeCommand(0x92); // partial out
}

void GxEPD2_750_YT7::_Init_Full()
{
  _InitDisplay();
  _writeCommand(0x00); // panel setting
  _writeData(0x1f);    // full update LUT from OTP
  _PowerOn();
  _using_partial_mode = false;
}

void GxEPD2_750_YT7::_Init_Part()
{
  _InitDisplay();
  _writeCommand(0x00); //panel setting
  _writeData(hasFastPartialUpdate ? 0x3f : 0x1f); // partial update LUT from registers
  _writeCommand(0x82); // vcom_DC setting
  _writeData (0x30); // -2.5V same value as in OTP
  _writeCommand(0x50); // VCOM AND DATA INTERVAL SETTING
  _writeData(0x39);    // LUTBD, N2OCP: copy new to old
  _writeData(0x07);
  _writeCommand(0x20);
  _writeDataPGM(lut_20_LUTC_partial, sizeof(lut_20_LUTC_partial), 42 - sizeof(lut_20_LUTC_partial));
  _writeCommand(0x21);
  _writeDataPGM(lut_21_LUTWW_partial, sizeof(lut_21_LUTWW_partial), 42 - sizeof(lut_21_LUTWW_partial));
  _writeCommand(0x22);
  _writeDataPGM(lut_22_LUTKW_partial, sizeof(lut_22_LUTKW_partial), 42 - sizeof(lut_22_LUTKW_partial));
  _writeCommand(0x23);
  _writeDataPGM(lut_23_LUTWK_partial, sizeof(lut_23_LUTWK_partial), 42 - sizeof(lut_23_LUTWK_partial));
  _writeCommand(0x24);
  _writeDataPGM(lut_24_LUTKK_partial, sizeof(lut_24_LUTKK_partial), 42 - sizeof(lut_24_LUTKK_partial));
  _writeCommand(0x25);
  _writeDataPGM(lut_25_LUTBD_partial, sizeof(lut_25_LUTBD_partial), 42 - sizeof(lut_25_LUTBD_partial));
  _PowerOn();
  _using_partial_mode = true;
}

void GxEPD2_750_YT7::_Update_Full()
{
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("_Update_Full", full_refresh_time);
}

void GxEPD2_750_YT7::_Update_Part()
{
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("_Update_Part", partial_refresh_time);
}

