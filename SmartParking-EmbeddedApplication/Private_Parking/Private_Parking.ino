#include <dummy.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <NTPClient.h>
#include <Servo.h>

#define ligne_0 14
#define ligne_1 27
#define ligne_2 26
#define ligne_3 25 

#define colonne_0 19
#define colonne_1 21
#define colonne_2 22
#define colonne_3 23


int LEDs[] = {13,15,18,5,1,34,35};    // for ESP32 microcontroller gfedcba

int zero[] = {0, 1, 1, 1, 1, 1, 1};   // LED states to display number zero
int one[] = {0, 0, 0, 0, 1, 1, 0};   // LED states to display number one
int two[] = {1, 0, 1, 1, 0, 1, 1};  // LED states to display number two
int three[] = {1, 0, 0, 0, 1, 1, 1};   // LED states to display number three
int four[] = {1, 1, 0, 0, 1, 1, 0};   // LED states to display number four
int five[] = {1, 1, 0, 1, 1, 0, 1};   // LED states to display number five
int six[] = {1, 1, 1, 1, 1, 0, 1};   // LED states to display number six
int seven[] = {0, 0, 0, 0, 1, 1, 1};   // LED states to display number seven
int eight[] = {1, 1, 1, 1, 1, 1, 1}; // LED states to display number eight
int nine[] = {1, 0, 0, 1, 1, 1, 1}; // LED states to display number nine


#define LED 12
int LED_BUILTIN = 2;
#define  PinServo  33 // entry sensor 

Servo myservo;                          //servo as gate
int s;
int pos=0;   
boolean entrysensor;
int count=0;
int capacity=9;
// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;
// Database child nodes
String carPath = "/cars";
String capacityPath = "/capacity";
String EntryGate  = "/EntryGate";
String ExitGate   = "/ExitGate";
String timePath   = "/timestamp";

// Parent Node (to be updated in every loop)
String parentPath;

const char* ntpServer = "pool.ntp.org";

float motion;
float entryGate;
int timestamp;

//----------------------------------------------Firebase---------------------------------------------
// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"



// I Firebase project API Key
#define API_KEY "AIzaSyDQWjuCKfEFi56D8SNWu2lNXXwWtoBiSA8"

// Authorized Email and Corresponding Password
#define USER_EMAIL "sabbaghsarah1998@gmail.com"
#define USER_PASSWORD "123456789"

// RTDB URLefine the RTDB URL
#define DATABASE_URL "https://smart-parking-9fc2c-default-rtdb.europe-west1.firebasedatabase.app/"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson json;

// ----------------------------------------------Network credentials------------------------------
//#define WIFI_SSID "TOPNET_20D8"
//#define WIFI_PASSWORD "trxnr0x9x6"
//#define WIFI_SSID "ISITcom_wifi_ens"
//#define WIFI_PASSWORD "@isitcom@2022@ens"
#define WIFI_SSID "Airbox-FA5B"
#define WIFI_PASSWORD "02333510"

