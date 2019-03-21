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

// Etat actuel des LED
bool LED_prev[3] = {false, false, false};

// connect buton to pin 2 and LOW
//const byte c_interruptPin = 2;
// WeMos
const int c_interruptPin = D1;
const int c_eventModeFSM = 12345;
volatile byte ButtonReleased = false;
void release_button();
Ticker buttonReleaseTicker(release_button, 800); // in milisecs
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

// LED state machine
State state_all_off(&all_off_enter,NULL,NULL);
State state_green(&green_enter,NULL,&green_exit);
State state_orange(&orange_enter,NULL,&orange_exit);
State state_red(&red_enter,NULL,&red_exit);
State state_orangered(&orangered_enter,NULL,&orangered_exit);
State state_random(&random_enter,NULL,NULL);
Fsm * led_fsm;

void sleep_fct(){
  //  TODO //  TODO //  TODO //  TODO //  TODO //  TODO // 
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

  // Serial.println("Going to sleep forever...");
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

Fsm * mode_fsm;

// mode state machine
State state_francais(&modeFr,NULL,NULL);
State state_travaux(&modeTravaux,NULL,NULL);
State state_suisse(&modeCh,NULL,NULL);
State state_random_mode(&modeRandom,NULL,NULL);

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

  mode_fsm = new Fsm(&state_francais);
  modeFr();
  mode_fsm->add_transition(&state_francais,&state_travaux,c_eventModeFSM,NULL);
  mode_fsm->add_transition(&state_travaux,&state_suisse,c_eventModeFSM,NULL);
  mode_fsm->add_transition(&state_suisse,&state_random_mode,c_eventModeFSM,NULL);
  mode_fsm->add_transition(&state_random_mode,&state_francais,c_eventModeFSM,NULL);

  ButtonReleased = false;

  //Low power configuration
  /*
  wifi_fpm_open();
  sint8 valid = wifi_fpm_do_sleep(MODEM_SLEEP_T);
  delay(200);
  Serial.print("wifi_fpm_do_sleep: ");
  */
  Serial.println("setup END");
}

void loop() {
  led_fsm->run_machine();
  mode_fsm->run_machine();
  buttonReleaseTicker.update();
}
