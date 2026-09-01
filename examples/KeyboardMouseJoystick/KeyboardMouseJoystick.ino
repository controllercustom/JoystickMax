/*
  KeyboardMouseJoystick - one USB device, three HID functions, sent concurrently.

  Proves that JoystickMax (custom HID, Report ID 4/5) shares the same USB
  device as a standard USB Keyboard and a standard USB Mouse, and that all
  three are received by the host at the same time.

  Keyboard / Mouse come from the standard Arduino USB HID classes provided
  by each backend, each registering its own HID interface (or its own
  sub-descriptor on the legacy single-HID cores):

        legacy cores (AVR/SAMD/SAM/Renesas)          mbed (Giga R1/Portenta/RP2040)
        ─────────────────────────────────────────       ─────────────────────────────
        Keyboard  <Keyboard.h>  report id 2              USBKeyboard  own HID iface
        Mouse     <Mouse.h>     report id 1              USBMouse     own HID iface
        JoystickMax  report id 4/5                     JoystickMax  3rd  iface

  All three are driven in a single loop with a 30 ms tick; the host sees
  events on all of them simultaneously.
 */

#include <JoystickMax.h>

#if defined(ARDUINO_ARCH_MBED)
#include "USBKeyboard.h"
#include "USBMouse.h"
arduino::USBKeyboard Keyboard;   // mbed: instantiated here (core does not expose a global)
arduino::USBMouse     Mouse;
#elif defined(ARDUINO_ARCH_SAMD)
// SAMD per-ID split (mbed-style): avoid HID().AppendDescriptor 355 > _pack_buffer[256]
// JoystickMax stays on HID_ (254 B, single iface). Keyboard/Mouse get own PluggableUSBModule ifaces
// so host sees 3 HID ifaces like Giga (63/54/254) not merged 355. Fixes -110 timeout on SAMD21.
#include "api/PluggableUSB.h"
#include "HID.h"
static const uint8_t _kbdDescSAMD[] PROGMEM = {
  0x05,0x01, 0x09,0x06, 0xa1,0x01, 0x85,0x02, 0x05,0x07,
  0x19,0xe0, 0x29,0xe7, 0x15,0x00, 0x25,0x01, 0x75,0x01, 0x95,0x08, 0x81,0x02,
  0x95,0x01, 0x75,0x08, 0x81,0x03, 0x95,0x06, 0x75,0x08, 0x15,0x00, 0x25,0x73,
  0x05,0x07, 0x19,0x00, 0x29,0x73, 0x81,0x00, 0xc0
};
static const uint8_t _mouseDescSAMD[] PROGMEM = {
  0x05,0x01, 0x09,0x02, 0xa1,0x01, 0x09,0x01, 0xa1,0x00, 0x85,0x01, 0x05,0x09,
  0x19,0x01, 0x29,0x03, 0x15,0x00, 0x25,0x01, 0x95,0x03, 0x75,0x01, 0x81,0x02,
  0x95,0x01, 0x75,0x05, 0x81,0x03, 0x05,0x01, 0x09,0x30, 0x09,0x31, 0x09,0x38,
  0x15,0x81, 0x25,0x7f, 0x75,0x08, 0x95,0x03, 0x81,0x06, 0xc0, 0xc0
};
static unsigned int _samdKbdEpType[1] = { USB_ENDPOINT_TYPE_INTERRUPT | USB_ENDPOINT_IN(0) };
static unsigned int _samdMsEpType[1]  = { USB_ENDPOINT_TYPE_INTERRUPT | USB_ENDPOINT_IN(0) };
class SAMD_Keyboard_ : public arduino::PluggableUSBModule {
public: SAMD_Keyboard_(): arduino::PluggableUSBModule(1,1,_samdKbdEpType){ PluggableUSB().plug(this); }
protected:
  int getInterface(uint8_t* c){ *c+=1; HIDDescriptor d={ D_INTERFACE(pluggedInterface,1,USB_DEVICE_CLASS_HUMAN_INTERFACE,0,0), D_HIDREPORT(sizeof(_kbdDescSAMD)), D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint),USB_ENDPOINT_TYPE_INTERRUPT,0x40,0x01)}; return USBDevice.sendControl(&d,sizeof(d));}
  int getDescriptor(USBSetup& s){ if(s.bmRequestType!=REQUEST_DEVICETOHOST_STANDARD_INTERFACE) return 0; if(s.wValueH!=HID_REPORT_DESCRIPTOR_TYPE) return 0; if(s.wIndex!=pluggedInterface) return 0; return USBDevice.sendControl(_kbdDescSAMD,sizeof(_kbdDescSAMD));}
  bool setup(USBSetup& s){ if(pluggedInterface!=s.wIndex) return false; uint8_t r=s.bRequest, t=s.bmRequestType; if(t==REQUEST_DEVICETOHOST_CLASS_INTERFACE && r==HID_GET_REPORT) return true; if(t==REQUEST_DEVICETOHOST_CLASS_INTERFACE && r==HID_GET_PROTOCOL) return true; if(t==REQUEST_DEVICETOHOST_CLASS_INTERFACE && r==HID_GET_IDLE){ USBDevice.armSend(0,&_idle,1); return true;} if(t==REQUEST_HOSTTODEVICE_CLASS_INTERFACE && (r==HID_SET_REPORT||r==HID_SET_IDLE||r==HID_SET_PROTOCOL)) return true; return false;}
