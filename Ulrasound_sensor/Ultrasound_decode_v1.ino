void setup() {
  // Initialize the debugging Serial port
  Serial.begin(115200);
  while (!Serial) {
    // Wait for the Serial port to be ready
  }

  // Initialize the UART port for reading the signal
  Serial1.begin(600, SERIAL_8N1);  // 600 baud, 8 data bits, no parity, 1 stop bit

  Serial.println("Setup complete. Ready to receive message.");
}

void loop() {
  if (Serial1.available()) {
    // Read the incoming byte from the UART port
    char incomingByte = Serial1.read();

    // Print the received byte to the Serial Monitor
    Serial.print("Received byte: ");
    Serial.println(incomingByte, BIN);  // Print the byte as a binary number

    // Decode the byte in ASCII
    Serial.print("Character: ");
    Serial.println(incomingByte);

    // Optionally, store the byte in a buffer for further processing
  }

  // Debugging: Print analog values from the signal pin
  int signal_value = analogRead(A0);
  Serial.print("Analog value: ");
  Serial.println(signal_value);
  delay(10);  // Small delay to avoid flooding the serial monitor
}