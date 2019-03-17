#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <Fsm.h>


// connect to LED1 to pin 3
// connect to LED2 to pin 4
// connect to LED3 to pin 5
//int c_LED_pin[3] = {3, 4, 5};

// WeMos
//  c_LED_pin[0] > GREEN
//  c_LED_pin[1] > ORANGE
//  c_LED_pin[2] > RED
const int c_LED_pin[3] = {D2, D3, D4};

/* Types d'allumage :
 *  - 0 => OFF
 *  - 1 => ON
 *  - 2 => clignote (blink)
 */
const int c_type_OFF   = 0;
const int c_type_ON    = 1;
const int c_type_BLINK = 2;

// Etat actuel des LED
bool LED_prev[3] = {false, false, false};

// connect buton to pin 2 and LOW
//const byte c_interruptPin = 2;
// WeMos
const int c_interruptPin = D1;
volatile byte ButtonReleased = false;
volatile byte WakeUp         = false;

// LED control
// ---------- GREEN ----------
void green_enter() {
    digitalWrite(c_LED_pin[0], HIGH); 
}
void green_exit() {
    digitalWrite(c_LED_pin[0], LOW); 
}
// ---------- ORANGE----------
void orange_enter() {
    digitalWrite(c_LED_pin[1], HIGH); 
}
void orange_exit() {
    digitalWrite(c_LED_pin[1], LOW); 
}
// ---------- RED  -----------
void red_enter() {
    digitalWrite(c_LED_pin[2], HIGH); 
}
void red_exit() {
    digitalWrite(c_LED_pin[2], LOW); 
}
// ------- ORANGE/RED --------
void orangered_enter() {
    digitalWrite(c_LED_pin[1], HIGH); 
    digitalWrite(c_LED_pin[2], HIGH); 
}
void orangered_exit() {
    digitalWrite(c_LED_pin[1], LOW); 
    digitalWrite(c_LED_pin[2], LOW); 
}

// State machine
State state_green(&green_enter,NULL,&green_exit);
State state_orange(&orange_enter,NULL,&orange_exit);
State state_red(&red_enter,NULL,&red_exit);
State state_orangered(&orangered_enter,NULL,&orangered_exit);
Fsm led_fsm(&state_green);

/* Mode :
 *  - 0 Tricolore _fr
 *  - 1 Travaux
 *  - 2 Tricolore _ch
 *  - 3 Random
 */
int current_mode = 0;
int loop_cnt = 0;

void press_button() {
  if (digitalRead(c_interruptPin) == HIGH && !ButtonReleased){
    ButtonReleased = true;
    loop_cnt = 0;
    Serial.println("ButtonReleased");
  } 
  Serial.println("press_button END");
}

void random_light(int Type_LED[]){
  for(int i = 0; i < 3; i++){
    Type_LED[i] = random(3);
  }
}

void sleep_fct(){
  Serial.println("sleep_fct START");
  // int asked_type[3]={c_type_ON, c_type_ON, c_type_ON};
  // // going down LED  
  // asked_type[2] = c_type_ON;
  // asked_type[1] = c_type_OFF;
  // asked_type[0] = c_type_OFF;
  // avance_allumage(asked_type);
  // asked_type[2] = c_type_OFF;
  // asked_type[1] = c_type_ON;
  // asked_type[0] = c_type_OFF;
  // avance_allumage(asked_type);
  // asked_type[2] = c_type_OFF;
  // asked_type[1] = c_type_OFF;
  // asked_type[0] = c_type_ON;
  // avance_allumage(asked_type);
  // asked_type[2] = c_type_OFF;
  // asked_type[1] = c_type_OFF;
  // asked_type[0] = c_type_OFF;
  // avance_allumage(asked_type);

  Serial.println("Going to sleep forever...");
  //system_deep_sleep_set_option(4);
  //delay(200);
  //system_deep_sleep(0);
  Serial.println("Going into deep sleep for 20 seconds");
  ESP.deepSleep(10e6); // 20e6 is 20 microseconds

  // Jamais ici
  Serial.println("sleep_fct END");
}

void modeFr(){
  led_fsm.add_timed_transition(&state_green,  &state_orange, 1000, NULL);
  led_fsm.add_timed_transition(&state_orange, &state_red,    500,  NULL);
  led_fsm.add_timed_transition(&state_red,    &state_green,  1000, NULL);
}

