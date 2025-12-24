#include <BleConnectionStatus.h>

#include <BleCompositeHID.h>
#include <XboxGamepadDevice.h>

int ledPin = 5;  // LED connected to digital pin 13
const int AccPin = 32;
const int BreakPin = 33;
const int SteeringPin = 35;  // Potentiometer is connected to GPIO 34 (Analog ADC1_CH6)

const int numberOfPotSamples = 25;     // Number of pot samples to take (to smooth the values)
const int delayBetweenSamples = 4;     // Delay in milliseconds between pot samples
const int delayBetweenHIDReports = 5;

XboxGamepadDevice* gamepad;
BleCompositeHID compositeHID("CompositeHID XInput Controller", "Mystfit", 100);

void OnVibrateEvent(XboxGamepadOutputReportData data) {
  if (data.weakMotorMagnitude > 0 || data.strongMotorMagnitude > 0) {
    digitalWrite(ledPin, LOW);
  } else {
    digitalWrite(ledPin, HIGH);
  }
  Serial.println("Vibration event. Weak motor: " + String(data.weakMotorMagnitude) + " Strong motor: " + String(data.strongMotorMagnitude));
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);  // sets the digital pin as output
  pinMode(21, INPUT_PULLDOWN);
  pinMode(22, INPUT_PULLDOWN);
  pinMode(23, INPUT_PULLDOWN);
  Serial.println("Starting BLE work!");
  // Uncomment one of the following two config types depending on which controller version you want to use
  // The XBox series X controller only works on linux kernels >= 6.5

  XboxOneSControllerDeviceConfiguration* config = new XboxOneSControllerDeviceConfiguration();
  //XboxSeriesXControllerDeviceConfiguration* config = new XboxSeriesXControllerDeviceConfiguration();

  // The composite HID device pretends to be a valid Xbox controller via vendor and product IDs (VID/PID).
  // Platforms like windows/linux need this in order to pick an XInput driver over the generic BLE GATT HID driver.
  BLEHostConfiguration hostConfig = config->getIdealHostConfiguration();
  Serial.println("Using VID source: " + String(hostConfig.getVidSource(), HEX));
  Serial.println("Using VID: " + String(hostConfig.getVid(), HEX));
  Serial.println("Using PID: " + String(hostConfig.getPid(), HEX));
  Serial.println("Using GUID version: " + String(hostConfig.getGuidVersion(), HEX));
  Serial.println("Using serial number: " + String(hostConfig.getSerialNumber()));

  // Set up gamepad
  gamepad = new XboxGamepadDevice(config);

  // Set up vibration event handler
  FunctionSlot<XboxGamepadOutputReportData> vibrationSlot(OnVibrateEvent);
  gamepad->onVibrate.attach(vibrationSlot);

  // Add all child devices to the top-level composite HID device to manage them
  compositeHID.addDevice(gamepad);

  // Start the composite HID device to broadcast HID reports
  Serial.println("Starting composite HID device...");
  compositeHID.begin(hostConfig);
}

void loop() {
  if (compositeHID.isConnected()) {

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
    gamepad->setLeftTrigger(BreakAdjustedValue);
    gamepad->setRightTrigger(AccAdjustedValue);
    gamepad->setLeftThumb(SteeringAdjustedValue, 0);
    gamepad->sendGamepadReport(SteeringAdjustedValue);

    
    // gamepad->setRightThumb(x, y);
    // gamepad->sendGamepadReport();
    // bleGamepad.setX(SteeringAdjustedValue);
    // bleGamepad.setY(AccAdjustedValue);
    // bleGamepad.setZ(BreakAdjustedValue);
    Serial.print("Sent X: ");
    Serial.print(SteeringAdjustedValue);
    Serial.print(" Y: ");
    Serial.print(AccAdjustedValue);
    Serial.print(" Z: ");
    Serial.println(BreakAdjustedValue);

    if (CENTERstate == HIGH && UPstate == LOW && DOWNstate == LOW) {
      Serial.println("CENTER");
      // gamepad->press(button);
      gamepad->release(XBOX_BUTTON_LB);
      gamepad->release(XBOX_BUTTON_RB);
      gamepad->sendGamepadReport();
      // bleGamepad.release(BUTTON_7);
      // bleGamepad.release(BUTTON_8);
      // bleGamepad.releaseStart();
      // bleGamepad.setAxes(AccVal, BreakVal, SteeringVal, 0, 0, 0, 0, 0);
    }
    if (UPstate == HIGH) {
      Serial.println("LB");
      gamepad->press(XBOX_BUTTON_LB);
      gamepad->sendGamepadReport();
      // bleGamepad.press(); //XBOX_BUTTON_LB, XBOX_BUTTON_RB,
      // bleGamepad.pressStart();
      // bleGamepad.setAxes(0, 0, 0, 0, 0, 0, 0, 0);
    }
    if (DOWNstate == HIGH) {
      Serial.println("RB");
      
      gamepad->press(XBOX_BUTTON_RB);
      gamepad->sendGamepadReport();
      // bleGamepad.press(BUTTON_8);
      // bleGamepad.pressStart();
      // bleGamepad.setAxes(0, 0, 0, 0, 0, 0, 0, 0);
    }
    delay(delayBetweenHIDReports);
    // testButtons();
    // testPads();
    // testTriggers();
    // testThumbsticks();
  }
}

