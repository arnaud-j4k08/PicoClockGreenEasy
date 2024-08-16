#pragma once

enum Gpio
{
    K2 = 2,         // Input:  top button
    SDA = 6,        // Output: I2C data line to DS3231
    SCL = 7,        // Output: I2C clock line to DS3231
    CLK = 10,       // Output: clock line for LED matrix controller
    SDI = 11,       // Output: data to the LED matrix controller
    LE = 12,        // Output: Latch Enable for LED matrix controller
    OE = 13,        // Output: Output Enable for LED matrix controller
    BUZZ = 14,      // Output: Buzzer
    K0 = 15,        // Input:  bottom button
    A0 = 16,        // Output: address line 0 for LED matrix controller
    K1 = 17,        // Input:  middle button 
    A1 = 18,        // Output: address line 1 for LED matrix controller
    A2 = 22,        // Output: address line 2 for LED matrix controller
    AIN = 26        // Input:  ambient light sensor
};