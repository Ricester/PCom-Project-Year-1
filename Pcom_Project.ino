
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <MFRC522.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define RST_PIN         13 
#define SS_PIN          10    

#define STATE_UNKNOWN       0
#define STATE_START         1
#define STATE_TIMER_RDY     2
#define STATE_ADD_TIME      3
#define STATE_REMOVE_TIME   4
#define STATE_TIMER_RUNNING 5
#define STATE_TIMER_PAUSED  6
#define STATE_TIMER_ENDED   7

#define BUTTON_ADD    4 //pin for the button that adds to the timer
#define BUTTON_REMOVE 5 //pin for the button that subtracts from the timer

#define BUTTON_PAUSE  4
#define BUTTON_RESUME 4

#define TIMER_BASE     180000
#define TIMER_INTERVAL 60000
#define TIMER_BUTTON_DELAY    200

#define PAUSE_FLASH_ON     1000
#define PAUSE_FLASH_OFF    500
#define PAUSE_BUTTON_DELAY 200 //changes delay time before being able to press the button more

int state = STATE_UNKNOWN;

unsigned long timer;
unsigned long lastLoop;

unsigned long pauseFlashLast;
boolean pauseScreen;

unsigned long enablePause;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

byte cardID[] = { 0x64, 0xf1, 0xa7, 0xb3 } ;
byte tagID[] = { 0x82, 0x64, 0xf4, 0x02 };

MFRC522::MIFARE_Key key;
// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  SPI.begin();
  mfrc522.PCD_Init();

  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("Please scan card");
  display.display();

  timer = TIMER_BASE;
  lastLoop = millis();
  pauseFlashLast = millis();
  pauseScreen = false;
  enablePause = millis();

  // Set button pins
  pinMode(BUTTON_ADD, INPUT);
  pinMode(BUTTON_REMOVE, INPUT);

  changeState(STATE_START);
}

// the loop function runs over and over again forever
void loop() {
  unsigned long currentTime = millis();
  if (state == STATE_START && waitForCard() && isCard()) {
    changeState(STATE_TIMER_RDY);
    displayTime();
  }
  if (state == STATE_TIMER_RDY) {
    // Get the button states
    int addState = digitalRead(BUTTON_ADD);
    int removeState = digitalRead(BUTTON_REMOVE);
    if (addState == HIGH) {
      timer += min(TIMER_INTERVAL, 60000 * 99);
    }
    if (removeState == HIGH) {
      timer -= max(TIMER_INTERVAL, 60000);
    }

    if (addState == HIGH || removeState == HIGH) {
      displayTime();
      // Delay to prevent the constant reading of buttons
      // Nothing else will be able to happen
      delay(TIMER_BUTTON_DELAY);
    }
  }
  if (state == STATE_TIMER_RDY && waitForCard() && isTag()) {
    changeState(STATE_TIMER_RUNNING);
  }
  if (state == STATE_TIMER_RUNNING && currentTime > enablePause) {
    // Get the button states
    if (digitalRead(BUTTON_PAUSE) == HIGH) {
      changeState(STATE_TIMER_PAUSED);
      enablePause = currentTime + PAUSE_BUTTON_DELAY;
    }
  }
  if (state == STATE_TIMER_PAUSED) {
    // Flash the OLED to indicate pausing
    if (!pauseScreen && (currentTime > (pauseFlashLast + PAUSE_FLASH_OFF))) {
      pauseFlashLast = currentTime;
      pause();
    }
    if (pauseScreen && (currentTime > (pauseFlashLast + PAUSE_FLASH_ON))) {
      pauseFlashLast = currentTime;
      displayTime();
    }
    
    if (digitalRead(BUTTON_RESUME) == HIGH && currentTime >  enablePause) {
      changeState(STATE_TIMER_RUNNING);
      enablePause = currentTime + PAUSE_BUTTON_DELAY;
    }
  }
  if (state == STATE_TIMER_RUNNING) {
    // Get the difference in time
    unsigned long diff = currentTime - lastLoop;
    // Update the timer
    if (timer <= diff) {
      timer = 0;  
    } else {
      timer = timer - diff;
    }
    Serial.print("Current time: ");
    Serial.println(currentTime);
    Serial.print("Timer: ");
    Serial.println(timer);
    Serial.print("Diff: ");
    Serial.println(diff);
    if (timer == 0) {
      changeState(STATE_TIMER_ENDED);
      displayTimerEnd();
    } else {
      displayTime();
    }
  }
  if (state == STATE_TIMER_ENDED) {
    // Currently does nothing
  }
  lastLoop = currentTime;
}