void testButtons() {
  // Test each button
  uint16_t buttons[] = {
    XBOX_BUTTON_A,
    XBOX_BUTTON_B,
    XBOX_BUTTON_X,
    XBOX_BUTTON_Y,
    XBOX_BUTTON_LB,
    XBOX_BUTTON_RB,
    XBOX_BUTTON_START,
    XBOX_BUTTON_SELECT,
    //XBOX_BUTTON_HOME,   // Uncomment this to test the hom/guide button. Steam will flip out and enter big picture mode when running this sketch though so be warned!
    XBOX_BUTTON_LS,
    XBOX_BUTTON_RS
  };
  for (uint16_t button : buttons) {
    Serial.println("Pressing button " + String(button));
    gamepad->press(button);
    gamepad->sendGamepadReport();
    delay(500);
    gamepad->release(button);
    gamepad->sendGamepadReport();
    delay(100);
  }

  // The share button is a seperate call since it doesn't live in the same
  // bitflag as the rest of the buttons
  gamepad->pressShare();
  gamepad->sendGamepadReport();
  delay(500);
  gamepad->releaseShare();
  gamepad->sendGamepadReport();
  delay(100);
}

void testPads() {
  XboxDpadFlags directions[] = {
    XboxDpadFlags::NORTH,
    XboxDpadFlags((uint8_t)XboxDpadFlags::NORTH | (uint8_t)XboxDpadFlags::EAST),
    XboxDpadFlags::EAST,
    XboxDpadFlags((uint8_t)XboxDpadFlags::EAST | (uint8_t)XboxDpadFlags::SOUTH),
    XboxDpadFlags::SOUTH,
    XboxDpadFlags((uint8_t)XboxDpadFlags::SOUTH | (uint8_t)XboxDpadFlags::WEST),
    XboxDpadFlags::WEST,
    XboxDpadFlags((uint8_t)XboxDpadFlags::WEST | (uint8_t)XboxDpadFlags::NORTH)
  };

  for (XboxDpadFlags direction : directions) {
    Serial.println("Pressing DPad: " + String(direction));
    gamepad->pressDPadDirectionFlag(direction);
    gamepad->sendGamepadReport();
    delay(500);
    gamepad->releaseDPad();
    gamepad->sendGamepadReport();
    delay(100);
  }
}

void testTriggers() {
  for (int16_t val = XBOX_TRIGGER_MIN; val <= XBOX_TRIGGER_MAX; val++) {
    if (val % 8 == 0)
      Serial.println("Setting trigger value to " + String(val));
    gamepad->setLeftTrigger(val);
    gamepad->setRightTrigger(val);
    gamepad->sendGamepadReport();
    delay(10);
  }
}

void testThumbsticks() {
  int startTime = millis();
  int reportCount = 0;
  while (millis() - startTime < 8000) {
    reportCount++;
    int16_t x = cos((float)millis() / 1000.0f) * XBOX_STICK_MAX;
    int16_t y = sin((float)millis() / 1000.0f) * XBOX_STICK_MAX;

    gamepad->setLeftThumb(x, y);
    gamepad->setRightThumb(x, y);
    gamepad->sendGamepadReport();

    if (reportCount % 8 == 0)
      Serial.println("Setting left thumb to " + String(x) + ", " + String(y));

    delay(10);
  }
}