// Mapping segments to Arduino Pins based on your wiring
const int segments[] = {2, 3, 4, 5, 6, 8, 9}; 
const int btnUp = 11;   
const int btnDown = 12; 

int count = 0;

// Binary map for 0-9
byte numbers[10] = {
  0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110,
  0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01101111
};

// --- FUNCTIONS DEFINED FIRST TO AVOID SCOPE ERRORS ---

void displayNum(int num) {
  for (int i = 0; i < 7; i++) {
    bool bit = bitRead(numbers[num], i);
    // Logic flipped (!) for Common Anode so it stays lit correctly
    digitalWrite(segments[i], !bit); 
  }
}

void updateSystem() {
  displayNum(count);
  Serial.println(count); // Sends to your Qt C++ app
}

// --- STANDARD ARDUINO STRUCTURE ---

void setup() {
  Serial.begin(9600); 
  
  for (int i = 0; i < 7; i++) {
    pinMode(segments[i], OUTPUT);
  }
  
  pinMode(btnUp, INPUT_PULLUP);
  pinMode(btnDown, INPUT_PULLUP);
  
  updateSystem(); // Initialize at 0
}

void loop() {
  // Sync with Qt/BDD (Listen for incoming numbers)
  if (Serial.available() > 0) {
    int incoming = Serial.parseInt();
    if (incoming >= 0 && incoming <= 9) {
      count = incoming;
      updateSystem();
    }
  }

  // UP Button Logic: 0 to 9 loop
  if (digitalRead(btnUp) == LOW) {
    delay(50); // Debounce
    if (digitalRead(btnUp) == LOW) {
      count++;
      if (count > 9) count = 0; 
      updateSystem();
      while(digitalRead(btnUp) == LOW); // Wait for release
    }
  }

  // DOWN Button Logic: 9 to 0 loop
  if (digitalRead(btnDown) == LOW) {
    delay(50); 
    if (digitalRead(btnDown) == LOW) {
      count--;
      if (count < 0) count = 9; 
      updateSystem();
      while(digitalRead(btnDown) == LOW); // Wait for release
    }
  }
}