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
    This is the code that should go on the controller arduino.

    The code here is based in large part on Hardwired.ino . The sections which
    deal with bluetooth also draw heavily on the information found at:
    http://www.ram-e-shop.com/ds/general/Bluetooth_TRx_Module_New.pdf
*/

#include <Psx.h>
#include <SoftwareSerial.h>

#define dataPin 8
#define cmndPin 9
#define attPin 11
#define clockPin 10

#define RxD 6
#define TxD 7

#define FUDGE_LOWER 95
#define FUDGE_UPPER 160

#define BT_DELAY 50

//#define PAIR_ID_VAL "0"
#define PAIR_HW_ADDR = "aa:bb:cc:dd:ee:ff"
#define BT_DELAY_TIME 25

Psx Psx;
SoftwareSerial BT(RxD, TxD);

unsigned int RStick  = 0;
unsigned int RStickX = 0;
unsigned int RStickY = 0;

unsigned int LStick  = 0;
unsigned int LStickX = 0;
unsigned int LStickY = 0;

int leftServoVal = 0;
int rightServoVal = 0;

void setup() {
    Serial.begin(9600);

    // setting up the controller
    Psx.setupPins(dataPin, cmndPin, attPin, clockPin, 10);

    // setting up bluetooth
    // the code in here will start with the module 100% uninitialized and will
    // end with a link set up to the desired robot module, so that we can
    // devote the loop to reading & sending data over bluetooth serial.
    // we're optimists, so we assume we're getting sent "all clear" messages
    // back at every stage in this process. That simplifies the code a whole
    // bunch! Plus, come on, what would we even do with errors? Turn off?

    BT.begin(38400);
    delay(BT_DELAY * 3);
    BT.print("AT+ORGL\r\n");    // restores default device state

    // we're now in AT state with default params. let's set the relevant info

    // this code is used to set names. but we can't look up a remote device's
    // name unless we're paired with it, so that has limited usefulness to us.
    // currently we just bake in hardware addresses instead.
    //delay(BT_DELAY);
    //// we set the device name to e.g. ArdRemote0 or ArdRemote5 etc, depending
    //// on the value of the PAIR_ID_VAL preprocessor macro
    //char buf[32];
    //sprintf(buf, "AT+NAME=ArdRemote%s\r\n", PAIR_ID_VAL);
    //BT.print(buf);

    delay(BT_DELAY);
    BT.print("AT+PSWD=1234\r\n");
    clear_bt_buffer();

    // next we initialize the SPP profile lib, whatever that means
    delay(BT_DELAY);
    BT.print("AT+INIT\r\n");
    clear_bt_buffer();

    // this puts the device in pairable state
    delay(BT_DELAY);
    BT.print("AT+INQ\r\n");
    clear_bt_buffer();

    // this sets the device as a master (0=slave 1=master 2=some bs)
    delay(BT_DELAY);
    BT.print("AT+ROLE=1\r\n");
    clear_bt_buffer();

    delay(BT_DELAY);
    while (bt_pair(PAIR_HW_ADDR) != 1);
    delay(BT_DELAY * 2);
}

void loop() {
    LStick = Psx.readLStick();
    RStick = Psx.readRStick();

    LStickX = LStick >> 8;
    LStickY = LStick & 0xFF;

    RStickX = RStick >> 8;
    RStickY = RStick & 0xFF;

    // send: two 255 bytes, to mark transmission border
    //delay(BT_DELAY_TIME);
    //BT.write(255);
    //delay(BT_DELAY_TIME);
    //BT.write(255);

    if ((FUDGE_LOWER < LStickY) && (LStickY < FUDGE_UPPER)) {
        leftServoVal = 1500;
    } else {
        leftServoVal = 1245 + (2*LStickY);
    }

    if ((FUDGE_LOWER < RStickY) && (RStickY < FUDGE_UPPER)) {
        rightServoVal = 1500;
    } else {
        rightServoVal = 1755 - (2*RStickY)
    }

    // sending the data
    // order: LSBytes, MSBytes
    // for example, 1500 = bx00000101 11011100 // so to send 1500 we send:
    // 11011100 followed by 00000101
    delay(BT_DELAY_TIME);
    BT.write(leftServoVal >> 8);
    delay(BT_DELAY_TIME);
    BT.write(leftServoVal & 0xFF);

    delay(BT_DELAY_TIME);
    BT.write(rightServoVal >> 8);
    delay(BT_DELAY_TIME);
    BT.write(rightServoVal & 0xFF);
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
