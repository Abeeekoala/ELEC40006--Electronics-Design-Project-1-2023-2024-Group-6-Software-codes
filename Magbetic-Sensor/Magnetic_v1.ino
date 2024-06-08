const int hallSensorPin = A0;  // Analog input pin that the Hall sensor is connected to
const float biasVoltage = 2.5; // Bias voltage around which the sensor is centered
const int adcResolution = 1024; // ADC resolution for Arduino Uno, Mega, etc. (10-bit resolution)
const float referenceVoltage = 5.0; // Reference voltage for the ADC (typically 5V for most Arduinos)

void setup() {
  Serial.begin(9600); 
}

void loop() {
  // Read the value from the Hall sensor
  int sensorValue = analogRead(hallSensorPin);

  // Convert the analog reading (which goes from 0 - adcResolution) to a voltage (0 - referenceVoltage)
  float sensorVoltage = sensorValue * (referenceVoltage / adcResolution);

  // Calculate the deviation from the bias voltage
  float deviation = sensorVoltage - biasVoltage;

  // Print out the sensor voltage and deviation
  Serial.print("Sensor Voltage: ");
  Serial.print(sensorVoltage);
  Serial.print(" V, Deviation: ");
  Serial.print(deviation);
  Serial.println(" V");
   // Check the deviation and print the corresponding pole detection
  if (deviation > 0.15) {
    Serial.println("South pole detected");
  } else if (deviation < -0.15) {
    Serial.println("North pole detected");
  }
  
}

