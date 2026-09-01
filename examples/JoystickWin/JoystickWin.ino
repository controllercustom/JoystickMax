#include <JoystickWin.h>

// JoystickWin alias test — drop-in for JoystickMax
// This example proves #include <JoystickWin.h> and the JoystickWin
// global still compile and enumerate as 32b/11a/2hats (Report ID 4).
// New sketches should prefer #include <JoystickMax.h> and JoystickMax.

void setup() {
  Serial.begin(115200);
  while (!Serial && (millis() < 3000)) delay(10);
  Serial.println(F("JoystickWin alias -> JoystickMax - 32b/11a/2hats @ ~10 Hz"));
  Serial.print(F("sizeof report = "));
  Serial.println(sizeof(JoystickWin_Report_t));
  JoystickWin.begin();
}

void loop() {
  static uint8_t count = 0;
  if (count >= BUTTONS_MAX) {
    JoystickWin.releaseAll();
    count = 0;
  }
  JoystickWin.press(count);
  count++;

  static uint8_t dir = 0;
  JoystickWin.hat1(dir);
  JoystickWin.hat2(dir);
  dir++;
  if (dir > 7) dir = 0;

  JoystickWin.x(random(AXIS_MAX + 1));
  JoystickWin.y(random(AXIS_MAX + 1));
  JoystickWin.z(random(AXIS_MAX + 1));
  JoystickWin.rx(random(AXIS_MAX + 1));
  JoystickWin.ry(random(AXIS_MAX + 1));
  JoystickWin.rz(random(AXIS_MAX + 1));
  JoystickWin.rudder(random(AXIS_MAX + 1));
  JoystickWin.throttle(random(AXIS_MAX + 1));
  JoystickWin.accelerator(random(AXIS_MAX + 1));
  JoystickWin.brake(random(AXIS_MAX + 1));
  JoystickWin.steering(random(AXIS_MAX + 1));

  JoystickWin.sendReport();

  if (Serial && (count == 0 || count == 16)) {
    Serial.print(F("tick "));
    Serial.println(count);
  }
  delay(100);
}
