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
// -------- All off ----------
void all_off_enter() {
    digitalWrite(c_LED_pin[0], LOW); 
    digitalWrite(c_LED_pin[1], LOW); 
    digitalWrite(c_LED_pin[2], LOW); 
}
// ------ Random LED ----------
void random_enter() {
  for(int i = 0; i < 3; i++){
    if(random(2) >= 1){
      digitalWrite(c_LED_pin[i], HIGH); 
    }else
    {
      digitalWrite(c_LED_pin[i], LOW); 
    }
    
  }
}

// State machine
State state_all_off(&all_off_enter,NULL,NULL);
State state_green(&green_enter,NULL,&green_exit);
State state_orange(&orange_enter,NULL,&orange_exit);
State state_red(&red_enter,NULL,&red_exit);
State state_orangered(&orangered_enter,NULL,&orangered_exit);
State state_random(&random_enter,NULL,NULL);
Fsm * led_fsm;

/* Mode :
 *  - 0 Tricolore _fr
 *  - 1 Travaux
 *  - 2 Tricolore _ch
 *  - 3 Random
 */
int current_mode = 0;
int loop_cnt = 0;

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
  all_off_enter();
  delete led_fsm;
  led_fsm = new Fsm(&state_green);
  led_fsm->add_timed_transition(&state_green,  &state_orange, 5000, NULL);
  led_fsm->add_timed_transition(&state_orange, &state_red,    1000, NULL);
  led_fsm->add_timed_transition(&state_red,    &state_green,  5000, NULL);
}

void modeCh(){
  all_off_enter();
  delete led_fsm;
  led_fsm = new Fsm(&state_green);
  led_fsm->add_timed_transition(&state_green,  &state_orange,    5000, NULL);
  led_fsm->add_timed_transition(&state_orange, &state_red,       800,  NULL);
  led_fsm->add_timed_transition(&state_red,    &state_orangered, 5000,  NULL);
  led_fsm->add_timed_transition(&state_orangered, &state_green,  400, NULL);
}

void modeTravaux(){
  all_off_enter();
  delete led_fsm;
  led_fsm = new Fsm(&state_orange);
  led_fsm->add_timed_transition(&state_orange, &state_all_off, 800, NULL);
  led_fsm->add_timed_transition(&state_all_off, &state_orange, 800, NULL);
}

void modeRandom(){
  all_off_enter();
  delete led_fsm;
  led_fsm = new Fsm(&state_random);
  led_fsm->add_timed_transition(&state_random, &state_random,  1000, NULL);
}

void change_mode(){
  Serial.println("change_mode START");
  switch (current_mode){
    case 0:
      modeTravaux();
      current_mode=1;
      break;
    case 1:
      modeCh();
      current_mode=2;
      break;
    case 2:
      modeRandom();
      current_mode=3;
      break;
    case 3:
      modeFr();
      current_mode=0;
      break;
  }
  ButtonReleased = false;
  Serial.println(current_mode);
  Serial.println("change_mode END");
}

void press_button() {
  if (digitalRead(c_interruptPin) == HIGH && !ButtonReleased){
    ButtonReleased = true;
    change_mode();
    Serial.println("ButtonReleased");
  } 
  Serial.println("press_button END");
}

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

  randomSeed(analogRead(0));
  
  current_mode = 0;
  loop_cnt = 0;
  ButtonReleased = false;

  led_fsm = new Fsm(&state_green);
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
  led_fsm->run_machine();
}
