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
  static char buffer[5]; // Buffer to store 4 characters + null terminator
  static int index = 0;  // Index to track the buffer position
  static bool messageComplete = false; // Flag to stop receiving

  if (!messageComplete && Serial1.available()) {
    // Read the incoming byte from the UART port
    char incomingByte = Serial1.read();

    // Print the received byte to the Serial Monitor
    Serial.print("Received byte: ");
    Serial.println(incomingByte, BIN);  // Print the byte as a binary number

    // Decode the byte if necessary (this depends on the expected format)
    // Assuming the message is directly readable ASCII characters
    Serial.print("Character: ");
    Serial.println(incomingByte);

    // Store the byte in the buffer if it's part of the 4-character name
    if (index < 4) {
      buffer[index++] = incomingByte;
    }

    // Check if the buffer contains a 4-character name starting with '#'
    if (index == 4) {
      buffer[index] = '\0';  // Null-terminate the buffer
      if (buffer[0] == '#') {
        Serial.print("Detected name: ");
        Serial.println(buffer);
        messageComplete = true;  // Stop receiving further characters
      } else {
        // Shift buffer content to the left by one position and reset index
        for (int i = 0; i < 3; i++) {
          buffer[i] = buffer[i + 1];
        }
        index = 3;
      }
    }
  }

  // Debugging: Print analog values from the signal pin
  // int signal_value = analogRead(A0);
  // Serial.print("Analog value: ");
  // Serial.println(signal_value);
  // delay(10);  // Small delay to avoid flooding the serial monitor
}
