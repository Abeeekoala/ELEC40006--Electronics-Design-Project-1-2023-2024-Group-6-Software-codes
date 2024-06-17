#define USE_WIFI_NINA         false
#define USE_WIFI101           true


#include <WiFiWebServer.h>
#include <string.h>
#include <SPI.h>
#include <Ethernet.h>
#include <avr/pgmspace.h>

// Replace with your network credentials
const char ssid[] = "XXXXX";
const char pass[] = "XXXXX";

const int groupNumber = 0; // Set your group number to make the IP address constant - only do this on the EEERover network

// Define motor control pins
const int DIR1_PIN = 4; // Direction pin for left motor
const int PWM1_PIN = 3; // PWM pin for left motor
const int DIR2_PIN = 8; // Direction pin for right motor
const int PWM2_PIN = 9; // PWM pin for right motor
const int signalPinRadio = 1;
const int signalPinIR = 2;
const int hallSensorPin = A0;

WiFiWebServer server(80);
const unsigned long TIMEOUT_MS = 200;

String nameDetected = "N/A";
float IR_frequency = 0;
float radio_frequency = 0;
String magnetic = "Unknown";
String species = "Unknown";

bool messageComplete = false;             // Indicates message completion

static char buffer[5];                    // Buffer to store 4 characters + null terminator
static int bufferIndex = 0;               // Index to track the buffer position

unsigned long lastTimeIR = 0;
unsigned long currentTimeIR = 0;
unsigned long intervalIR = 0;
unsigned long lastTimeRA = 0;
unsigned long currentTimeRA = 0;
unsigned long intervalRA = 0;


bool lastStateIR = LOW;
bool currentStateIR = LOW;
bool lastStateRadio = LOW;
bool currentStateRadio = LOW;

float biasVoltage = 0;                    // Bias voltage around which the sensor is centered
const int adcResolution = 1024;           // ADC (analog-digital-converter) resolution
const float referenceVoltage = 3.3;       // Reference voltage for the ADC

// const char webpage[] PROGMEM = R"rawliteral(
// <html>
// <head>
// <style>
//  table, th, td {border: 1px solid black;}
//  .btn {background-color: inherit; padding: 14px 28px; font-size: 16px;}
//  .btn:hover {background: #eee;}
// </style>
// </head>
// <body>
// <p>Detected Name: <span id="name">N/A</span></p>
// <table id="lizardTable">
//  <tr>
//    <th>Name of Lizard</th>
//    <th>Infrared</th>
//    <th>Radio</th>
//    <th>Magnetic</th>
//    <th>Species</th>
//  </tr>
// </table>
// <button class="btn" onclick="requestData()">Detect New Lizard</button>
// </body>
// <script>
// </script>
// </html>
// )rawliteral"; // old document object model

//Allow data to send to different origins look up CORS for more details
void applyCORSHeaders() { 
  server.sendHeader("Access-Control-Allow-Origin", "*", true);                      // allows outside access to server
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS", true);   // allows the listed methods from outside sources
  server.sendHeader("Access-Control-Allow-Headers", "*", true);
}

// Return the web page
void handleRoot() {
  Serial.println("Root page requested");
  // server.send(200, F("text/html"), webpage); old obsolete way of sending document object model
}

