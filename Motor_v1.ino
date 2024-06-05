#define USE_WIFI_NINA         false
#define USE_WIFI101           true

#include <WiFiWebServer.h>
#include <string.h>

// Replace with your network credentials
const char ssid[] = "XXXX";
const char pass[] = "xxxx";

const int groupNumber = 0; // Set your group number to make the IP address constant - only do this on the EEERover network

// Define motor control pins
const int DIR1_PIN = 4; // Direction pin for left motor
const int PWM1_PIN = 3; // PWM pin for left motor
const int DIR2_PIN = 8; // Direction pin for right motor
const int PWM2_PIN = 9; // PWM pin for right motor

WiFiWebServer server(80);
String nameDetected = "N/A";
bool messageComplete = false; // Flag to indicate message completion
static char buffer[5]; // Buffer to store 4 characters + null terminator
static int bufferIndex = 0;  // Index to track the buffer position

// Webpage to return when root is requested
const char webpage[] = R"rawliteral(
<html>
<head>
  <style>
    .btn {background-color: inherit; padding: 14px 28px; font-size: 16px;}
    .btn:hover {background: #eee;}
  </style>
</head>
<body>
  <h1>Rover Control</h1>
  <p>Use WASD keys to control the rover. Press 'E' to detect name. Press 'R' to reset.</p>
  <p>Detected Name: <span id="name">N/A</span></p>
</body>
<script>
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("name").innerHTML = this.responseText;
    }
  };
  
  document.addEventListener('keydown', function(event) {
    var key = event.key.toLowerCase();
    if (key === 'w') {
      xhttp.open("GET", "/forward", true);
    } else if (key === 'a') {
      xhttp.open("GET", "/left", true);
    } else if (key === 's') {
      xhttp.open("GET", "/backward", true);
    } else if (key === 'd') {
      xhttp.open("GET", "/right", true);
    } else if (key === ' ') { // Space bar to stop
      xhttp.open("GET", "/stop", true);
    }
    xhttp.send();
  });

</script>
</html>
)rawliteral";

// Return the web page
void handleRoot() {
  Serial.println("Root page requested");
  server.send(200, F("text/html"), webpage);
}

// Handle motor control commands
void moveForward() {
  Serial.println("Move Forward");
  stopMotors(); // Ensure motors are stopped before changing direction
  delay(100); // Brief delay to ensure motors stop
  digitalWrite(DIR1_PIN, HIGH);
  analogWrite(PWM1_PIN, 255); // Full speed
  digitalWrite(DIR2_PIN, HIGH);
  analogWrite(PWM2_PIN, 255); // Full speed
  server.send(200, F("text/plain"), F("Forward"));
}

void moveBackward() {
  Serial.println("Move Backward");
  stopMotors(); // Ensure motors are stopped before changing direction
  delay(100); // Brief delay to ensure motors stop
  digitalWrite(DIR1_PIN, LOW);
  analogWrite(PWM1_PIN, 255); // Full speed
  digitalWrite(DIR2_PIN, LOW);
  analogWrite(PWM2_PIN, 255); // Full speed
  server.send(200, F("text/plain"), F("Backward"));
}

void turnLeft() {
  Serial.println("Turn Left");
  stopMotors(); // Ensure motors are stopped before changing direction
  delay(100); // Brief delay to ensure motors stop
  digitalWrite(DIR1_PIN, LOW);
  analogWrite(PWM1_PIN, 255); // Full speed
  digitalWrite(DIR2_PIN, HIGH);
  analogWrite(PWM2_PIN, 255); // Full speed
  server.send(200, F("text/plain"), F("Left"));
}

void turnRight() {
  Serial.println("Turn Right");
  stopMotors(); // Ensure motors are stopped before changing direction
  delay(100); // Brief delay to ensure motors stop
  digitalWrite(DIR1_PIN, HIGH);
  analogWrite(PWM1_PIN, 255); // Full speed
  digitalWrite(DIR2_PIN, LOW);
  analogWrite(PWM2_PIN, 255); // Full speed
  server.send(200, F("text/plain"), F("Right"));
}

void stopMotors() {
  Serial.println("Stop Motors");
  analogWrite(PWM1_PIN, 0); // Stop left motor
  analogWrite(PWM2_PIN, 0); // Stop right motor
  server.send(200, F("text/plain"), F("Stop"));
}

// Generate a 404 response with details of the failed request
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
  pinMode(PWM2_PIN, OUTPUT); // Although used with analogWrite, it's still set as OUTPUT

  Serial.begin(9600);
  Serial1.begin(600, SERIAL_8N1);  // 600 baud, 8 data bits, no parity, 1 stop bit

  // Wait 10s for the serial connection before proceeding
  // This ensures you can see messages from startup() on the monitor
  // Remove this for faster startup when the USB host isn't attached
  while (!Serial && millis() < 10000);

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

  // Register the callbacks to respond to HTTP requests
  server.on(F("/"), handleRoot);
  server.on(F("/forward"), moveForward);
  server.on(F("/backward"), moveBackward);
  server.on(F("/left"), turnLeft);
  server.on(F("/right"), turnRight);
  server.on(F("/stop"), stopMotors);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
