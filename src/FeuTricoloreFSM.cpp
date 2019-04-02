#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <Fsm.h>

// WeMos
//  c_LED_pin[0] > GREEN
//  c_LED_pin[1] > ORANGE
//  c_LED_pin[2] > RED
const int c_LED_pin[3] = {D2, D3, D4};

// Etat actuel des LED
bool LED_prev[3] = {false, false, false};

// connect buton to pin 2 and LOW
//const byte c_interruptPin = 2;
// WeMos
const int c_interruptPin = D1;
const int c_eventModeFSM = 12345;
const int c_eventModeFSM_toSleep = 98765;

volatile byte ButtonReleased = false;
void release_button();
Ticker buttonReleaseTicker(release_button, 200); // in milisecs
volatile byte WakeUp         = false;

// Finite State machine
// Mode = Transitions between FR/CH/RAND/WARNING
Fsm * mode_fsm;
// LED  = Transitions for each LED
Fsm * led_fsm;

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
// -------- All off ----------
void all_on_enter() {
    digitalWrite(c_LED_pin[0], HIGH); 
    digitalWrite(c_LED_pin[1], HIGH); 
    digitalWrite(c_LED_pin[2], HIGH); 
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
// ---- Virtual Button pressed --------
void virtual_button_pressed() {
  Serial.println("virtual_button_pressed");
  ButtonReleased = true;
  mode_fsm->trigger(c_eventModeFSM);    
  buttonReleaseTicker.start();
}
// DeepSleep function
void sleep_fct(){ 
  Serial.println("sleep_fct START");
  Serial.println("Going to sleep forever...");
  
  system_deep_sleep_set_option(4);
  
  //system_deep_sleep(0);
  Serial.println("Going into deep sleep for 20 seconds");
  ESP.deepSleep(10e6); // 20e6 is 20 microseconds

  // Never here
  Serial.println("sleep_fct END");
}

// LED state machine
State state_all_off(&all_off_enter,NULL,NULL);
State state_all_on(&all_on_enter,NULL,NULL);
State state_green(&green_enter,NULL,&green_exit);
State state_orange(&orange_enter,NULL,&orange_exit);
State state_red(&red_enter,NULL,&red_exit);
State state_orangered(&orangered_enter,NULL,&orangered_exit);
State state_random(&random_enter,NULL,NULL);
State state_virtual_button(&virtual_button_pressed,NULL,NULL);
State state_shutdown(&sleep_fct,NULL,NULL);

void START_mode(){
  Serial.println("START_mode START");
  all_on_enter();
  delete led_fsm;
  led_fsm = new Fsm(&state_all_on);
  led_fsm->add_timed_transition(&state_all_on, &state_red,    800, NULL);
  led_fsm->add_timed_transition(&state_red,    &state_orange, 200, NULL);
  led_fsm->add_timed_transition(&state_orange, &state_green,  200, NULL);
  led_fsm->add_timed_transition(&state_green,  &state_virtual_button,  200, NULL);
  Serial.println("START_mode END");
}
void SHUTDOWN_mode(){
  Serial.println("SHUTDOWN_mode START");
  all_off_enter();
  delete led_fsm;
  led_fsm = new Fsm(&state_red);
  led_fsm->add_timed_transition(&state_red,    &state_orange, 200, NULL);
  led_fsm->add_timed_transition(&state_orange, &state_green,  200, NULL);
  led_fsm->add_timed_transition(&state_green,  &state_shutdown,  200, NULL);
  Serial.println("SHUTDOWN_mode END");
}
void FR_mode(){
  Serial.println("FR_mode START");
  all_off_enter();
  //delete led_fsm;
  led_fsm = new Fsm(&state_green);
  led_fsm->add_timed_transition(&state_green,  &state_orange, 5000, NULL);
  led_fsm->add_timed_transition(&state_orange, &state_red,    1000, NULL);
  led_fsm->add_timed_transition(&state_red,    &state_green,  5000, NULL);
  Serial.println("FR_mode END");
}
void CH_mode(){
  Serial.println("CH_mode START");
  all_off_enter();
  delete led_fsm;
  led_fsm = new Fsm(&state_green);
  led_fsm->add_timed_transition(&state_green,  &state_orange,    5000, NULL);
  led_fsm->add_timed_transition(&state_orange, &state_red,       800,  NULL);
  led_fsm->add_timed_transition(&state_red,    &state_orangered, 5000,  NULL);
  led_fsm->add_timed_transition(&state_orangered, &state_green,  400, NULL);
  Serial.println("CH_mode END");
}
void WARNING_mode(){
  Serial.println("WARNING_mode START");
  all_off_enter();
  delete led_fsm;
  led_fsm = new Fsm(&state_orange);
  led_fsm->add_timed_transition(&state_orange,  &state_all_off, 800, NULL);
  led_fsm->add_timed_transition(&state_all_off, &state_orange, 800, NULL);
  Serial.println("WARNING_mode END");
}
void RANDOM_mode(){
  Serial.println("RANDOM_mode START");
  all_off_enter();
  delete led_fsm;
  led_fsm = new Fsm(&state_random);
  led_fsm->add_timed_transition(&state_random, &state_random,  1000, NULL);
  Serial.println("RANDOM_mode END");
}

// mode state machine
State START_sate(&START_mode,NULL,NULL);
State FR_state(&FR_mode,NULL,NULL);
State CH_state(&CH_mode,NULL,NULL);
State WARNING_state(&WARNING_mode,NULL,NULL);
State RAND_state(&RANDOM_mode,NULL,NULL);
State SHUTDOWN_state(&SHUTDOWN_mode,NULL,NULL);

void press_button() {
  if (digitalRead(c_interruptPin) == HIGH && !ButtonReleased){
    ButtonReleased = true;
    mode_fsm->trigger(c_eventModeFSM);    
    buttonReleaseTicker.start();
    Serial.println("ButtonReleased");
  } 
  Serial.println("press_button END");
}

void release_button() {
  Serial.println("release_button START");
  ButtonReleased = false;
  buttonReleaseTicker.stop();
  Serial.println("release_button END");
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

  led_fsm  = new Fsm(&state_all_on);
  mode_fsm = new Fsm(&START_sate);

  mode_fsm->add_transition(&START_sate,   &FR_state,     c_eventModeFSM,NULL);
  mode_fsm->add_transition(&FR_state,     &WARNING_state,c_eventModeFSM,NULL);
  mode_fsm->add_transition(&WARNING_state,&CH_state,     c_eventModeFSM,NULL);
  mode_fsm->add_transition(&CH_state,     &RAND_state,   c_eventModeFSM,NULL);
  mode_fsm->add_transition(&RAND_state,   &FR_state,     c_eventModeFSM,NULL);

  // Transition to DeepSleed
  mode_fsm->add_transition(&FR_state,     &SHUTDOWN_state,     c_eventModeFSM_toSleep,NULL);
  mode_fsm->add_transition(&WARNING_state,&SHUTDOWN_state,     c_eventModeFSM_toSleep,NULL);
  mode_fsm->add_transition(&CH_state,     &SHUTDOWN_state,     c_eventModeFSM_toSleep,NULL);
  mode_fsm->add_transition(&RAND_state,   &SHUTDOWN_state,     c_eventModeFSM_toSleep,NULL);

  ButtonReleased = false;

  //Low power configuration
  /*
  wifi_fpm_open();
  wifi_fpm_do_sleep(MODEM_SLEEP_T);
  Serial.print("wifi_fpm_do_sleep: ");
  */

  Serial.println("setup END");
}

void loop() {
  led_fsm->run_machine();
  mode_fsm->run_machine();
  buttonReleaseTicker.update();
}