void setup() {

  // Initialise le port série
  Serial.begin(115200);
  
  initWiFi();
  configTime(0, 0, ntpServer);

  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/UsersData/" + uid +"/readings";
 


  
  myservo.attach(PinServo);      
  pinMode(LED, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  for (int i = 0; i<7; i++) pinMode(LEDs[i], OUTPUT);
  // --------------------------------------On configure les lignes en sorties--------------------------
  pinMode(ligne_0, OUTPUT);
  pinMode(ligne_1, OUTPUT);
  pinMode(ligne_2, OUTPUT);
  pinMode(ligne_3, OUTPUT);

  // ---------------------------On configure les colonnes en entrées avec pull-up------------------------
  pinMode(colonne_0, INPUT_PULLUP);
  pinMode(colonne_1, INPUT_PULLUP);
  pinMode(colonne_2, INPUT_PULLUP);
  pinMode(colonne_3, INPUT_PULLUP);  
  
}

void loop() {
  
    s=capacity-count;
    afficheur(s);
  int taille_du_code_secret = 4;
  char code_secret[taille_du_code_secret] = {'D', 'A', '0', '4'};
   Serial.print("Entrez votre code");
  char code_tape[taille_du_code_secret] = {0};

  for(int nbr_caracteres_tapes = 0 ; nbr_caracteres_tapes < taille_du_code_secret ; nbr_caracteres_tapes++ ) {
    code_tape[nbr_caracteres_tapes] = bouton_appuye_et_relache();
    Serial.print("*");
  }
  
  boolean code_juste = true;
  
  for(int index=0; index < taille_du_code_secret; index++) {

    // Si on a pas le même caractère à l'index
    if(code_secret[index] != code_tape[index]) {
      code_juste = false;
      break;
    }
    
  }

  // Si le code est juste
  if((code_juste == true)&& (count<capacity)) {
   
    s=capacity-count;
    afficheur(s);
    Serial.print("open\n");
    Serial.print("cars : ");
    Serial.print(count);
    open_close_servo();
     Serial.print("cars : ");
    Serial.print(count);
    Serial.print("close");
    // Send new readings to database
    if (Firebase.ready()){
   
        //Get current timestamp
        timestamp = getTime();
        Serial.print ("time: ");
        Serial.println (timestamp);
    
        parentPath= databasePath + "/" + String(timestamp);

    json.set(carPath.c_str(), String(count));
    json.set(capacityPath.c_str(), String(capacity-count));
    json.set(timePath, String(timestamp));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
  }
  }
  if (count>=capacity) {
    Serial.print("parking est complet");}
  // sinon
  else {
    digitalWrite(LED, HIGH);
    delay(3000);
    digitalWrite(LED, LOW);  
  }
}

//-------------------------------------------bouton_appuye_et_relache-----------------------------
char bouton_appuye_et_relache() {

  char bouton = NULL;
  // On attend l'appui sur un bouton
  do {
    bouton = bouton_presse();
  } while(bouton == NULL);

  delay(100);

  // On attend que l'utilisateur relache le bouton
  while(bouton_presse() != NULL);
  
  delay(100);

  // On retourne le bouton qui a été pressé
  return bouton;
  
}
//------------------------------------------bouton_presse---------------------------------------
char bouton_presse() {

  const char boutons[4][4] = {'1','2','3','A',
                              '4','5','6','B',
                              '7','8','9','C',
                              '*','0','#','D'};
                              
  for(int ligne=0;ligne<4;ligne++) {

    // On met le ligne selectionnée à LOW.
    digitalWrite(ligne_0, ligne == 0 ? LOW : HIGH);
    digitalWrite(ligne_1, ligne == 1 ? LOW : HIGH);
    digitalWrite(ligne_2, ligne == 2 ? LOW : HIGH);
    digitalWrite(ligne_3, ligne == 3 ? LOW : HIGH);    

    int etat_colonne_0 = digitalRead(colonne_0);
    int etat_colonne_1 = digitalRead(colonne_1);
    int etat_colonne_2 = digitalRead(colonne_2);
    int etat_colonne_3 = digitalRead(colonne_3);

    // Si on a la colonne 0 à LOW
    if(etat_colonne_0 == LOW) {
      return boutons[ligne][0];
    }
  
    // Si on a la colonne 1 à LOW
    if(etat_colonne_1 == LOW) {
      return boutons[ligne][1];  
    }
  
      // Si on a la colonne 2 à LOW
    if(etat_colonne_2 == LOW) {
      return boutons[ligne][2];  
    }
  
      // Si on a la colonne 3 à LOW
    if(etat_colonne_3 == LOW) {
      return boutons[ligne][3];   
    }   
    
  }

  // Si on a aucun bouton de pressé, on retourne NULL
  return NULL;
  
}
// ------------------------------------------------Initialize WiFi---------------------------------------------------------------
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}
//------------------------------- Function that gets current epoch time--------------------------------
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return(0);
  }
  time(&now);
  return now;
}
//------------------------------------- 7 segment display----------------------
void afficheur(int capacity){
  if(capacity==0){
    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], zero[i]);
    }
  else if (capacity==1){
    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], one[i]);
    }
   else if (capacity==2){
    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], two[i]);
    }
     else if (capacity==3){
    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], three[i]);
    }
     else if (capacity==4){
    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], four[i]);
    }
     else if (capacity==5){
    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], five[i]);
    }
     else if (capacity==6){
    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], six[i]);
    }
     else if (capacity==7){
    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], seven[i]);
    } else if (capacity==8){
    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], eight[i]);
    }
     else if (capacity==9){
    for (int i = 0; i<7; i++) digitalWrite(LEDs[i], nine[i]);
    }
}
// ---------------------------------------------open  and close enty Gate -------------------------------------------------------
void open_close_servo(){
       digitalWrite(LED_BUILTIN, HIGH);
       count=  count+1;
       for (pos = 0; pos <= 90; pos += 1) { // goes from 0 degrees to 180 degrees
        // in steps of 1 degree
        myservo.write(pos);              // tell servo to go to position in variable 'pos'
        delay(15);  }                     // waits 15ms for the servo to reach the position
       
      delay(3000);
      for (pos = 90; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
        myservo.write(pos);              // tell servo to go to position in variable 'pos'
        delay(15);                       // waits 15ms for the servo to reach the position
      }
      digitalWrite(LED_BUILTIN, LOW);
   }
