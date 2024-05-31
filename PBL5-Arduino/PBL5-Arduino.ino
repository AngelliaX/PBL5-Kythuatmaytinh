#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

//Code explaination for future use:
// READYTOUSE is an idle state, it doesnt print anything on LCD, also, when currentState is a number, it is also the same use-case of READYTOUSE

#define STARTING "STARTING"
#define TAKINGPICTURE "TAKINGPICTURE"
#define PROCESSIMAGE "PROCESSIMAGE"
#define COMPLETE "COMPLETE"
#define READYTOUSE "READYTOUSE"

#define WIFIFAILED "WIFIFAILED"
#define CAMFAILURE "CAMFAILURE"
#define INTERNALERROR "INTERNALERROR"
#define FLASKERROR "FLASKERROR"
String currentState = STARTING;

byte heart[8] = { 0b00000, 0b01010, 0b11111, 0b11111, 0b01110, 0b00100, 0b00000, 0b00000 };
byte R[] = {B00000,B00000,B00000,B01110,B01010, B01100, B01010, B01010};
byte E[] = {B00000,B00000,B00000,B01110,B01000,B01110,B01000,B01110};
byte A[] = {B00000,B00000,B00000,B00100,B01010,B01110,B01010,B01010};
byte D[] = {B00000,B00000,B00000,B01100,B01010,B01010,B01010,B01100};
byte Y[] = {B00000,B00000,B00000,B01010,B01010,B00100,B00100,B00100};
byte tick[] = {B00000,B00000,B00000,B00001,B00011,B10110,B11100,B01000};

int button = 7;

void setup() {
  Serial.begin(9600);

  pinMode(button, INPUT_PULLUP);

  lcd.begin(16, 2);
  lcd.clear();

  LCDPrint(0, "Pipe Counter");
  LCDPrint(1, "Initializing");
  
  lcd.createChar(1, R);
  lcd.createChar(2, E);
  lcd.createChar(3, A);
  lcd.createChar(4, D);
  lcd.createChar(5, Y);
  lcd.createChar(6, tick);

  lcd.createChar(0, heart);
  lcd.setCursor(13, 0);
  lcd.write( (byte) 0 );
  lcd.setCursor(14, 0);
  lcd.write( (byte) 0 );
  lcd.setCursor(15, 0);
  lcd.write( (byte) 0 );
}

//use this to check if esp32-cam has not passed the READYTOUSE part then keep printing everything 
static int isFinish = 0;

void loop() {
  String esp32Data = readEsp32CAMData();
  if (esp32Data != "") {
    currentState = esp32Data;
    Serial.println(esp32Data);
    Serial.println("");
    Serial.println("-----");
  }

  if(currentState == STARTING){
    checkEsp32CAMState();
  }else if(currentState == TAKINGPICTURE){
    takingPicture();
  }else if(currentState == PROCESSIMAGE){
    processImage();
  }else if(currentState == WIFIFAILED){
    LCDPrint(1, "Wifi Failed !!!");
  }else if(currentState == CAMFAILURE){
    LCDPrint(1, "Camera Failed !!");
  }else if(currentState == INTERNALERROR){
    LCDPrint(1, "Internal Error !");
  }else if(currentState == FLASKERROR){
    LCDPrint(1, "AI Error !!!");
  }else if(currentState == READYTOUSE){
    if(!isFinish){
      isFinish = 1;
    }
    static bool isFirstTime = true;
    static unsigned long animationTimer = 0;
    const int animationInterval = 300;
    unsigned long currentMillis = millis();
    
    if (isFirstTime) {
      isFirstTime = false;
      LCDPrint(1,"");
    }

    if (currentMillis - animationTimer >= 500) {
      animationTimer = currentMillis;

      lcd.setCursor(10, 1);
      lcd.write( (byte) 6 );
      lcd.setCursor(11, 1);
      lcd.write( (byte) 1 );
      lcd.setCursor(12, 1);
      lcd.write( (byte) 2 );
      lcd.setCursor(13, 1);
      lcd.write( (byte) 3 );
      lcd.setCursor(14, 1);
      lcd.write( (byte) 4 );
      lcd.setCursor(15, 1);
      lcd.write( (byte) 5 );
    }
  }else if(currentState.startsWith("Result:") && currentState.indexOf('-') != -1){
    completeCounting(currentState);
  }
  else{
    if(!isFinish){
      LCDPrint(1,currentState.c_str());
    }
  }
}

