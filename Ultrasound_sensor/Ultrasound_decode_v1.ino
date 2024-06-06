#define SIGNAL_PIN A0  // Pin where the demodulated signal is connected
#define BAUD_RATE 600  // Baud rate of the UART signal

const int THRESHOLD_HIGH = 450;  // Threshold for binary 1
const int THRESHOLD_LOW = 125;   // Threshold for binary 0
const int BIT_DURATION = 1000000 / BAUD_RATE;  // Bit duration in microseconds

void setup() {
  Serial.begin(115200);  // Serial monitor for debugging
  pinMode(SIGNAL_PIN, INPUT);
}

void loop() {
  decode_uart();
  while (true) {
    // Stop the loop after decoding one message
  }
}

void decode_uart() {
  String binary_message = "";
  bool capturing = false;
  unsigned long start_time = 0;
  int bit_counter = 0;
  int bits_read = 0;

  Serial.println("Starting UART signal capture...");

  while (bits_read < 44) {  // 4 characters * (1 start bit + 8 data bits + 1 stop bit)
    int signal_value = analogRead(SIGNAL_PIN);
    bool signal_bit = (signal_value > THRESHOLD_HIGH) ? 1 : ((signal_value < THRESHOLD_LOW) ? 0 : -1);

    if (!capturing && signal_bit == 0) {
      // Detected start bit
      capturing = true;
      start_time = micros();
      bit_counter = 0;
      bits_read = 0;
      binary_message += '0';  // Start bit
      bits_read++;
      Serial.println("Start bit detected.");
    }

    if (capturing) {
      if ((micros() - start_time) >= (BIT_DURATION * (bit_counter + 1.5))) {
        if (bit_counter > 0 && bit_counter < 9) {  // Valid signal bit
          if (signal_bit != -1) {  // Valid signal bit
            binary_message += signal_bit ? '1' : '0';
            bits_read++;
            Serial.print("Bit read: ");
            Serial.println(signal_bit ? '1' : '0');
          } else {
            Serial.println("Invalid signal bit detected.");
          }
        } else if (bit_counter == 9) {  // Stop bit
          binary_message += '1';  // Stop bit is always 1
          bits_read++;
          Serial.println("Stop bit detected.");
          capturing = false;
        }
        bit_counter++;
      }
    }
  }

  Serial.println("Binary Message: " + binary_message);

  // Convert binary message to readable characters
  for (int i = 0; i < 4; i++) {
    String char_bits = binary_message.substring(1 + i * 11, 9 + i * 11); // Extract the 8 data bits
    char c = (char) strtol(char_bits.c_str(), NULL, 2);
    Serial.print(c);
  }
  Serial.println();
}
