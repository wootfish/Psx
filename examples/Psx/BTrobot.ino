/*  Bluetooth PSX -> BOEBot Code

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
    This is similar to Hardwired.ino in spirit, but it uses a Bluetooth
    connection between two Arduinos instead of one Arduino that's hardwired.
    This is the code that should go on the robot arduino.

    The code here is based in large part on Hardwired.ino . The sections which
    deal with bluetooth also draw heavily on the information found at:
    http://www.ram-e-shop.com/ds/general/Bluetooth_TRx_Module_New.pdf
*/

#include <SoftwareSerial.h>

#define RxD 6
#define TxD 7

#define leftServo 12
#define rightServo 13

#define PAIR_HW_ADDR = "aa:bb:cc:dd:ee:ff"
#define BT_DELAY_TIME 500

SoftwareSerial BT(RxD, TxD);

int leftServoVal = 0;
int rightServoVal = 0;

void setup() {
    // set up the servo hardware
    // the servos may need to be physically calibrated in addition to this
    servoLeft.attach(leftServo);
    servoRight.attach(rightServo);

    servoLeft.writeMicroseconds(1500);
    servoRight.writeMicroseconds(1500);

    // set up bluetooth
    // details of this process are outlined in BTcontroller.ino
    BT.begin(38400);
    delay(BT_DELAY * 3);
    BT.print("AT+ORGL\r\n");
    clear_bt_buffer();

    delay(BT_DELAY);
    BT.print("AT+PSWD=1234\r\n");
    clear_bt_buffer();

    delay(BT_DELAY);
    BT.print("AT+INIT\r\n");
    clear_bt_buffer();

    delay(BT_DELAY);
    BT.print("AT+INQ\r\n");
    clear_bt_buffer();

    // set as slave
    delay(BT_DELAY);
    BT.print("AT+ROLE=0\r\n");
    clear_bt_buffer();

    delay(BT_DELAY);
    while (bt_pair(PAIR_HW_ADDR) != 1);
    delay(BT_DELAY * 2);
}

void loop() {
    // this'll give us the left servo value, in two bytes
    // least significant bits
    while (BT.available() == 0);
    leftServoVal = BT.read();

    // most significant bits
    while (BT.available() == 0);
    leftServoVal += BT.read() << 8;

    // least sig
    while (BT.available() == 0);
    rightServoVal = BT.read();

    // most sig
    while (BT.available() == 0);
    rightServoVal += BT.read() << 8;

    // set the servos
    servoLeft.writeMicroseconds(leftServoVal);
    servoRight.writeMicroseconds(rightServoVal);
}

void clear_bt_buffer() {
    while (BT.available() > 0) BT.read();
}

int bt_pair(char* targetaddr) {
    char buf[32];
    sprintf(buf, "AT+LINK=%s\r\n", targetaddr);
    BT.print(buf);

    while (BT.available() == 0);

    if (BT.read() == 'O') {
        clear_bt_buffer();
        return 1;
    }

    clear_bt_buffer();
    return 0;
}
