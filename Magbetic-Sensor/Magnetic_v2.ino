const int hallSensorPin = A0;  // Analog input pin that the Hall sensor is connected to
const int adcResolution = 1024; // ADC resolution 
const float referenceVoltage = 5.0; // Reference voltage for the ADC 
float biasVoltage = 0;


void setup() {
  Serial.begin(9600);  // Initialize serial communication at 9600 bits per second
   biasVoltage = analogRead(hallSensorPin)* (referenceVoltage / adcResolution); // Bias voltage around which the sensor is centered
}

void loop() {
  // Read the value from the Hall sensor
  int sensorValue = analogRead(hallSensorPin);

  // Convert the analog reading 
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
  if (deviation > 0.4) {
    Serial.println("South pole detected");
  } else if (deviation < -0.4) {
    Serial.println("North pole detected");
  }
}
