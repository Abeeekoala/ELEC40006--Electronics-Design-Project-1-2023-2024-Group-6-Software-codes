#define USE_WIFI_NINA         false 
#define USE_WIFI101           true  
#include <WiFiWebServer.h>  
#include <arduinoFFT.h>  // Ensure this library is installed


#define SAMPLES 128             // Must be a power of 2
#define SAMPLING_FREQUENCY 1000 // Hz, must be less than 10000 due to ADC

const char ssid[] = "xxxx";
const char pass[] = "xxxx";

// FFT related variables
ArduinoFFT<double> FFT = ArduinoFFT<double>();
unsigned int samplingPeriodUs;
unsigned long microseconds;
double vReal[SAMPLES];
double vImag[SAMPLES];

WiFiWebServer server(80);
String frequencyDetected;


// Webpage to return when root is requested
const char webpage[] = R"rawliteral(
<html>
<head>
 <style>   // CSS part
   .btn {background-color: inherit; padding: 14px 28px; font-size: 16px;}
   .btn:hover {background: #eee;}
 </style>
</head>
<body>   // HTML part 
 <h1>Rover Control</h1>
  <p>Use WASD keys to control the rover. Press 'E' to detect frequency.</p>
  <p>Detected Frequency: <span id="frequency">N/A</span></p>
</body>
<script>  // Javascript part 
 var xhttp = new XMLHttpRequest();
 xhttp.onreadystatechange = function() {
   if (this.readyState == 4 && this.status == 200) {
     document.getElementById("state").innerHTML = this.responseText;
   }
 };
  document.addEventListener('keydown', function(event) {
   var key = event.key.toLowerCase();
   if (key === 'e') {
     xhttp.open("GET", "/detect", true);
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

void detectFrequency() {
  Serial.println("Detecting Frequency");
  
  // Sampling period in microseconds
  samplingPeriodUs = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
  
  // Collect samples
  for (int i = 0; i < SAMPLES; i++) {
    microseconds = micros();
    
    vReal[i] = analogRead(A0); // Reading the analog signal from A0
    vImag[i] = 0;
    
    while (micros() < (microseconds + samplingPeriodUs)) {
      // wait for the required sampling interval
    }
  }
  
  // Perform FFT
  FFT.windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.complexToMagnitude(vReal, vImag, SAMPLES);  // Corrected function call

  // Find peak frequency
  double peakFrequency = FFT.majorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);
  frequencyDetected = String(peakFrequency) + " Hz";
  Serial.println("Peak Frequency: " + frequencyDetected);
  
  // Send the frequency to the client
  server.send(200, "text/plain", frequencyDetected);
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

 Serial.begin(9600);
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
 server.on(F("/detect"), detectFrequency);
 server.onNotFound(handleNotFound);

 server.begin();
 Serial.println("HTTP server started");
}


void loop() {
 server.handleClient();
}
