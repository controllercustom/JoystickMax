#include <JoystickMax.h>

void setup() {
  Serial.begin(115200);
  while (!Serial && (millis() < 3000)) delay(10);
  Serial.println(F("JoystickMax - 32b/11a/2hats @ ~10 Hz  (Report ID 4)"));
  Serial.print(F("sizeof report = ")); Serial.println(sizeof(JoystickMax_Report_t));
  JoystickMax.begin();
}

void loop() {
  static uint8_t count = 0;
  if (count >= BUTTONS_MAX) {
    JoystickMax.releaseAll();
    count = 0;
  }
  JoystickMax.press(count);
  count++;

  static uint8_t dir = 0;
  JoystickMax.hat1(dir);
  JoystickMax.hat2(dir);
  dir++; if (dir > 7) dir = 0;

  JoystickMax.x(random(AXIS_MAX + 1));
  JoystickMax.y(random(AXIS_MAX + 1));
  JoystickMax.z(random(AXIS_MAX + 1));
  JoystickMax.rx(random(AXIS_MAX + 1));
  JoystickMax.ry(random(AXIS_MAX + 1));
  JoystickMax.rz(random(AXIS_MAX + 1));
  JoystickMax.rudder(random(AXIS_MAX + 1));
  JoystickMax.throttle(random(AXIS_MAX + 1));
  JoystickMax.accelerator(random(AXIS_MAX + 1));
  JoystickMax.brake(random(AXIS_MAX + 1));
  JoystickMax.steering(random(AXIS_MAX + 1));

  JoystickMax.sendReport();
  if (Serial && (count == 0 || count == 16)) {
    Serial.print(F("tick ")); Serial.println(count);
  }
  delay(100);
}