void moveForward() {
  applyCORSHeaders();
  server.send(200, F("text/plain"), F("Forward"));
  Serial.println("Move Forward");
  stopMotors();                                            // Ensure motors are stopped before changing direction
  delay(50);                                               // Brief delay to ensure motors stop
  digitalWrite(DIR1_PIN, HIGH);                            // writes to the digital pin that controls the motor that it should be HIGH (going forward)
  analogWrite(PWM1_PIN, 255);                              // Writes to the PWM pin that the speed it should go at (255 is max)
  digitalWrite(DIR2_PIN, HIGH);
  analogWrite(PWM2_PIN, 255);
  server.send(200, F("text/plain"), F("Forward"));
}
// Backward movement control for rover
void moveBackward() {
  applyCORSHeaders();
  server.send(200, F("text/plain"), F("Backward"));
  Serial.println("Move Backward");
  stopMotors(); // Ensure motors are stopped before changing direction
  delay(50); // Brief delay to ensure motors stop
  digitalWrite(DIR1_PIN, LOW);
  analogWrite(PWM1_PIN, 255); // Full speed
  digitalWrite(DIR2_PIN, LOW);
  analogWrite(PWM2_PIN, 255); // Full speed

}
// Turn left movement control for rover
void turnLeft() {
  applyCORSHeaders();
  server.send(200, F("text/plain"), F("Left"));
  Serial.println("Turn Left");
  stopMotors(); // Ensure motors are stopped before changing direction
  delay(100); // Brief delay to ensure motors stop
  digitalWrite(DIR1_PIN, LOW);
  analogWrite(PWM1_PIN, 100); 
  digitalWrite(DIR2_PIN, HIGH);
  analogWrite(PWM2_PIN, 100); 

}
// Turn right movement control for rover
void turnRight() {
  applyCORSHeaders();
  server.send(200, F("text/plain"), F("Right"));
  Serial.println("Turn Right");
  stopMotors(); // Ensure motors are stopped before changing direction
  delay(100); // Brief delay to ensure motors stop
  digitalWrite(DIR1_PIN, HIGH);
  analogWrite(PWM1_PIN, 100); 
  digitalWrite(DIR2_PIN, LOW);
  analogWrite(PWM2_PIN, 100); 
}


void turnLeftSlow() {
  applyCORSHeaders();
  server.send(200, F("text/plain"), F("Left"));
  Serial.println("Turn Left");
  stopMotors(); // Ensure motors are stopped before changing direction
  delay(100); // Brief delay to ensure motors stop
  digitalWrite(DIR1_PIN, LOW);
  analogWrite(PWM1_PIN, 40); 
  digitalWrite(DIR2_PIN, HIGH);
  analogWrite(PWM2_PIN, 40); 

}
// Turn right movement control for rover
void turnRightSlow() {
  applyCORSHeaders();
  server.send(200, F("text/plain"), F("Right"));
  Serial.println("Turn Right");
  stopMotors(); // Ensure motors are stopped before changing direction
  delay(100); // Brief delay to ensure motors stop
  digitalWrite(DIR1_PIN, HIGH);
  analogWrite(PWM1_PIN, 40); 
  digitalWrite(DIR2_PIN, LOW);
  analogWrite(PWM2_PIN, 40); 
}

void turnForwardLeft() {
  applyCORSHeaders();
  server.send(200, F("text/plain"), F("Forward Left"));
  Serial.println("Turn Left Slow");
  stopMotors(); // Ensure motors are stopped before changing direction
  delay(100); 
  digitalWrite(DIR1_PIN, HIGH);
  analogWrite(PWM1_PIN, 155); 
  digitalWrite(DIR2_PIN, HIGH);
  analogWrite(PWM2_PIN, 255); 

}

void turnForwardRight() {
  applyCORSHeaders();
  server.send(200, F("text/plain"), F("Forward Right"));
  Serial.println("Turn Right Slow");
  stopMotors(); // Ensure motors are stopped before changing direction
  delay(100); // Brief delay to ensure motors stop
  digitalWrite(DIR1_PIN, HIGH);
  analogWrite(PWM1_PIN, 255); // slower speed
  digitalWrite(DIR2_PIN, HIGH);
  analogWrite(PWM2_PIN, 155); // slower speed
}
// Stop motor movement control for rover
void stopMotors() {
  applyCORSHeaders();
  server.send(200, F("text/plain"), F("Stop"));
  Serial.println("Stop Motors");
  analogWrite(PWM1_PIN, 0); // Stop left motor
  analogWrite(PWM2_PIN, 0); // Stop right motor
}

void detectName() {
  unsigned long startTime = millis();  // Record the start time
  Serial.println("Detecting Name");

  while (true) {
    if (millis() - startTime >= TIMEOUT_MS) {
      applyCORSHeaders();
      Serial.println("Detection timed out.");
      nameDetected = "N/A";
      server.send(200, "text/plain", nameDetected);
      return;
    }

    // Check if there are any bytes available to read from Serial1
    if (Serial1.available()) {
      // Read the incoming byte from the UART port
      char incomingByte = Serial1.read();

      // Print the received byte
      Serial.print("Received byte: ");
      Serial.println(incomingByte, BIN);  // Print as binary

      // Filter out noise
      if (incomingByte < 32 || incomingByte > 126) {
        Serial.println("Noise detected, ignoring byte.");
        continue;
      }

      Serial.print("Character: ");
      Serial.println(incomingByte);

      // Store the byte in the buffer if it's part of the 4-character name
      if (bufferIndex < 4) {
        buffer[bufferIndex++] = incomingByte;
      }

      // Check if the buffer contains a 4-character name starting with '#'
      if (bufferIndex == 4) {
        buffer[bufferIndex] = '\0';  // Null-terminate the buffer
        if (buffer[0] == '#') {
          Serial.print("Detected name: ");
          Serial.println(buffer);
          nameDetected = String(buffer);  // Save the detected name
          break;
        } else {
          // Shift buffer content to the left by one position and reset bufferIndex
          for (int i = 0; i < 3; i++) {
            buffer[i] = buffer[i + 1];
          }
          bufferIndex = 3;
        }
      }
    }
  }

}

