#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ------------ Pin Definitions ------------
#define IR_PIN 15         // IR sensor output connected to digital pin 2
int devicePin[3] = {18, 23, 19};

LiquidCrystal_I2C lcd(0x27, 16, 2);  // (I2C address, columns, rows)

// --------------PushBullet Credentials-----------
const char* token = "o.hKuX2oErevtNMYwygabluMGOEIZj0n3G";  // Pushbullet API token
const char* ssid = "JIO_0711_5G";              // Your WiFi SSID
const char* pass = "RUDRAKSHA@1234";            // Your WiFi password

// ------------ State Variables ------------
bool eyeClosed = false;     // Tracks whether the eye is currently closed   
bool unconFlag = false;
bool systemFlag = true;
int flag = 1;
bool toggleState[3] = {LOW,LOW,LOW};
String turn="";

// ------------ Timing Variables ------------
unsigned long blinkStart = 0;           // Timestamp when blink starts
unsigned long blinkEnd = 0;             // Timestamp when blink ends
unsigned long firstBlinkTime = 0;       // Timestamp of first short blink
const unsigned long timeLimit = 400;    // Max gap between short blinks (ms)

int blinkCount = 0;  // Counts short blinks in a sequence
int devicecode = 0;
// -------------------PushBullet SOS part-------------------
void push_SOS(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    digitalWrite(Wifiled,HIGH);
    http.begin("https://api.pushbullet.com/v2/pushes");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Access-Token", token);

    String sms = "{\"type\": \"note\", \"title\": \"SOS_ALERT!\", \"body\": \"" + message + "\"}";
    Serial.println(sms);

    int response_no = http.POST(sms);
    String response = http.getString();

    switch (response_no) {
      case 200:
        Serial.println("SOS sent successfully");
        Serial.println("Pushbullet Response: " + response);
        break;
      case 401:
        Serial.println("Token error");
        Serial.println("Pushbullet Response: " + response);
        break;
      default:
        Serial.println("Pushbullet Response: " + response);
        Serial.println("Error sending push: " + http.errorToString(response_no));
    }

    http.end();
  } else {
    Serial.println("WiFi not connected.");
  }
}
// ------------ Setup Function ------------
void setup() {
  Serial.begin(9600);            // Start serial communication
 lcd.init();           // initialize the LCD
 lcd.backlight();       // turn on backlight
  pinMode(IR_PIN, INPUT);        // Set IR pin as input
  pinMode(devicePin[0], OUTPUT);      // Set LED pin as output
  pinMode(devicePin[1], OUTPUT);     // Set relay pin as output
  pinMode(devicePin[2], OUTPUT);
  WiFi.begin(ssid, pass);
  pinMode(Wifiled, OUTPUT);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected.");
  
  Serial.println("System Initialized.");
}

// ------------ Main Loop ------------
void loop() 
{
  int irState = digitalRead(IR_PIN);     // Read current IR sensor state
  unsigned long currentTime = millis();  // Record current time

  // ---- Blink Start Detection (Eye Closed) ----
  if (irState == LOW && !eyeClosed) 
  {
    blinkStart = currentTime;    // Mark the blink start time
    eyeClosed = true;            // Mark eye as closed
  }

  // ---- Blink End Detection (Eye Opened) ----
  if (irState == HIGH && eyeClosed) 
  {
    blinkEnd = currentTime;      // Mark the blink end time
    eyeClosed = false;           // Mark eye as open again

    unsigned long blinkDuration = blinkEnd - blinkStart;  // Calculate blink duration

    if ( blinkDuration > 4000 && blinkDuration < 6000) 
    {
      systemFlag = !systemFlag;     // Soft ON/OFF
    }

    if (blinkDuration > 5000 && systemFlag) 
    {
      (devicecode==2)? devicecode = 0 : devicecode++ ;
      if(devicecode==0)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("LIGHT");
      }
      if(devicecode==1)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("FAN");
      }
      if(devicecode==2)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("SPEAKER");
      }
      Serial.println(devicePin[devicecode]);
      delay(200);
    }
    else if (blinkDuration > 2500 && systemFlag) 
    {
      toggleState[devicecode] = !toggleState[devicecode];
      digitalWrite(devicePin[devicecode], toggleState[devicecode]);
      if(toggleState[devicecode]==LOW)
      turn="OFF";
      if(toggleState[devicecode]==HIGH)
      turn="ON";

      if(devicecode==0)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("LIGHT "+turn);
      }
      if(devicecode==1)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("FAN "+turn);
      }
      if(devicecode==2)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("SPEAKER"+turn);
      }
      Serial.println(toggleState[devicecode]);
      delay(200);
    }
    
    // ---- Short Blink Handling (Double or Triple Blink) ----
    else if (irState == HIGH && systemFlag) 
    {
      delay(200);  // Small delay for stable reading
      bool recheck = digitalRead(IR_PIN);  // Confirm eye is open again

      if (recheck == HIGH) 
      {
        blinkCount++;  // Count this short blink
        Serial.print("Blink ");
        Serial.print(blinkCount);
        Serial.println(" (LOW→HIGH) detected.");

        // ---- First Short Blink: Start Timer ----
        if (blinkCount == 1) {
          firstBlinkTime = currentTime;
        }

        // ---- Double Blink Detected ----
        else if (blinkCount == 2) {
          if (currentTime - firstBlinkTime <= timeLimit) 
          {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("SOS Sent");
            push_SOS("SOS Sent"); 
            delay(1000);
            blinkCount = 0;  // Reset counter
            }
        } 
      }
    }
  }

  else if((millis()-blinkStart)>timeLimit && blinkCount==1)
  {
    blinkCount=0;
    Serial.println("done");
    delay(200);
  }
  else if((millis()-blinkStart)>8000 && irState == LOW && !unconFlag && systemFlag)
  {
    push_SOS("User may be uncounsious");
    while ((millis()-blinkStart)<12000 && irState == LOW) {
      irState = digitalRead(IR_PIN);
    }
    if(irState == HIGH) {
      push_SOS("User seems okay");
      //break;
    } else {
      push_SOS("🚨 User is surely unconsious, Please return home");
    }
    while ((millis()-blinkStart)<20000 && irState == LOW) {
      irState = digitalRead(IR_PIN);
    }
     if(irState == HIGH) {
      push_SOS("User seems okay");
      break;
    } else {
      push_SOS("🚨 ALERT!!!! Please return home asap");
      unconFlag = true;
    }
  }
}
