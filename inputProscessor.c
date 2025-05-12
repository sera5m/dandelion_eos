#include "Wiring.h"



//volatile bool button0Pressed = false;
//volatile bool button1Pressed = false;

//volatile int encoder0_Pos = 0;
//volatile int encoder1_Pos = 0;
/*
int lastCLK_0 = HIGH;
int lastCLK_1 = HIGH;

void IRAM_ATTR handleButton0Interrupt() {
  button0Pressed = true;
}

void IRAM_ATTR handleButton1Interrupt() {
  button1Pressed = true;
}
/*
void setup() {
  Serial.begin(115200);

  pinMode(ENCODER0_CLK_PIN, INPUT_PULLUP);
  pinMode(ENCODER0_DT_PIN, INPUT_PULLUP);
  pinMode(ENCODER0_SW_PIN, INPUT_PULLUP);

  pinMode(ENCODER1_CLK_PIN, INPUT_PULLUP);
  pinMode(ENCODER1_DT_PIN, INPUT_PULLUP);
  pinMode(ENCODER1_SW_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENCODER0_SW_PIN), handleButton0Interrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(ENCODER1_SW_PIN), handleButton1Interrupt, FALLING);
}

void PollEncoders() {
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck >= 2) {
    // Encoder 0 - X Axis
    int currentCLK_0 = digitalRead(ENCODER0_CLK_PIN);
    if (currentCLK_0 != lastCLK_0 && currentCLK_0 == LOW) {
      int dtValue = digitalRead(ENCODER0_DT_PIN);
      if (dtValue != currentCLK_0) {
        encoder0_Pos++;
        Serial.println("Alpha: Scroll Right");
      } else {
        encoder0_Pos--;
        Serial.println("Alpha: Scroll Left");
      }
    }
    lastCLK_0 = currentCLK_0;

    // Encoder 1 - Y Axis
    int currentCLK_1 = digitalRead(ENCODER1_CLK_PIN);
    if (currentCLK_1 != lastCLK_1 && currentCLK_1 == LOW) {
      int dtValue = digitalRead(ENCODER1_DT_PIN);
      if (dtValue != currentCLK_1) {
        encoder1_Pos++;
        Serial.println("Beta: Scroll Down");
      } else {
        encoder1_Pos--;
        Serial.println("Beta: Scroll Up");
      }
    }
    lastCLK_1 = currentCLK_1;

    lastCheck = millis();
  }

  if (button0Pressed) {
    button0Pressed = false;
    Serial.println("Alpha Button Pressed (SELECT)");
    // Handle SELECT
  }

  if (button1Pressed) {
    button1Pressed = false;
    Serial.println("Beta Button Pressed (BACK)");
    // Handle BACK
  }
}


 // PollEncoders();
*/