void completeCounting(String esp32Data) {
    LCDPrint(1, "");
    // String message = "Result: " + esp32Data;
    // LCDPrint(0, message.c_str());
    // currentState = READYTOUSE;
    
    int dashIndex = esp32Data.indexOf('-');
    String numbers = esp32Data.substring(7, dashIndex);
    int firstNumber = numbers.toInt();
    String secondPart = esp32Data.substring(dashIndex + 1);
    int secondNumber = secondPart.toInt();
    
    // Concatenate the strings to form the message
    String message = "Result: " + String(firstNumber) + " (" + String(secondNumber) + ")";
    // Check if the message length is greater than or equal to 17
    if (message.length() >= 17) {
        // Remove the space bar at index 8
        message.remove(7, 1);
    }

    LCDPrint(0, message.c_str());
    currentState = READYTOUSE;
}


void processImage() {
  LCDPrint(1,"");

  static unsigned long animationTimer = 0;
  const int animationInterval = 300;
  unsigned long currentMillis = millis();

  if (currentMillis - animationTimer >= animationInterval) {
    animationTimer = currentMillis;
    static byte animationCount = 0;
    switch (animationCount) {
      case 0: LCDPrint(0, "AI Detecting."); break;
      case 1: LCDPrint(0, "AI Detecting.."); break;
      case 2: LCDPrint(0, "AI Detecting..."); break;
      case 3: LCDPrint(0, "AI Detecting..."); break;
    }
    animationCount = (animationCount + 1) % 4;
  }
}

void takingPicture() {
  LCDPrint(1,"");

  static unsigned long animationTimer = 0;
  const int animationInterval = 300;
  unsigned long currentMillis = millis();

  if (currentMillis - animationTimer >= animationInterval) {
    animationTimer = currentMillis;
    static byte animationCount = 0;
    switch (animationCount) {
      case 0: LCDPrint(0, "Taking picture."); break;
      case 1: LCDPrint(0, "Taking picture.."); break;
      case 2: LCDPrint(0, "Taking picture..."); break;
      case 3: LCDPrint(0, "Taking picture..."); break;
    }
    animationCount = (animationCount + 1) % 4;
  }
}

void checkEsp32CAMState() {
  static unsigned long animationTimer = 0;
  const int animationInterval = 300;
  unsigned long currentMillis = millis();

  if (currentMillis - animationTimer >= animationInterval) {
    animationTimer = currentMillis;
    static byte animationCount = 0;
    switch (animationCount) {
      case 0: LCDPrint(1, "Initializing."); break;
      case 1: LCDPrint(1, "Initializing.."); break;
      case 2: LCDPrint(1, "Initializing..."); break;
      case 3: LCDPrint(1, "Initializing..."); break;
    }
    animationCount = (animationCount + 1) % 4;
  }
}

String readEsp32CAMData() {
  if (Serial.available()) {
    String receivedString = Serial.readStringUntil('\n');
    receivedString.trim();
    return receivedString;
  }
  return "";  // Return an empty string if no data is available
}

void LCDPrint(int line, const char* text) {
  char truncatedText[17];
  int textLength = strlen(text);

  // Copy the text to truncatedText and fill the remaining characters with empty spaces
  for (int i = 0; i < 16; ++i) {
    if (i < textLength) {
      truncatedText[i] = text[i];
    } else {
      truncatedText[i] = ' ';
    }
  }
  truncatedText[16] = '\0';

  lcd.setCursor(0, line);
  lcd.print(truncatedText);
}

bool isNumeric(String str) {
  for (unsigned int i = 0; i < str.length(); i++) {
    if (!isdigit(str.charAt(i))) {
      return false;
    }
  }
  return true;
}
