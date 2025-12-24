/*
 * This example turns the ESP32 into a Bluetooth LE gamepad that presses buttons and moves axis
 *
 * At the moment we are using the default settings, but they can be canged using a BleGamepadConfig instance as parameter for the begin function.
 *
 * Possible buttons are:
 * BUTTON_1 through to BUTTON_16
 * (16 buttons by default. Library can be configured to use up to 128)
 *
 * Possible DPAD/HAT switch position values are:
 * DPAD_CENTERED, DPAD_UP, DPAD_UP_RIGHT, DPAD_RIGHT, DPAD_DOWN_RIGHT, DPAD_DOWN, DPAD_DOWN_LEFT, DPAD_LEFT, DPAD_UP_LEFT
 * (or HAT_CENTERED, HAT_UP etc)
 *
 * bleGamepad.setAxes sets all axes at once. There are a few:
 * (x axis, y axis, z axis, rx axis, ry axis, rz axis, slider 1, slider 2)
 *
 * Library can also be configured to support up to 5 simulation controls
 * (rudder, throttle, accelerator, brake, steering), but they are not enabled by default.
 *
 * Library can also be configured to support different function buttons
 * (start, select, menu, home, back, volume increase, volume decrease, volume mute)
 * start and select are enabled by default
 */

#include <Arduino.h>
#include <BleGamepad.h>

BleGamepad bleGamepad("EZTGamePad", "EZT", 100);  // Set custom device name, manufacturer and initial battery level
BleGamepadConfiguration bleGamepadConfig;

// Use the procedure below to set a custom Bluetooth MAC address
// Compiler adds 0x02 to the last value of board's base MAC address to get the BT MAC address, so take 0x02 away from the value you actually want when setting
// I've noticed the first number is a little picky and if set incorrectly don't work and will default to the board's embedded address
// 0xAA definately works, so use that, or experiment

// uint8_t newMACAddress[] = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF - 0x02 };


const int AccPin = 32;
const int BreakPin = 33;
const int SteeringPin = 35;  // Potentiometer is connected to GPIO 34 (Analog ADC1_CH6)

const int numberOfPotSamples = 10;     // Number of pot samples to take (to smooth the values)
const int delayBetweenSamples = 4;     // Delay in milliseconds between pot samples
const int delayBetweenHIDReports = 5;  // Additional delay in milliseconds between HID reports

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");
  pinMode(21, INPUT_PULLDOWN);
  pinMode(22, INPUT_PULLDOWN);
  pinMode(23, INPUT_PULLDOWN);

  // bleGamepadConfig.setAutoReport(false);
  // bleGamepadConfig.setControllerType(CONTROLLER_TYPE_GAMEPAD);  // CONTROLLER_TYPE_JOYSTICK, CONTROLLER_TYPE_GAMEPAD (DEFAULT), CONTROLLER_TYPE_MULTI_AXIS
  bleGamepadConfig.setVid(0x045e); //045E
  bleGamepadConfig.setPid(0xd202); //02D2

  // bleGamepadConfig.setModelNumber("1.0");
  // bleGamepadConfig.setSoftwareRevision("Software Rev 1");
  // bleGamepadConfig.setSerialNumber("9876543210");
  // bleGamepadConfig.setFirmwareRevision("2.0");
  // bleGamepadConfig.setHardwareRevision("1.7");

  bleGamepad.begin(&bleGamepadConfig);  // Begin gamepad with configuration options
  // The default bleGamepad.begin() above enables 16 buttons, all axes, one hat, and no simulation controls or special buttons
  // esp_base_mac_addr_set(&newMACAddress[0]);  // Set new MAC address
}

void loop() {

  if (bleGamepad.isConnected()) {
    bool UPstate = digitalRead(21);
    bool DOWNstate = digitalRead(22);
    bool CENTERstate = digitalRead(23);

    int SteeringValues[numberOfPotSamples];  // Array to store pot readings
    int AccValues[numberOfPotSamples];
    int BreakValues[numberOfPotSamples];

    int SteeringVal = 0;  // Variable to store calculated pot reading average
    int AccVal = 0;
    int BreakVal = 0;

    // Populate readings
    for (int i = 0; i < numberOfPotSamples; i++) {
      SteeringValues[i] = analogRead(SteeringPin);
      AccValues[i] = analogRead(AccPin);
      BreakValues[i] = analogRead(BreakPin);

      SteeringVal += SteeringValues[i];
      AccVal += AccValues[i];
      BreakVal += BreakValues[i];
      delay(delayBetweenSamples);
    }

    // Calculate the average
    SteeringVal = SteeringVal / numberOfPotSamples;
    AccVal = AccVal / numberOfPotSamples;
    BreakVal = BreakVal / numberOfPotSamples;

    // Map analog reading from 0 ~ 4095 to 32737 ~ 0 for use as an axis reading
    int SteeringAdjustedValue = map(SteeringVal, 0, 4095, 32737, 0);
    int AccAdjustedValue = map(AccVal, 0, 4095, 32737, 0);
    int BreakAdjustedValue = map(BreakVal, 0, 4095, 32737, 0);
    // Update X axis and auto-send report
    bleGamepad.setX(SteeringAdjustedValue);
    bleGamepad.setY(AccAdjustedValue);
    bleGamepad.setZ(BreakAdjustedValue);
    Serial.print("Sent X: ");
    Serial.print(SteeringAdjustedValue);
    Serial.print(" Y: ");
    Serial.print(AccAdjustedValue);
    Serial.print(" Z: ");
    Serial.println(BreakAdjustedValue);

    if (CENTERstate == HIGH && UPstate == LOW && DOWNstate == LOW) {
      Serial.println("CENTER");
      bleGamepad.release(BUTTON_7);
      bleGamepad.release(BUTTON_8);
      bleGamepad.releaseStart();
      // bleGamepad.setAxes(AccVal, BreakVal, SteeringVal, 0, 0, 0, 0, 0);
    }
    if (UPstate == HIGH) {
      Serial.println("Break");
      bleGamepad.press(BUTTON_7);
      bleGamepad.pressStart();
      // bleGamepad.setAxes(0, 0, 0, 0, 0, 0, 0, 0);
    }
    if (DOWNstate == HIGH) {
      Serial.println("Acc");
      bleGamepad.press(BUTTON_8);
      bleGamepad.pressStart();
      // bleGamepad.setAxes(0, 0, 0, 0, 0, 0, 0, 0);
    }
    delay(delayBetweenHIDReports);
    // for (int i = 0; i <= 32767; i ++)
    // {
    //   bleGamepad.setAxes(i, (32767 - i), 0, 0, 0, 0, 0, 0);
    //   delay(100);
    // }
  }
}
