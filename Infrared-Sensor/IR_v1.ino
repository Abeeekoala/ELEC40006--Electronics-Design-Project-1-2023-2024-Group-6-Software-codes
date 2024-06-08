const int signalPin = 2; // The pin where the signal is connected
unsigned long lastTime = 0;
unsigned long currentTime = 0;
unsigned long interval = 0;
bool lastState = LOW;
bool currentState = LOW;

void setup() {
  Serial.begin(9600);
  pinMode(signalPin, INPUT);
  Serial.println("Setup complete");  // Debugging statement
}

void loop() {
  currentState = digitalRead(signalPin); // Read the current state of the pin
  if (currentState != lastState) { // Check if state has changed
    if (currentState == HIGH) { // Check for a rising edge
      currentTime = micros(); // Get the current time in microseconds
      interval = currentTime - lastTime; // Calculate the time interval
      lastTime = currentTime; // Update the last time
      float frequency = 1000000.0 / interval; // Calculate the frequency in Hz
      Serial.print("Frequency: ");
      Serial.print(frequency);
      Serial.println(" Hz");
    } else {
      Serial.println("Falling edge detected"); // Debugging statement for falling edge
    }
  }
  lastState = currentState; // Update the last state

  // Additional debug statement to verify loop is running
  static unsigned long lastDebugTime = 0;
  if (millis() - lastDebugTime > 1000) { // Print every 1 second
    Serial.println("Loop running");
    lastDebugTime = millis();
  }
}
