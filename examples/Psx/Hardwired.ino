/*  Hardwired PSX -> BOEBot Code

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.


    INFORMATION:
    This code lets you physically wire up a PS1 controller to an Arduino. If
    the arduino is hooked up to some servos (as in e.g. the Parallax robotics
    shield https://www.sparkfun.com/products/11494 ), this can drive them.

    servo_____.writeMicroseconds(1500) to center a servo.
    servoLeft.writeMicroseconds(1700) goes forward on the left side
    servoLeft.writeMicroseconds(1300) goes backward on the left side

    servoRight.writeMicroseconds(1700) goes backward on the right side
    servoRight.writeMicroseconds(1300) goes forward on the right side

    http://www.gamesx.com/controldata/psxcont/psxcont.htm is the bible
    It will tell you all you need to know
*/

#include <Servo.h>
#include <Psx.h>

#define dataPin 8
#define cmndPin 9
#define attPin 11
#define clockPin 10

#define leftServo 12
#define rightServo 13

Psx Psx;
Servo servoLeft;
Servo servoRight;

unsigned int data = 0;

unsigned int RStick  = 0;
unsigned int RStickX = 0;
unsigned int RStickY = 0;

unsigned int LStick  = 0;
unsigned int LStickX = 0;
unsigned int LStickY = 0;

int fudgeLower = 95;
int fudgeUpper = 160;

void setup()
{
  Psx.setupPins(dataPin, cmndPin, attPin, clockPin, 10);
  Serial.begin(9600);

  servoLeft.attach(leftServo);
  servoRight.attach(rightServo);

  servoLeft.writeMicroseconds(1500);
  servoRight.writeMicroseconds(1500);
}

void loop()
{
  LStick = Psx.readLStick();
  RStick = Psx.readRStick();

  /* X coords == mask with 0xFF to isolate the least significant 8 bits */
  /* Y coords == shifted to isolate the most significant 8 bits */
  LStickX = LStick >> 8;
  LStickY = LStick & 0xFF;
  RStickX = RStick >> 8;
  RStickY = RStick & 0xFF;

  /* debug messages */
  //Serial.println(LStick);
  //Serial.println(RStick);
  //Serial.println("");
  //Serial.println(LStickX);
  Serial.println(LStickY);
  //Serial.println("");
  //Serial.println(RStickX);
  Serial.println(RStickY);
  Serial.println("");

  /* Neutral (no movement) is 1500. Stick values go 0 to 255.
     The approximate value we want for going forward is 1700,
     and going backward is 1300. So if you multiply the RStickY
     value by 2 and add it, you'll get ~1300 for min and ~1700
     for max.*/

  // the sticks don't sit quite at 127, so we have a small fudge factor
  if ((fudgeLower < LStickY) && (LStickY < fudgeUpper)) {
   servoLeft.writeMicroseconds(1500);
  } else {
    servoLeft.writeMicroseconds(1245 + (2*LStickY));
  }

  if ((fudgeLower < RStickY) && (RStickY < fudgeUpper)) {
    servoRight.writeMicroseconds(1500);
  } else {
    servoRight.writeMicroseconds(1755 - (2*RStickY));
  }

  /* Time in milliseconds between subsequent pollings of the controller. */
  delay(50);
}