void resetName(){
  Serial.println("Resetting Detection");
  nameDetected = "N/A";
  messageComplete = false;
  bufferIndex = 0;
  memset(buffer, 0, sizeof(buffer)); // Clear the buffer
  while (Serial1.available()) {
    Serial1.read();  // Flush any remaining data in the buffer
  }
  Serial.println("Serial1 buffer flushed.");
}

void detectMagnetic(){
  int sensorValue = analogRead(hallSensorPin);                            // Read the value from the Hall sensor
  float sensorVoltage = sensorValue * (referenceVoltage / adcResolution); // Convert the analog reading
  float deviation = sensorVoltage - biasVoltage;   
  Serial.println(F("Hall Sensor Bias voltage"));
  Serial.println(biasVoltage);                       // Calculate the deviation from the bias voltage
  // Print out the sensor voltage and deviation
  Serial.print("Sensor Voltage: ");             
  Serial.print(sensorVoltage);
  Serial.print(" V, Deviation: ");
  Serial.print(deviation);
  Serial.println(" V");
  // Check the deviation and print the corresponding pole detection
  if (deviation > 0.2) {
    Serial.println("South pole detected");
    magnetic = "S";
  } else if (deviation < -0.2) {
    Serial.println("North pole detected");
    magnetic = "N";
  }
}

void resetMagnetic(){
  analogRead(hallSensorPin);
  magnetic = "Unknown";
}

void detectRadio(){
  unsigned long startTime = millis();
  Serial.println("Detecting Radio");
  while ( (millis() - startTime < TIMEOUT_MS) && radio_frequency < 10) {
      currentStateRadio = digitalRead(signalPinRadio);
      if (currentStateRadio != lastStateRadio) { // Check if state has changed
        if (currentStateRadio == HIGH) { // Check for a rising edge
          currentTimeRA = micros(); // Get the current time in microseconds
          intervalRA = currentTimeRA - lastTimeRA; // Calculate the time interval
          lastTimeRA = currentTimeRA; // Update the last time
          radio_frequency = 1000000.0 / intervalRA; // Calculate the frequency in Hz
          Serial.print("Radio Frequency: ");
          Serial.print(radio_frequency);
          Serial.println(" Hz");
       }
      lastStateRadio = currentStateRadio;
    }
  }
}

void resetRadio(){
  radio_frequency = 0;
  intervalRA = 0;
  lastTimeRA = 0;
  currentTimeRA = 0;
}

void detectIR(){
  unsigned long startTime = millis();  // Record the start time
  Serial.println("Detecting IR");
  while ( (millis() - startTime < TIMEOUT_MS) && IR_frequency < 10) {
      currentStateIR = digitalRead(signalPinIR);
      if (currentStateIR != lastStateIR) { // Check if state has changed
        if (currentStateIR == HIGH) { // Check for a rising edge
          currentTimeIR = micros(); // Get the current time in microseconds
          intervalIR = currentTimeIR - lastTimeIR; // Calculate the time interval
          lastTimeIR = currentTimeIR; // Update the last time
          IR_frequency = 1000000.0 / intervalIR; // Calculate the frequency in Hz
          Serial.print("IR Frequency: ");
          Serial.print(IR_frequency);
          Serial.println(" Hz");
       }
      lastStateIR = currentStateIR;
    }
  }
}

void resetIR(){
  IR_frequency = 0;
  intervalIR = 0;
  lastTimeIR = 0;
  currentTimeIR = 0;
}


void Reset_all() {
  resetName();
  resetMagnetic();
  resetRadio();
  resetIR();
}

