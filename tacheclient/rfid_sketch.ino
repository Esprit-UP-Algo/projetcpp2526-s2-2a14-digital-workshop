#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9

MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  Serial.println("Pose la carte...");
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  unsigned long uidPhysique = 0;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uidPhysique = (uidPhysique << 8) | mfrc522.uid.uidByte[i];
  }

  // On envoie directement le numéro brut au PC !
  Serial.println(uidPhysique); 
  
  delay(2000); 
}
