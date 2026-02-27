// LAB 4  Microprocessors //
//Solar and Battery Power Management System//

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//set up screen//
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Pinout defined//
#define SOLAR_PIN    A0   // Potentiometer representing Solar//
#define BAT_PIN      A1   // Potentiometer representing Battery//
#define PIN_MCU_SRC   8   // HIGH =solar, LOW =battery//
#define PIN_CHARGE    9   // HIGH =battery charging//

//Constants defined//
#define VREF            5.0   
#define V_SOLAR_MIN     2.50  
#define V_BAT_FULL      4.0 
#define V_BAT_RECHARGE  3.0 


enum State {
  NO_SOLAR,            
  CHARGING_PRIORITY,  
  BAT_FULL_SOLAR_MCU   

State currentState = NO_SOLAR;


float readVolts(int pin) {
  return (analogRead(pin) * VREF) / 1023.0;
}


void updateState(float Vsolar, float Vbat) {
  switch (currentState) {

    case NO_SOLAR:
      if (Vsolar >= V_SOLAR_MIN) {
        currentState = (Vbat < V_BAT_FULL) ? CHARGING_PRIORITY : BAT_FULL_SOLAR_MCU;
      }
      break;

    case CHARGING_PRIORITY:
      if (Vsolar < V_SOLAR_MIN) {
        currentState = NO_SOLAR;          
      } else if (Vbat >= V_BAT_FULL) {
        currentState = BAT_FULL_SOLAR_MCU; 
      break;

    case BAT_FULL_SOLAR_MCU:
      if (Vsolar < V_SOLAR_MIN) {
        currentState = NO_SOLAR;           
      } else if (Vbat <= V_BAT_RECHARGE) {
        currentState = CHARGING_PRIORITY;  
      }
      break;
  }
}


void applyOutputs(bool mcuOnSolar, bool charging) {
  digitalWrite(PIN_MCU_SRC, mcuOnSolar ? HIGH : LOW);
  digitalWrite(PIN_CHARGE,  charging   ? HIGH : LOW);
}

//Screen Updating//
void updateOLED(float Vsolar, float Vbat, bool mcuOnSolar, bool charging) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.print("Vsolar: "); display.print(Vsolar, 2); display.println("V");
  display.print("Vbat:   "); display.print(Vbat,   2); display.println("V");
  display.println();
  display.print("MCU: "); display.println(mcuOnSolar ? "SOLAR"   : "BATTERY");
  display.print("CHG: "); display.println(charging   ? "ON"      : "OFF");
  display.print("ST:  ");

  switch (currentState) {
    case NO_SOLAR:          display.println("NO_SOLAR");    break;
    case CHARGING_PRIORITY: display.println("CHG_PRIORITY"); break;
    case BAT_FULL_SOLAR_MCU: display.println("BAT_FULL");   break;
  }

  display.display();
}

void setup() {
  pinMode(PIN_MCU_SRC, OUTPUT);
  pinMode(PIN_CHARGE,  OUTPUT);
  analogReference(DEFAULT);

  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();
}

void loop() {
  float Vsolar = readVolts(SOLAR_PIN);
  float Vbat   = readVolts(BAT_PIN);

  updateState(Vsolar, Vbat);

  bool mcuOnSolar = (currentState == BAT_FULL_SOLAR_MCU);
  bool charging   = (currentState == CHARGING_PRIORITY);

  applyOutputs(mcuOnSolar, charging);
  updateOLED(Vsolar, Vbat, mcuOnSolar, charging);

  delay(50);
}