boolean waitForCard() { //repeats the string until a card/tag is present on the reader
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    Serial.println("New card not present!");
    return false;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    Serial.println("Can't read serial card");
    return false;
  }
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return false;
  }
  /*if (mfrc522.uid.uidByte[0] != key.keyByte[0] || 
    mfrc522.uid.uidByte[1] != key.keyByte[1] || 
    mfrc522.uid.uidByte[2] != key.keyByte[2] || 
    mfrc522.uid.uidByte[3] != key.keyByte[3] ) {
    Serial.println(F("A new card has been detected."));

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      key.keyByte[i] = mfrc522.uid.uidByte[i];
    }
  }
  else Serial.println(F("Card read previously."));*/

  for (byte i = 0; i < 4; i++) {
    key.keyByte[i] = mfrc522.uid.uidByte[i];
  }

  Serial.println(F("The NUID tag is:"));
  Serial.print(F("In hex: "));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
  Serial.println();
  Serial.print(F("In dec: "));
  printDec(key.keyByte, MFRC522::MF_KEY_SIZE);
  Serial.println();

  // Halt PICC
  mfrc522.PICC_HaltA();

  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();

  Serial.println("Card read success!");
  return true;
}
void changeState(int s) {
  state = s;
  Serial.print("Setting state to ");
  if (state == STATE_START) {
    Serial.print("STATE_START");
  } else if (state == STATE_ADD_TIME) {
    Serial.print("STATE_ADD_TIME");
  } else if (state == STATE_REMOVE_TIME) {
    Serial.print("STATE_REMOVE_TIME");
  } else if (state == STATE_TIMER_RUNNING) {
    Serial.print("STATE_TIMER_RUNNING");
  } else if (state == STATE_TIMER_PAUSED) {
    Serial.print("STATE_TIMER_PAUSED");
  } else if (state == STATE_TIMER_ENDED) {
    Serial.print("STATE_TIMER_ENDED");
  } else if (state == STATE_UNKNOWN) {
    Serial.print("STATE_UNKNOWN");
  }
  Serial.println(".");
}

void displayTimerEnd() { //for when the timer ends
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("Timer done, please reset!");
  display.display();
}

void displayTime() { //displays time remaining on the timer on the OLED screen
  char str[6];
  display.clearDisplay();

  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  unsigned long seconds = timer / 1000;
  // Display static text
  str[0] = '0' + (seconds / 600);
  str[1] = '0' + ((seconds % 600) / 60);
  str[2] = ':';
  str[3] = '0' + ((seconds % 60) / 10);
  str[4] = '0' + (seconds % 10);
  str[5] = '\0';
  
  display.print(str);
  display.display();
  pauseScreen = false;
}

void pause() {
  display.clearDisplay();
  pauseScreen = true;
  display.display();
}

boolean isCard() {
  for (int i = 0; i < 4; i++) {
    if (cardID[i] != key.keyByte[i]) {
      return false;
    }
  }
  return true;
}

boolean isTag() { 
  for (int i = 0; i < 4; i++) {
    if (tagID[i] != key.keyByte[i]) {
      return false;
    }
  }
  return true;
}

void printHex(byte *buffer, byte bufferSize) { //prints Hex value in the serial monitor
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void printDec(byte *buffer, byte bufferSize) { //prints Dec value in the serial monitor
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}
