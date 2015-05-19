/*
 * DTIIC - An Arduino project implementing an I2C-UART bridge.
 *
 * Copyright (C) 2015 Alex Beregszaszi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * This is an Arduino project implementing the same I2C-UART bridge as
 * Devantech's USB-ISS (http://www.robot-electronics.co.uk/htm/usb_iss_tech.htm).
 *
 * The USB-ISS module is capable of I2C, SPI, UART and GPIO, but we
 * are limiting the functionality to I2C only.  It is aimed to be
 * a cheap replacement, especially if someone has an Arduino
 * compatible board at his/her disposal.
 *
 * Sending direct I2C requests (the I2C_DIRECT command) is not supported.
 *
 * Turning on OLD_API aims to make this sketch compatible with the older
 * USB-I2C product from Devantech (http://www.robot-electronics.co.uk/htm/usb_i2c_tech.htm)
 *
 * TODO: rewrite in a fully asynchronic manner with a state machine?
 */

#define UART_SPEED	19200
#define SERIAL_NO	"9999001"
//#define OLD_API		1

#include <Wire.h>

void setup()
{
  Wire.begin();
  Wire.setClock(100000);
  Serial.begin(UART_SPEED);
  pinMode(13, OUTPUT);
}

static uint8_t get_byte() {
  while(Serial.available() == 0)
    delay(5);
  return Serial.read();
}

static inline void push_byte(uint8_t v)
{
  Serial.write(v);
}

static uint8_t get_i2c_byte() {
  while(Wire.available() == 0)
    delay(5);
  return Wire.read();
}

/*
 * Clear incoming buffer and push return value
 */
static void invalid(int ret = 0)
{
  // timeout on client side is 500ms
  // try to ensure we have really received the full command
  delay(50);
  
  // empty serial buffer
  while(Serial.available())
    Serial.read();

  // the usual error condition is signaled by a simple 0 byte
  push_byte(ret);
}

#ifndef OLD_API
static void process_iss_mode()
{
  uint8_t mode = get_byte();
  int freq;

  // remove the SERIAL flag
  int serial = mode & 1;
  mode &= 0xfe;
  
  get_byte();
  if (serial)
    get_byte();
    
  // Note: no distinction between SW / HW I2C
  switch(mode) {
    case 0x20: freq = 20000;  break;
    case 0x30: freq = 50000;  break;
    case 0x40:
    case 0x60: freq = 100000; break;
    case 0x50: 
    case 0x70: freq = 400000; break;
    default:
      invalid();
      push_byte(0x05);
      return;
  }
  
  Wire.setClock(freq);
  push_byte(0xff);
  push_byte(0);
}

static void process_iss_subcmd()
{
  uint8_t subcmd = get_byte();
  switch(subcmd) {
    case 0x01: // version
      push_byte(0x07); // module id
      push_byte(0x05); // firmware version (for I2C_STATUS)
      push_byte(0x60); // iss mode (100kHz HW I2C + GPIO)
      break;
    case 0x03: // serial
      Serial.print(SERIAL_NO);
      break;
    case 0x02:
      process_iss_mode();
      break;
    default:
      invalid();
      break;
  }
}
#else
static void process_usb_subcmd()
{
  uint8_t subcmd = get_byte();
  get_byte(); // data1
  get_byte(); // data2
  switch(subcmd) {
    case 0x01: // version
      push_byte(0x06); // firmware version
      break;
    case 0x02:
    case 0x03:
    case 0x10:
    case 0x11:
      push_byte(0);
      break;
    case 0x12:
      push_byte(0);
      push_byte(0);
      push_byte(0);
      push_byte(0);
      break;
    default:
      invalid();
      break;
  }
}
#endif

/*
 * Note: this cannot be supported by using the
 * Wire library. I am not sure if AVR HW itself
 * supports it, but in worst case it can be
 * implemented via bit-banging.
 *
 * FIXME: dummy
 */
static void process_i2c_direct()
{
  invalid(0); // also sends the NACK

  //push_byte(0x00); // NACK
  push_byte(0x01); // device error
  //push_byte(0x04); // unknown command
}

static void process_i2c_single()
{
   uint8_t addr = get_byte();
   uint8_t data = get_byte();
   int doread = addr & 1;
   addr >>= 1;
   
   if (doread) {
     Wire.requestFrom(addr, (uint8_t)1);
     push_byte(get_i2c_byte());
   } else {
     Wire.beginTransmission(addr);
     Wire.write(data);
     // endTransmission == 0 means success; return !0 on success
     push_byte(!!Wire.endTransmission());
   }
}

static void process_i2c_addr0() {
  uint8_t addr = get_byte();
  uint8_t count = get_byte();
  int doread = addr & 1;
  addr >>= 1;
  
  if (doread) {
     Wire.requestFrom(addr, count);
     while(count--)
       push_byte(get_i2c_byte());
  } else {
    Wire.beginTransmission(addr);
    while(count--)
      Wire.write(get_byte());
    // endTransmission == 0 means success; return !0 on success
    push_byte(!!Wire.endTransmission());
  }
}

static void process_i2c_addr12(int two = 0) {
  uint8_t addr = get_byte();
  uint8_t hi = get_byte();
  uint8_t lo = two ? get_byte() : 0;
  uint8_t count = get_byte();
  int doread = addr & 1;
  addr >>= 1;
  
  // send address first
  Wire.beginTransmission(addr);
  Wire.write(hi);
  if (two)
    Wire.write(lo);
  
  if (doread) {
     Wire.endTransmission();
     Wire.requestFrom(addr, count);
     while(count--)
       push_byte(get_i2c_byte());
  } else {
    while(count--)
      Wire.write(get_byte());
    // endTransmission == 0 means success; return !0 on success
    push_byte(!!Wire.endTransmission());
  }
}

static void process_i2c_test() {
  uint8_t addr = get_byte() >> 1;
  Wire.beginTransmission(addr);
  // endTransmission == 0 means success; return !0 on success
  push_byte(!!Wire.endTransmission());
}

#ifndef OLD_API
/*
 * The following are only implemented, because
 * the mode setting enables either GPIO or UART
 * and thus these can be called.
 */

/*
 * FIXME: dummy
 */
static void process_serial_io() {
  invalid(0xff); // failure
  push_byte(0);  // empty tx buffer
  push_byte(0);  // empty rx buffer
}

/*
 * FIXME: dummy
 */
static void process_setpins() {
  get_byte();
  push_byte(0xff); // pin status
}

/*
 * FIXME: dummy
 */
static void process_getpins() {
  push_byte(0xff); // pin status
}

/*
 * FIXME: dummy
 */
static void process_getad() {
  get_byte();
  push_byte(0);
  push_byte(0);
}
#endif

void loop()
{
}

void serialEvent() {
  digitalWrite(13, HIGH);
    
  uint8_t cmd = get_byte();
  
  switch(cmd) {
    case 0x53: process_i2c_single();  break;
    case 0x54: process_i2c_addr0();   break;
    case 0x55: process_i2c_addr12(0); break;
    case 0x56: process_i2c_addr12(1); break;
    case 0x57: process_i2c_direct();  break;
    case 0x58: process_i2c_test();    break;
#ifdef OLD_API
    case 0x5A: process_usb_subcmd();  break;
#else
    case 0x5A: process_iss_subcmd();  break;
    case 0x62: process_serial_io();   break;
    case 0x63: process_setpins();     break;
    case 0x64: process_getpins();     break;
    case 0x65: process_getad();       break;
#endif
    default:   invalid();             break;
  }
  Serial.flush();
  
  digitalWrite(13, LOW);
}
