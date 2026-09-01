#include <JoystickMax.h>

void setup() {
  Serial.begin(115200);
  while (!Serial && (millis() < 3000)) delay(10);
  Serial.println(F("JoystickMax Dual - ID 4 + ID 5  (254 B descriptor)"));
  Serial.print(F("sizeof report = ")); Serial.println(sizeof(JoystickMax_Report_t));
  JoystickMax.begin();
  JoystickMax2.begin();
}

void loop() {
  static uint8_t count = 0;
  if (count >= BUTTONS_MAX) {
    JoystickMax.releaseAll();
    JoystickMax2.releaseAll();
    count = 0;
  }
  JoystickMax.press(count);
  JoystickMax2.press((count + 16) % BUTTONS_MAX);
  count++;

  static uint8_t dir = 0;
  JoystickMax.hat1(dir);
  JoystickMax.hat2(dir);
  JoystickMax2.hat1((dir + 4) % 8);
  JoystickMax2.hat2((dir + 4) % 8);
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

  JoystickMax2.x(random(AXIS_MAX + 1));
  JoystickMax2.y(random(AXIS_MAX + 1));
  JoystickMax2.z(random(AXIS_MAX + 1));
  JoystickMax2.rx(random(AXIS_MAX + 1));
  JoystickMax2.ry(random(AXIS_MAX + 1));
  JoystickMax2.rz(random(AXIS_MAX + 1));
  JoystickMax2.rudder(random(AXIS_MAX + 1));
  JoystickMax2.throttle(random(AXIS_MAX + 1));
  JoystickMax2.accelerator(random(AXIS_MAX + 1));
  JoystickMax2.brake(random(AXIS_MAX + 1));
  JoystickMax2.steering(random(AXIS_MAX + 1));

  JoystickMax.sendReport();
  delay(5);
  JoystickMax2.sendReport();
  delay(95);
}
