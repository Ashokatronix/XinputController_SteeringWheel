#include <BleConnectionStatus.h>
#include <BleCompositeHID.h>
#include <XboxGamepadDevice.h>

#define DEBUG 1  // Set to 1 to enable debug, 0 to disable

int ledPin = 5;  // LED connected to digital pin 13
const int AccPin = 32;
const int BreakPin = 33;
const int SteeringPin = 35;  // Potentiometer is connected to GPIO 34 (Analog ADC1_CH6)

const int numberOfPotSamples = 20;  // Number of pot samples to take (to smooth the values)
const int delayBetweenSamples = 3;  // Delay in milliseconds between pot samples
const int delayBetweenHIDReports = 3;

int SteeringAdjustedValue = 0;
int AccAdjustedValue = 0;
int BreakAdjustedValue = 0;

// Initialize previous values
int prev_break_value = 0;
int prev_acc_value = 0;
int prev_steering_value = 0;

float linearValueAcc, linearValueBrake, linearValueSteering;
// More precise lookup tables for nonlinear to linear conversion
const int lookupTableAcc[21] = { 0, 120, 360, 840, 1560, 2520, 3720, 5160, 6840, 8760, 10920, 13320, 15960, 18840, 21960, 25320, 28920, 32760, 36840, 41160, 45720 };
const int lookupTableBrake[21] = { 0, 120, 360, 840, 1560, 2520, 3720, 5160, 6840, 8760, 10920, 13320, 15960, 18840, 21960, 25320, 28920, 32760, 36840, 41160, 45720 };
const int lookupTableSteering[21] = { 0, 120, 360, 840, 1560, 2520, 3720, 5160, 6840, 8760, 10920, 13320, 15960, 18840, 21960, 25320, 28920, 32760, 36840, 41160, 45720 };

// Define a threshold for significant change
int significant_change_threshold = 2;  // You can adjust this value as needed

String inputString = "";      // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

XboxGamepadDevice* gamepad;
BleCompositeHID compositeHID("BLE Driving Controller", "Mystfit", 100);
// BleCompositeHID compositeHID("CompositeHID XInput Controller", "Mystfit", 100);
//BleCompositeHID compositeHID("BLE Driving Controller", "lemmingDev", 100);
//GamepadDevice* gamepad;

void OnVibrateEvent(XboxGamepadOutputReportData data) {
  if (data.weakMotorMagnitude > 0 || data.strongMotorMagnitude > 0) {
    digitalWrite(ledPin, LOW);
  } else {
    digitalWrite(ledPin, HIGH);
  }
#if DEBUG
  Serial.println("Vibration event. Weak motor: " + String(data.weakMotorMagnitude) + " Strong motor: " + String(data.strongMotorMagnitude));
#endif
}

void setup() {
  // initialize serial:
  Serial.begin(115200);
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);

  pinMode(ledPin, OUTPUT);  // sets the digital pin as output
  pinMode(21, INPUT_PULLDOWN);
  pinMode(22, INPUT_PULLDOWN);
  pinMode(23, INPUT_PULLDOWN);
#if DEBUG
  Serial.println("Starting BLE work!");
#endif
  /*
  // Set accelerator and brake to min
  gamepad->setAccelerator(-32767);
  gamepad->setBrake(-32767);

  // Set steering to center
  gamepad->setSteering(0);
  */

  // Uncomment one of the following two config types depending on which controller version you want to use
  // The XBox series X controller only works on linux kernels >= 6.5

  XboxOneSControllerDeviceConfiguration* config = new XboxOneSControllerDeviceConfiguration();
  //XboxSeriesXControllerDeviceConfiguration* config = new XboxSeriesXControllerDeviceConfiguration();

  // The composite HID device pretends to be a valid Xbox controller via vendor and product IDs (VID/PID).
  // Platforms like windows/linux need this in order to pick an XInput driver over the generic BLE GATT HID driver.
  BLEHostConfiguration hostConfig = config->getIdealHostConfiguration();
#if DEBUG
  Serial.println("Using VID source: " + String(hostConfig.getVidSource(), HEX));
  Serial.println("Using VID: " + String(hostConfig.getVid(), HEX));
  Serial.println("Using PID: " + String(hostConfig.getPid(), HEX));
  Serial.println("Using GUID version: " + String(hostConfig.getGuidVersion(), HEX));
  Serial.println("Using serial number: " + String(hostConfig.getSerialNumber()));
