
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <Servo.h>

#define FIREBASE_HOST "https://smart-parking-9fc2c-default-rtdb.europe-west1.firebasedatabase.app/"
#define FIREBASE_AUTH "9DLfSC2g8UHYrHOAoBmMQsjltbvUHd63u2Cya2TF"
// ----------------------------------------------Network credentials------------------------------
//#define WIFI_SSID "TOPNET_20D8"
//#define WIFI_PASSWORD "trxnr0x9x6"
#define WIFI_SSID "ISITcom_wifi_ens"
#define WIFI_PASSWORD "@isitcom@2022@ens"

// Set GPIOs for LED and PIR Motion Sensor
const int led = 12;
const int motionSensor = 34;
int pirStat = 0; 
String sdata="";


Servo myservo;                          //servo as gate

#define  carEnter  34  
#define  PinServo  33 // entry sensor 
int LED_BUILTIN = 2;

int pos=0;   
boolean entrysensor;
int count=0;
//Define FirebaseESP32 data object
FirebaseData firebaseData;
FirebaseJson json;

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
void setup()
{

  Serial.begin(115200);

 
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  pinMode(LED_BUILTIN, OUTPUT);
  myservo.attach(PinServo);      // servo pin to D5
  pinMode(carEnter, INPUT);     // ir as input
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  
   // PIR Motion Sensor mode INPUT_PULLUP
  pinMode(motionSensor, INPUT_PULLUP);
  // Set LED to LOW
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  //Set database read timeout to 1 minute (max 15 minutes)
  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  //tiny, small, medium, large and unlimited.
  //Size and its write timeout e.g. tiny (1s), small (10s), medium (30s) and large (60s).
  Firebase.setwriteSizeLimit(firebaseData, "tiny");

  /*
  This option allows get and delete functions (PUT and DELETE HTTP requests) works for device connected behind the
  Firewall that allows only GET and POST requests.
  
  Firebase.enableClassicRequest(firebaseData, true);
  */

  //String path = "/data";
  

  Serial.println("------------------------------------");
  Serial.println("Connected...");
  
}

void loop()
{
    pirStat = digitalRead(motionSensor); 
   
    if (pirStat == HIGH) {   
       open_close_servo();
      // if motion detected
      //digitalWrite(led, HIGH);  // turn LED ON
      sdata="Car detected!!!";
      Serial.println(sdata);
      json.set("/data", sdata);
      Firebase.updateNode(firebaseData,"/Sensor",json); 
  
    } 
    else {
      //digitalWrite(led, LOW); // turn LED OFF if we have no motion
      sdata = "Motion stopped...";
      Serial.println(sdata);
      json.set("/data", sdata);
      Firebase.updateNode(firebaseData,"/Sensor",json);
     
    }
//    // Current time
//    now = millis();
//    if(startTimer){
//      Serial.println(sdata);
//  delay(100); 
//  json.set("/data", sdata);
//  Firebase.updateNode(firebaseData,"/Sensor",json); 
//    }
//    // Turn off the LED after the number of seconds defined in the timeSeconds variable
//    if(startTimer && (now - lastTrigger > (timeSeconds*1000))) {
//       sdata = "Motion stopped...";
//      Serial.println(sdata);
//       delay(100); 
//      json.set("/data", sdata);
//      Firebase.updateNode(firebaseData,"/Sensor",json);
//      digitalWrite(led, LOW);
//      startTimer = false;
//    }

   

}
