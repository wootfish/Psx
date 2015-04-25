/*  PSX Controller Decoder Library (Psx.cpp)
    Written by: Kevin Ahrendt June 22nd, 2008

    Controller protocol implemented using Andrew J McCubbin's analysis.
    http://www.gamesx.com/controldata/psxcont/psxcont.htm

    Shift command is based on tutorial examples for ShiftIn and ShiftOut
    functions both written by Carlyn Maw and Tom Igoe
    http://www.arduino.cc/en/Tutorial/ShiftIn
    http://www.arduino.cc/en/Tutorial/ShiftOut

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
*/
#include "Psx.h"

Psx::Psx()
{
}

byte Psx::shift(byte _dataOut)                          // Does the actual shifting, both in and out simultaneously
{
    _temp = 0;
    _dataIn = 0;

    for (_i = 0; _i <= 7; _i++)
    {
        if ( _dataOut & (1 << _i) ) digitalWrite(_cmndPin, HIGH);   // Writes out the _dataOut bits
        else digitalWrite(_cmndPin, LOW);

        digitalWrite(_clockPin, LOW);

        delayMicroseconds(_delay);

        _temp = digitalRead(_dataPin);                  // Reads the data pin
        if (_temp)
        {
            _dataIn = _dataIn | (B10000000 >> _i);      // Shifts the read data into _dataIn
        }

        digitalWrite(_clockPin, HIGH);
        delayMicroseconds(_delay);
    }
    return _dataIn;
}


void Psx::setupPins(byte dataPin, byte cmndPin, byte attPin, byte clockPin, byte delay)
{
    pinMode(dataPin, INPUT);
    digitalWrite(dataPin, HIGH);    // Turn on internal pull-up
    _dataPin = dataPin;

    pinMode(cmndPin, OUTPUT);
    _cmndPin = cmndPin;

    pinMode(attPin, OUTPUT);
    _attPin = attPin;
    digitalWrite(_attPin, HIGH);

    pinMode(clockPin, OUTPUT);
    _clockPin = clockPin;
    digitalWrite(_clockPin, HIGH);

    _delay = delay;
}


unsigned int Psx::read()
{
    digitalWrite(_attPin, LOW);

    shift(0x01);
    shift(0x42);
    shift(0xFF);

    _data1 = ~shift(0xFF);
    _data2 = ~shift(0xFF);

    digitalWrite(_attPin, HIGH);

    _dataOut = (_data2 << 8) | _data1;

    return _dataOut;
}


unsigned int Psx::readRStick()
{
    digitalWrite(_attPin, LOW);

    shift(0x01); // some bs
    shift(0x42); // "ready to read"
    shift(0xFF); // clear the line & get "okay" from controller

    shift(0xFF); // skips some button data
    shift(0xFF); // skips the rest of the button data

    _data1 = ~shift(0xFF); // read right joystick's horizontal axis
    _data2 = ~shift(0xFF); // read same joystick's vertical axis

    // it sends more data at this point, but nothing we need

    digitalWrite(_attPin, HIGH); // telling the controller it can shut up now

    _dataOut = (_data2 << 8) | _data1;

    // this voodoo reverses the order of the first 16 bits
    // (which should be the only ones set at all)
    // this is necessary because the "shift" function reads data in the
    // opposite endianness from how we want it
    _dataOut = ((_dataOut & 0xFF00) >> 8) | ((_dataOut & 0x00FF) << 8); // swap the first & second halves
    _dataOut = ((_dataOut & 0xF0F0) >> 4) | ((_dataOut & 0x0F0F) << 4); // swap adjacent groups of 4 bits
    _dataOut = ((_dataOut & 0xCCCC) >> 2) | ((_dataOut & 0x3333) << 2); // swap adjacent groups of 2 bits
    _dataOut = ((_dataOut & 0xAAAA) >> 1) | ((_dataOut & 0x5555) << 1); // swap adjacent bits

    return _dataOut;
}


unsigned int Psx::readLStick()
{
    digitalWrite(_attPin, LOW);

    shift(0x01); // some bs
    shift(0x42); // "ready to read"
    shift(0xFF); // clear the line & get "okay" from controller

    shift(0xFF); // skips some button data
    shift(0xFF); // skips the rest of the button data

    shift(0xFF); // skips right joystick data
    shift(0xFF); // skips more right joystick data

    _data1 = ~shift(0xFF); // read left joystick's horizontal axis
    _data2 = ~shift(0xFF); // read same joystick's vertical axis

    // it sends more data but we can ignore it

    digitalWrite(_attPin, HIGH); // telling the controller it can shut up now

    _dataOut = (_data2 << 8) | _data1;

    // this voodoo reverses the order of the first 16 bits
    // (which should be the only ones set at all)
    // this is necessary because the "shift" function reads data in the
    // opposite endianness from how we want it
    _dataOut = ((_dataOut & 0xFF00) >> 8) | ((_dataOut & 0x00FF) << 8); // swap the first & second halves
    _dataOut = ((_dataOut & 0xF0F0) >> 4) | ((_dataOut & 0x0F0F) << 4); // swap adjacent groups of 4 bits
    _dataOut = ((_dataOut & 0xCCCC) >> 2) | ((_dataOut & 0x3333) << 2); // swap adjacent groups of 2 bits
    _dataOut = ((_dataOut & 0xAAAA) >> 1) | ((_dataOut & 0x5555) << 1); // swap adjacent bits

    return _dataOut;
}