#endif
  // Set up gamepad
  gamepad = new XboxGamepadDevice(config);

  // Set up vibration event handler
  FunctionSlot<XboxGamepadOutputReportData> vibrationSlot(OnVibrateEvent);
  gamepad->onVibrate.attach(vibrationSlot);

  // Add all child devices to the top-level composite HID device to manage them
  compositeHID.addDevice(gamepad);

// Start the composite HID device to broadcast HID reports
#if DEBUG
  Serial.println("Starting composite HID device...");
#endif
  compositeHID.begin(hostConfig);
}

void loop() {
  if (compositeHID.isConnected()) {
    // print the string when a newline arrives:
    if (stringComplete) {
#if DEBUG
      Serial.println(inputString);
      Serial.print("Lastchar ");
      Serial.println(inputString.charAt(0));
#endif

      switch (inputString.charAt(0)) {
        case 'A':
          gamepad->press(XBOX_BUTTON_A);
          gamepad->sendGamepadReport();
#if DEBUG
          Serial.println("_A");
#endif
          break;
        case 'B':
          gamepad->press(XBOX_BUTTON_B);
          gamepad->sendGamepadReport();
#if DEBUG
          Serial.println("_B");
#endif
          break;
        case 'X':
          gamepad->press(XBOX_BUTTON_X);
          gamepad->sendGamepadReport();
#if DEBUG
          Serial.println("_X");
#endif
          break;
        case 'Y':
          gamepad->press(XBOX_BUTTON_Y);
          gamepad->sendGamepadReport();
#if DEBUG
          Serial.println("_Y");
#endif
          break;
        case '>':
          // gamepad->press(15);
          gamepad->pressDPadDirectionFlag(XboxDpadFlags::EAST);
          gamepad->sendGamepadReport();
#if DEBUG
          Serial.println("->");
#endif
          break;
        case '<':
          // gamepad->press(14);
          gamepad->pressDPadDirectionFlag(XboxDpadFlags::WEST);
          gamepad->sendGamepadReport();
#if DEBUG
          Serial.println("<-");
#endif
          break;
        case '^':
          // gamepad->press(12);
          gamepad->pressDPadDirectionFlag(XboxDpadFlags::NORTH);
          gamepad->sendGamepadReport();
#if DEBUG
          Serial.println("_^");
#endif
          break;
        case 'V':
          // gamepad->press(13);
          gamepad->pressDPadDirectionFlag(XboxDpadFlags::SOUTH);
          gamepad->sendGamepadReport();
#if DEBUG
          Serial.println("_V");
#endif
          break;
        case 'T':
          gamepad->press(XBOX_BUTTON_START);
          gamepad->sendGamepadReport();
#if DEBUG
          Serial.println("_START");
#endif
          break;
        case 'E':
          gamepad->press(XBOX_BUTTON_SELECT);
          gamepad->sendGamepadReport();
#if DEBUG
          Serial.println("_SELECT");
#endif
          break;
        case 'L':
          gamepad->press(XBOX_BUTTON_LB);
          gamepad->sendGamepadReport();
#if DEBUG
          Serial.println("_LB");
#endif
          break;
        case 'R':
          gamepad->press(XBOX_BUTTON_RB);
          gamepad->sendGamepadReport();
#if DEBUG
          Serial.println("_RB");
#endif
          break;
        default:
          {
            gamepad->release(XBOX_BUTTON_A);
            gamepad->release(XBOX_BUTTON_B);
            gamepad->release(XBOX_BUTTON_X);
            gamepad->release(XBOX_BUTTON_Y);
            gamepad->releaseDPad();
            gamepad->sendGamepadReport();
#if DEBUG
            Serial.println("Default");
#endif
          }
          delay(1);
      }
      delay(10);
      // clear the string:
      inputString = "";
      stringComplete = false;
      gamepad->release(XBOX_BUTTON_A);
      gamepad->release(XBOX_BUTTON_B);
      gamepad->release(XBOX_BUTTON_X);
      gamepad->release(XBOX_BUTTON_Y);
      gamepad->release(XBOX_BUTTON_START);
      gamepad->release(XBOX_BUTTON_SELECT);
      gamepad->release(XBOX_BUTTON_LB);
      gamepad->release(XBOX_BUTTON_RB);
      gamepad->releaseDPad();
      // gamepad->release(12);
      // gamepad->release(13);
      // gamepad->release(14);
      // gamepad->release(15);
      gamepad->sendGamepadReport();
      delay(5);
    }

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

    linearValueAcc = interpolate(AccVal, lookupTableAcc);
    linearValueBrake = interpolate(BreakVal, lookupTableBrake);
    linearValueSteering = interpolate(SteeringVal, lookupTableSteering);

    // Check for significant change
    int break_change = abs(linearValueBrake - prev_break_value);
    int acc_change = abs(linearValueAcc - prev_acc_value);
    int steering_change = abs(linearValueSteering - prev_steering_value);
    /*
    if (break_change > significant_change_threshold) {
      // Map analog reading from 0 ~ 4095 to 32737 ~ 0 for use as an axis reading
      // BreakAdjustedValue = map(BreakVal, 0, 4095, XBOX_TRIGGER_MAX, XBOX_TRIGGER_MIN);
      BreakAdjustedValue = map(BreakVal, 0, 4095, -32737, 32736);
     
      // Update X axis and auto-send report
      gamepad->setRightThumb(,BreakAdjustedValue);
      gamepad->sendGamepadReport();
      delay(10);
      // Update previous values
      prev_break_value = BreakVal;
      Serial.print("Sent Break: ");
      Serial.println(BreakAdjustedValue);
      /*Serial.println("Move brake from min to max");
        for (int i = -32767; i < 32767; i += 256)*/
    // }

    if (acc_change > significant_change_threshold || break_change > significant_change_threshold) {
      // Map analog reading from 0 ~ 4095 to 32737 ~ 0 for use as an axis reading
      // AccAdjustedValue = map(AccVal, 0, 4095, XBOX_TRIGGER_MAX, XBOX_TRIGGER_MIN);
      AccAdjustedValue = map(linearValueAcc, 0, 4095, -32737, 32736);
      BreakAdjustedValue = map(linearValueBrake, 0, 4095, -32737, 32736);
      gamepad->setRightThumb(AccAdjustedValue, BreakAdjustedValue);
      gamepad->sendGamepadReport();
      delay(5);
      // Update previous values
      prev_acc_value = linearValueAcc;
#if DEBUG
      Serial.print("Acc: ");
      Serial.print(AccAdjustedValue);
#endif
      prev_break_value = linearValueBrake;
#if DEBUG
      Serial.print(" Break: ");
      Serial.println(BreakAdjustedValue);
#endif
      /*Serial.println("Move accelerator from min to max");
        // for(int i = -32767 ; i < 32767 ; i += 256) */
    }

    if (steering_change > significant_change_threshold) {
      // Map analog reading from 0 ~ 4095 to 32737 ~ 0 for use as an axis reading
      SteeringAdjustedValue = map(linearValueSteering, 0, 4095, -32737, 32736);
      gamepad->setLeftThumb(SteeringAdjustedValue, 0);
      gamepad->sendGamepadReport();
      delay(5);
      // Update previous values
      prev_steering_value = linearValueSteering;
#if DEBUG
      Serial.print(" Steering: ");
      Serial.println(SteeringAdjustedValue);
#endif
      /*Serial.println("Move steering from min to max");
        for (int i = -32767; i < 32767; i += 256)*/
    }

    // Serial.print("Sent X: ");
    // Serial.print(SteeringAdjustedValue);
    // Serial.print(" Y: ");
    // Serial.print(AccAdjustedValue);
    // Serial.print(" Z: ");
    // Serial.println(BreakAdjustedValue);

    if (CENTERstate == HIGH && UPstate == LOW && DOWNstate == LOW) {
      // Serial.println("CENTER");
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
#if DEBUG
      Serial.println("LB");
#endif
      gamepad->press(XBOX_BUTTON_LB);
      gamepad->sendGamepadReport();
      // bleGamepad.press(); //XBOX_BUTTON_LB, XBOX_BUTTON_RB,
      // bleGamepad.pressStart();
      // bleGamepad.setAxes(0, 0, 0, 0, 0, 0, 0, 0);
    }
    if (DOWNstate == HIGH) {
#if DEBUG
      Serial.println("RB");
#endif
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

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}

float interpolate(int raw, const int* lookupTable) {
  int index = raw / 204;  // Assuming 12-bit ADC and 20 segments
  float fraction = (raw % 204) / 204.0;
  return lookupTable[index] + fraction * (lookupTable[index + 1] - lookupTable[index]);
}