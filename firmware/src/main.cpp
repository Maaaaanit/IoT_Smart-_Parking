#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

// Keep these pin assignments aligned with hardware-guide.html.
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
// Use your computer's LAN IPv4 address, not localhost/127.0.0.1.
const char* SERVER_URL = "http://192.168.1.100:5000";
const char* DEVICE_ID = "ESP-ENTRY-001";
const char* DEVICE_API_KEY = "REPLACE_WITH_DEVICE_API_KEY";
const uint8_t SERVO_PIN = 18, BUZZER_PIN = 19, GREEN_LED = 4, RED_LED = 5;
const uint8_t ROWS=4, COLS=4;
char keys[ROWS][COLS]={{'1','2','3','A'},{'4','5','6','B'},{'7','8','9','C'},{'*','0','#','D'}};
byte rowPins[ROWS]={13,12,14,27};
byte colPins[COLS]={26,25,33,32};
Keypad keypad=Keypad(makeKeymap(keys),rowPins,colPins,ROWS,COLS);
LiquidCrystal_I2C lcd(0x27,20,4);
Servo gate;
String plate=""; bool exitMode=false; bool alphaMode=false;

void message(const String&a,const String&b="",const String&c="",const String&d=""){
  lcd.clear(); lcd.setCursor(0,0);lcd.print(a.substring(0,20));
  lcd.setCursor(0,1);lcd.print(b.substring(0,20));lcd.setCursor(0,2);lcd.print(c.substring(0,20));lcd.setCursor(0,3);lcd.print(d.substring(0,20));
}
void beep(){tone(BUZZER_PIN,1800,55);}
void gateOpen(){digitalWrite(GREEN_LED,HIGH);digitalWrite(RED_LED,LOW);gate.write(90);delay(2500);gate.write(0);digitalWrite(GREEN_LED,LOW);digitalWrite(RED_LED,HIGH);}
String postJson(const String&path,const String&body){
  if(WiFi.status()!=WL_CONNECTED)return "{\"success\":false,\"error\":\"WiFi disconnected\"}";
  HTTPClient http;http.setTimeout(10000);http.begin(String(SERVER_URL)+path);http.addHeader("Content-Type","application/json");
  // Change this header if the existing authenticateDevice middleware expects another scheme.
  http.addHeader("x-device-id",DEVICE_ID);http.addHeader("x-device-api-key",DEVICE_API_KEY);
  int code=http.POST(body);String response=code>0?http.getString():"{\"success\":false,\"error\":\"HTTP request failed\"}";
  Serial.printf("POST %s -> %d\n",path.c_str(),code);Serial.println(response);http.end();return response;
}
void submitPlate(){
  plate.trim();if(plate.length()<4){message("INVALID PLATE","Enter at least 4 chars");digitalWrite(RED_LED,HIGH);return;}
  plate.toUpperCase();message(exitMode?"EXIT: CHECKING...":"ENTRY: CHECKING...",plate,"Contacting server...");
  String body="{\"plateNumber\":\""+plate+"\",\"vehicleType\":\"CAR\"}";
  String response=postJson(exitMode?"/api/exit/manual-scan":"/api/entry/manual-entry",body);
  JsonDocument doc;auto err=deserializeJson(doc,response);
  if(err||!doc["success"].as<bool>()){
    const char* why=doc["error"]|"Server response error";message("REQUEST FAILED",String(why).substring(0,20),"Check Serial Monitor");digitalWrite(RED_LED,HIGH);digitalWrite(GREEN_LED,LOW);plate="";return;
  }
  digitalWrite(RED_LED,LOW);digitalWrite(GREEN_LED,HIGH);
  if(!exitMode){String slot=doc["data"]["slotId"]|"Slot assigned";message("ENTRY APPROVED",plate,"Proceed to:",slot);gateOpen();}
  else {float amount=doc["data"]["feeBreakdown"]["totalAmount"]|doc["data"]["payment"]["amount"]|0.0;String amountText="Amount: Rs "+String(amount,2);message("EXIT BILL",plate,amountText,"Confirm payment first");/* Deliberately do not open gate on exit automatically. Integrate verified payment-status endpoint before gateOpen(). */}
  plate="";
}
void setup(){
  Serial.begin(115200);pinMode(BUZZER_PIN,OUTPUT);pinMode(GREEN_LED,OUTPUT);pinMode(RED_LED,OUTPUT);digitalWrite(RED_LED,HIGH);
  Wire.begin(21,22);lcd.init();lcd.backlight();gate.setPeriodHertz(50);gate.attach(SERVO_PIN,500,2400);gate.write(0);
  message("SMART PARKING","Connecting WiFi...");WiFi.mode(WIFI_STA);WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  unsigned long start=millis();while(WiFi.status()!=WL_CONNECTED&&millis()-start<20000){delay(500);Serial.print('.');}
  if(WiFi.status()==WL_CONNECTED){Serial.print("\nWiFi IP: ");Serial.println(WiFi.localIP());message("WiFi Connected",WiFi.localIP().toString(),"C=ENTRY  D=EXIT","#=Submit A=Backspace");}
  else message("WIFI FAILED","Check SSID/password");
}
void loop(){
  char key=keypad.getKey();if(!key)return;beep();Serial.printf("Key: %c\n",key);
  if(key=='C'){exitMode=false;message("ENTRY MODE",plate,"#=Submit A=Backspace");return;}
  if(key=='D'){exitMode=true;message("EXIT MODE",plate,"#=Submit A=Backspace");return;}
  if(key=='*'){alphaMode=!alphaMode;message(alphaMode?"LETTER MODE":"NUMBER MODE",plate);return;}
  if(key=='A'){if(plate.length())plate.remove(plate.length()-1);message(exitMode?"EXIT MODE":"ENTRY MODE",plate,"#=Submit A=Backspace");return;}
  if(key=='B'){plate="";message("INPUT CLEARED");return;}
  if(key=='#'){submitPlate();return;}
  // Numeric input is the reliable baseline. T9 alpha mode is indicated on LCD but not implemented here.
  if(key>='0'&&key<='9'&&plate.length()<12){plate+=key;message(alphaMode?"LETTER MODE (T9 TBD)":"PLATE INPUT",plate,"C=Entry D=Exit","#=Submit B=Clear");}
}