public:
  int SendReport(uint8_t id, const void* d, int l){ uint8_t p[64]; p[0]=id; memcpy(&p[1],d,l); return USBDevice.send(pluggedEndpoint,p,l+1);}
  void write(uint8_t c);
  void press(uint8_t k);
  void releaseAll();
private: uint8_t _idle=0;
};
class SAMD_Mouse_ : public arduino::PluggableUSBModule {
public: SAMD_Mouse_(): arduino::PluggableUSBModule(1,1,_samdMsEpType){ PluggableUSB().plug(this); }
protected:
  int getInterface(uint8_t* c){ *c+=1; HIDDescriptor d={ D_INTERFACE(pluggedInterface,1,USB_DEVICE_CLASS_HUMAN_INTERFACE,0,0), D_HIDREPORT(sizeof(_mouseDescSAMD)), D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint),USB_ENDPOINT_TYPE_INTERRUPT,0x40,0x01)}; return USBDevice.sendControl(&d,sizeof(d));}
  int getDescriptor(USBSetup& s){ if(s.bmRequestType!=REQUEST_DEVICETOHOST_STANDARD_INTERFACE) return 0; if(s.wValueH!=HID_REPORT_DESCRIPTOR_TYPE) return 0; if(s.wIndex!=pluggedInterface) return 0; return USBDevice.sendControl(_mouseDescSAMD,sizeof(_mouseDescSAMD));}
  bool setup(USBSetup& s){ if(pluggedInterface!=s.wIndex) return false; uint8_t r=s.bRequest, t=s.bmRequestType; if(t==REQUEST_DEVICETOHOST_CLASS_INTERFACE && r==HID_GET_REPORT) return true; if(t==REQUEST_DEVICETOHOST_CLASS_INTERFACE && r==HID_GET_PROTOCOL) return true; if(t==REQUEST_DEVICETOHOST_CLASS_INTERFACE && r==HID_GET_IDLE){ USBDevice.armSend(0,&_idle,1); return true;} if(t==REQUEST_HOSTTODEVICE_CLASS_INTERFACE && (r==HID_SET_REPORT||r==HID_SET_IDLE||r==HID_SET_PROTOCOL)) return true; return false;}
public:
  int SendReport(uint8_t id, const void* d, int l){ uint8_t p[64]; p[0]=id; memcpy(&p[1],d,l); return USBDevice.send(pluggedEndpoint,p,l+1);}
  void move(int8_t x,int8_t y,int8_t w=0);
  void press(uint8_t b);
  void release(uint8_t b);
