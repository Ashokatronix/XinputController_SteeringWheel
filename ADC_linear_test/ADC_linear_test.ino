#define DEBUG 1 // Set to 1 to enable debug, 0 to disable

const int potPinAcc = 32; // Pin connected to the Acceleration potentiometer
const int potPinBrake = 33; // Pin connected to the Brake potentiometer
const int potPinSteering = 35; // Pin connected to the Steering potentiometer

int rawValueAcc, rawValueBrake, rawValueSteering;
float linearValueAcc, linearValueBrake, linearValueSteering;
int mappedValueAcc, mappedValueBrake, mappedValueSteering;

// Common lookup table for nonlinear to linear conversion
const int lookupTable[10] = {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4095};

void setup() {
  Serial.begin(115200);
  #if DEBUG
    Serial.println("Debugging is ON");
  #endif
}

void loop() {
  rawValueAcc = analogRead(potPinAcc);
  rawValueBrake = analogRead(potPinBrake);
  rawValueSteering = analogRead(potPinSteering);

  linearValueAcc = interpolate(rawValueAcc, lookupTable);
  linearValueBrake = interpolate(rawValueBrake, lookupTable);
  linearValueSteering = interpolate(rawValueSteering, lookupTable);

  mappedValueAcc = map(rawValueAcc, 200, 550, -32737, 32736);
  mappedValueBrake = map(rawValueBrake, 0, 550, -32737, 32736);
  mappedValueSteering = map(rawValueSteering, 200, 1024, -32737, 32736);

  #if DEBUG
    Serial.print("Raw Acc: ");
    Serial.print(rawValueAcc);


    Serial.print(" Raw Brake: ");
    Serial.print(rawValueBrake);


    Serial.print(" Raw Steering: ");
    Serial.println(rawValueSteering);

  #endif

  delay(100);
}

float interpolate(int raw, const int* lookupTable) {
  int index = raw / 455; // Assuming 12-bit ADC and 20 segments
  if (index >= 9) index = 8; // Ensure index is within bounds
  float fraction = (raw % 455) / 455.0;
  return lookupTable[index] + fraction * (lookupTable[index + 1] - lookupTable[index]);
}