void send_detections(){
  // The four posibilities of species each tested with unique if conditions
  if (IR_frequency > 10 && magnetic == "N")
    {species = "Abronia";}
  else if (radio_frequency > 10 && magnetic == "N")
    {species = "Elgaria";}
  else if (IR_frequency > 10 && magnetic == "S")
    {species = "Dixonius";}
  else if (radio_frequency > 10 && magnetic == "S")
    {species = "Cophotis";}

  applyCORSHeaders();
  String response = "{\"name\": \"" + nameDetected + "\", \"infrared\": \"" + String(IR_frequency) + "\", \"radio\": \"" + String(radio_frequency) + "\", \"magnetic\": \"" + magnetic + "\", \"species\": \"" + species + "\"}";
  Serial.println(response);
  server.send(200, "application/json", response);
}

void scan_all() {
  detectName();
  detectMagnetic();
  unsigned long startTime = millis();
  while ((millis() - startTime < TIMEOUT_MS) && IR_frequency < 10 && radio_frequency < 10) {
    detectIR();
    detectRadio();
  }
  send_detections();
  Reset_all();
 }

void detectName_sole(){
  detectName();
  applyCORSHeaders();
  server.send(200, "text/plain", nameDetected);
  resetName();
}

void detectMagnetic_sole(){
  detectMagnetic();
  applyCORSHeaders();
  server.send(200, "text/plain", magnetic);
  resetMagnetic();
}

void detectIR_sole(){
  detectIR();
  applyCORSHeaders();
  server.send(200, "text/plain", String(IR_frequency));
  resetIR();
}

void detectRadio_sole(){
  detectRadio();
  applyCORSHeaders();
  server.send(200, "text/plain", String(radio_frequency));
  resetRadio();
}

void handleNotFound() {
  Serial.println("404: Not Found");
  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += server.uri();
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET) ? F("GET") : F("POST");
  message += F("\nArguments: ");
  message += server.args();
  message += F("\n");
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, F("text/plain"), message);
}

void setup() {
  // Set motor control pins as outputs
  pinMode(DIR1_PIN, OUTPUT);
  pinMode(PWM1_PIN, OUTPUT); // Although used with analogWrite, it's still set as OUTPUT
  pinMode(DIR2_PIN, OUTPUT);
  pinMode(PWM2_PIN, OUTPUT); 

  Serial.begin(9600);
  Serial1.begin(600, SERIAL_8N1);  // 600 baud, 8 data bits, no parity, 1 stop bit

  while (!Serial && millis() < 10000);
  pinMode(signalPinIR, INPUT);
  pinMode(signalPinRadio, INPUT);
  biasVoltage = analogRead(hallSensorPin)*(referenceVoltage/ adcResolution);

  Serial.println(F("Hall Sensor Bias voltage"));
  Serial.println(biasVoltage);
  Serial.println(F("\nStarting Web Server"));
  // Check WiFi shield is present
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println(F("WiFi shield not present"));
    while (true);
  }

  // Attempt to connect to WiFi network
  Serial.print(F("Connecting to WPA SSID: "));
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }

  Serial.println();
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);


  // Initialize the server and register route handlers
  server.on("/", HTTP_GET, handleRoot);
  server.on("/forward", HTTP_GET, moveForward);
  server.on("/backward", HTTP_GET, moveBackward);
  server.on("/left", HTTP_GET, turnLeft);
  server.on("/right", HTTP_GET, turnRight);
  server.on("/leftslow", HTTP_GET, turnLeftSlow);
  server.on("/rightslow", HTTP_GET, turnRightSlow);
  server.on(F("/forwardLeft"), turnForwardLeft);
  server.on(F("/forwardRight"), turnForwardRight);
  server.on("/stop", HTTP_GET, stopMotors);
  server.on("/scan", HTTP_GET, scan_all);  // Handler for detecting and sending lizard data
  server.on("/detectName", HTTP_GET, detectName_sole);
  server.on("/detectMagnetic", HTTP_GET, detectMagnetic_sole);
  server.on("/detectIR", HTTP_GET, detectIR_sole);
  server.on("/detectRadio", HTTP_GET, detectRadio_sole);
  server.onNotFound(handleNotFound);

  // Start the server
  server.begin();
  Serial.println(F("HTTP server started"));
}


void loop() {
 server.handleClient(); 
}

