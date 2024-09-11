void setup() {
  Serial.begin(115200);    // Initialize serial communication with the Serial Monitor
  Serial1.begin(9600, SERIAL_8N1, 41, 40); // Initialize serial communication with the GPS module
  Serial.print("starting\r\n");

}

void loop() {
  if (Serial1.available()) {  // Check if data is available from the GPS module
    char c = Serial1.read();  // Read a character from the GPS module 
    Serial.print(c);            // Print the character to the Serial Monitor (Raw Data)
  }
}