private: uint8_t _idle=0; uint8_t _btn=0;
};
// — minimal boot Keyboard report helpers (no KeyboardLayout) —
void SAMD_Keyboard_::write(uint8_t c){ uint8_t key=0, mod=0; if(c>='a'&&c<='z') key=0x04 + (c - 'a'); else if(c>='A'&&c<='Z'){key=0x04 + (c - 'A'); mod=0x02;} else if(c>='1'&&c<='9') key=0x1E + (c - '1'); else if(c=='0') key=0x27; else if(c==' ') key=0x2c; else return; uint8_t rpt[8]={mod,0,key,0,0,0,0,0}; SendReport(2,rpt,8); delay(5); uint8_t rpt2[8]={0}; SendReport(2,rpt2,8);}
void SAMD_Keyboard_::press(uint8_t k){ (void)k;}
void SAMD_Keyboard_::releaseAll(){ uint8_t rpt[8]={0}; SendReport(2,rpt,8);}
void SAMD_Mouse_::move(int8_t x,int8_t y,int8_t w){ uint8_t rpt[4]={_btn,(uint8_t)x,(uint8_t)y,(uint8_t)w}; SendReport(1,rpt,4);}
void SAMD_Mouse_::press(uint8_t b){ _btn|=b; move(0,0,0);}
void SAMD_Mouse_::release(uint8_t b){ _btn&=~b; move(0,0,0);}
static SAMD_Keyboard_ SAMDKeyboard;
static SAMD_Mouse_ SAMDMouse;
#define Keyboard SAMDKeyboard
#define Mouse SAMDMouse
#else
#include <Keyboard.h>            // provides `Keyboard` global (Keyboard_) -> HID report id 2
#include <Mouse.h>               // provides `Mouse`    global (Mouse_)     -> HID report id 1
#endif

static const char *g_word = "JoystickMax";

void setup() {
  Serial.begin(115200);
  for (uint32_t t = 0; t < 2000 && !Serial; t++) delay(1);
  Serial.println(F("KeyboardMouseJoystick - KB(M) Mouse(M) Stick(JoystickMax) on one USB device"));
  JoystickMax.begin();
}

void loop() {
  // ---- Keyboard: type "JoystickMax", one letter per tick ----
  static uint8_t ck = 0;
  char c = g_word[ck++];
  if (ck >= strlen(g_word)) ck = 0;

#if defined(ARDUINO_ARCH_MBED)
  Keyboard.key_code(c);
#elif defined(ARDUINO_ARCH_SAMD)
  Keyboard.write(c);
#else
  Keyboard.write(c);
#endif

  // ---- Mouse: sweep X, alternate left button every 12 ticks, small wheel ----
  static int16_t cx = -40; static int8_t cdir = 8;
#if defined(ARDUINO_ARCH_MBED)
  Mouse.move(cdir, 0);
#elif defined(ARDUINO_ARCH_SAMD)
  Mouse.move(cdir, 0, 0);
#else
  Mouse.move((signed char)cdir, 0);
#endif
  cx += cdir;
  if (cx > 40 || cx < -40) cdir = (cdir > 0) ? -8 : 8;

  static uint8_t ct = 0;
  if (++ct >= 24) { ct = 0;
#if defined(ARDUINO_ARCH_MBED)
    Mouse.press(arduino::MOUSE_LEFT);
#elif defined(ARDUINO_ARCH_SAMD)
    Mouse.press(1);
#else
    Mouse.press(MOUSE_LEFT);
#endif
  } else if (ct == 12) {
#if defined(ARDUINO_ARCH_MBED)
    Mouse.release(arduino::MOUSE_LEFT);
#elif defined(ARDUINO_ARCH_SAMD)
    Mouse.release(1);
#else
    Mouse.release(MOUSE_LEFT);
#endif
  }

  // ---- Joystick: rotate button, rotate hat, random 11 axes, send report ----
  static uint8_t cb = 0;
  JoystickMax.releaseAll();
  JoystickMax.press(cb);
  cb = (cb + 1) % BUTTONS_MAX;

  static uint8_t ch = 0;
  JoystickMax.hat1(ch);
  JoystickMax.hat2(ch);
  ch = (ch + 1) % 8;

  JoystickMax.x(random((unsigned)AXIS_MAX + 1));
  JoystickMax.y(random((unsigned)AXIS_MAX + 1));
  JoystickMax.z(random((unsigned)AXIS_MAX + 1));
  JoystickMax.rx(random((unsigned)AXIS_MAX + 1));
  JoystickMax.ry(random((unsigned)AXIS_MAX + 1));
  JoystickMax.rz(random((unsigned)AXIS_MAX + 1));
  JoystickMax.rudder(random((unsigned)AXIS_MAX + 1));
  JoystickMax.throttle(random((unsigned)AXIS_MAX + 1));
  JoystickMax.accelerator(random((unsigned)AXIS_MAX + 1));
  JoystickMax.brake(random((unsigned)AXIS_MAX + 1));
  JoystickMax.steering(random((unsigned)AXIS_MAX + 1));
  JoystickMax.sendReport();

  delay(30);
}