/*
void avance_allumage(int Type_LED[]){
  Serial.println("avance_allumage START");
  //while(!ButtonReleased){
  if(!ButtonReleased){
    for(int i = 0; i < 3; i++){
      if (LED_prev[i] == false && Type_LED[i] == c_type_ON)
      {
        // Power ON LED
        digitalWrite(c_LED_pin[i], HIGH); 
        LED_prev[i] = true;
        Serial.print("LED ");
        Serial.print(i);
        Serial.println(" > HIGH");
      }
      if (LED_prev[i] == true && Type_LED[i] == c_type_OFF)
      {
        // Power OFF LED
        digitalWrite(c_LED_pin[i], LOW); 
        LED_prev[i] = false;
        Serial.print("LED ");
        Serial.print(i);
        Serial.println(" > LOW");
      }
      if (Type_LED[i] == c_type_BLINK){
        if (LED_prev[i] == false){
          // Power ON LED
          digitalWrite(c_LED_pin[i], HIGH); 
          LED_prev[i] = true;
          Serial.print("LED ");
          Serial.print(i);
          Serial.println(" > HIGH");
        }else{
          // Power OFF LED1
          digitalWrite(c_LED_pin[i], LOW); 
          LED_prev[i] = false;
          Serial.print("LED ");
          Serial.print(i);
          Serial.println(" > LOW");
        }
      }
    }
  }  
  Serial.println("avance_allumage END");
}
*/

/*
void main_loop() {  
  int asked_type[3] = {c_type_ON, c_type_ON, c_type_ON};
  if (loop_cnt == 0 || (WakeUp && ButtonReleased)){
    Serial.println("first_loop");
    loop_cnt = 0;
    ButtonReleased = false;
    WakeUp = false;
    // Validate LED     
    asked_type[0] = c_type_ON;
    asked_type[1] = c_type_ON;
    asked_type[2] = c_type_ON;
    avance_allumage(asked_type);
    
    asked_type[0] = c_type_OFF;
    asked_type[1] = c_type_OFF;
    asked_type[2] = c_type_OFF;
    avance_allumage(asked_type);
  }else{
    if(loop_cnt>5){
      Serial.println("Goto sleep");
      sleep_fct();
    }
    if (ButtonReleased && !WakeUp){
      current_mode = (current_mode + 1) % 4;
      ButtonReleased = false;
    }
    switch (current_mode){
      case 0:
        asked_type[2] = c_type_OFF;
        asked_type[1] = c_type_OFF;
        asked_type[0] = c_type_ON;
        avance_allumage(asked_type);
        
        asked_type[2] = c_type_OFF;
        asked_type[1] = c_type_ON;
        asked_type[0] = c_type_OFF;
        avance_allumage(asked_type);
      
        asked_type[2] = c_type_ON;
        asked_type[1] = c_type_OFF;
        asked_type[0] = c_type_OFF;
        avance_allumage(asked_type);
        break;
      case 1:
        asked_type[2] = c_type_OFF;
        asked_type[1] = c_type_BLINK;
        asked_type[0] = c_type_OFF;
        avance_allumage(asked_type);
        break;      
      case 2:
        asked_type[2] = c_type_OFF;
        asked_type[1] = c_type_OFF;
        asked_type[0] = c_type_ON;
        avance_allumage(asked_type);
        
        asked_type[2] = c_type_OFF;
        asked_type[1] = c_type_ON;
        asked_type[0] = c_type_OFF;
        avance_allumage(asked_type);
      
        asked_type[2] = c_type_ON;
        asked_type[0] = c_type_OFF;
        asked_type[1] = c_type_OFF;
        avance_allumage(asked_type);
        
        asked_type[2] = c_type_ON;
        asked_type[1] = c_type_ON;
        asked_type[0] = c_type_OFF;
        avance_allumage(asked_type);
        break;
      case 3:
        random_light(asked_type);
        avance_allumage(asked_type);
        break;
    }
  }
  loop_cnt = loop_cnt + 1;
  
  Serial.println("main_loop END");
}
*/

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(2000);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("   ");
  Serial.println("Hello World");

  pinMode(c_LED_pin[0], OUTPUT);
  pinMode(c_LED_pin[1], OUTPUT);
  pinMode(c_LED_pin[2], OUTPUT);

  pinMode(c_interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(c_interruptPin), press_button, CHANGE);
  // pinMode(13, OUTPUT);
  // digitalWrite(13, LOW);

  randomSeed(analogRead(0));
  
  current_mode = 0;
  loop_cnt = 0;
  ButtonReleased = false;

  //Low power configuration
  /*
  wifi_fpm_open();
  sint8 valid = wifi_fpm_do_sleep(MODEM_SLEEP_T);
  delay(200);
  Serial.print("wifi_fpm_do_sleep: ");
  */

  //Ticker timecontrol = 
  modeFr();
  Serial.println("setup END");

}

void loop() {
  led_fsm.run_machine();
}
