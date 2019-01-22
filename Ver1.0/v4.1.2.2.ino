#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <WiFiManager.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include<FirebaseArduino.h>
#include <time.h>

#define FIREBASE_HOST "ip-based-switching-ir.firebaseio.com"           //Your Firebase Project URL 
#define FIREBASE_AUTH "tSxkGo5UFXiTkxCQJ1fl1uqZNLBOz2woVixSsCeP"       //Your Firebase Database Secret 

uint16_t RECV_PIN = 14;     // IR Receiver Pin --connected to D5
int Relay1_Pin=5;           // Relay 1 connected to D1
int Relay2_Pin=4;           // Relay 1 connected to D2
uint16_t qwerty=0;          // Store DEC value of the IR Remote
int led=0;                  //Led ON=Wifi Connecting..  Led OFF=Connected ---connected to D3
int num=0;                  //No. of Devices Connected
int timezone = 5.5 * 3600;  //For INDIA
int dst = 1800;
int hr,mins,sec,r=0;
long t1;

ESP8266WiFiMulti wifiMulti;
WiFiManager wifiManager;
WiFiClientSecure client;
IRrecv irrecv(RECV_PIN);
decode_results results;

void setup() {
  Serial.begin(115200);
  pinMode(Relay1_Pin,OUTPUT);
  pinMode(Relay2_Pin,OUTPUT);
  pinMode(led,OUTPUT);
  digitalWrite(led,HIGH);
  irrecv.enableIRIn();  // Start the receiver
  Serial.println('\n');
  //wifi_connect();       //Connect to Wifi(Hardcoded Credential)
  fast_wifi_connect();    //Connect to Wifi(on time of Installation)
  Firebase.begin(FIREBASE_HOST,FIREBASE_AUTH);      //Firebase Authentication
  configTime(timezone, dst, "pool.ntp.org","time.nist.gov");
  Serial.println("\nWaiting for Internet time");

  while(!time(nullptr)){
     Serial.print("*");
     delay(1000);
  }
  Serial.println("\nTime response....OK");
  for(int i=0;i<=2;i++){
    time_t now = time(nullptr);
    struct tm* p_tm = localtime(&now);
    hr=p_tm->tm_hour;
    mins=p_tm->tm_min;
    sec=p_tm->tm_sec;
    Serial.print(hr);
    Serial.print(":");
    Serial.print(mins);
    Serial.print(":");
    Serial.println(sec);
    delay(1000);
  }
/*  
  //**Initialising Firebase empty JSON**
  int number=2;                                    //number of device want to connect
  Firebase.setInt("Number",number);
  firebase_init(number);      
*/  
}

void loop(){
  int ct=millis();
  t1=(hr*3600000)+(mins*60000)+ct;//+(sec*1000);
  firebase_conect();
  //wifi_connect();
  IR_action();                    //Actions performed with IR
  server_action();                //Actions performed with Firebase
  
}

void firebase_conect(){
  if (Firebase.failed()){
    Serial.print("Firebase Connection Failed.");
    Serial.println(Firebase.error());
    fast_wifi_connect();
    //firebasereconnect();                    //Firebase Server Reconnect
    ESP.restart();                            //ESP Reset
  }
}

void fast_wifi_connect(){
  WiFiManager wifiManager;
  //wifiManager.resetSettings();                   //Testing.
  wifiManager.autoConnect("KaranESP", "1!2@3#4$5%");
  Serial.println("Connected...");
  digitalWrite(led,LOW);
  delay(100);
}


void wifi_connect(){
  wifiMulti.addAP("Karan", "karan12341");
  wifiMulti.addAP("Liberin-Technologies", "l!ber!ntech");
  wifiMulti.addAP("Liberin", "11223344551");
  
  Serial.println("Connecting...");
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(250);
    digitalWrite(led,HIGH);
    delay(100);
    Serial.print('.');
  }
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  Serial.println();
  digitalWrite(led,LOW);
  delay(100);
}

void firebasereconnect(){
  Serial.println("Trying to Reconnect.");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void dump(decode_results *results) {
  uint16_t count = results->rawlen;
  if (results->decode_type == UNKNOWN) {
    //Serial.println("Unknown encoding: ");
  } else if (results->decode_type == NEC) {
    Serial.print("Decoded NEC: ");
  } else if (results->decode_type == SONY) {
    Serial.print("Decoded SONY: ");
  } else if (results->decode_type == RC5) {
    Serial.print("Decoded RC5: ");
  } else if (results->decode_type == RC5X) {
    Serial.print("Decoded RC5X: ");
  } else if (results->decode_type == RC6) {
    Serial.print("Decoded RC6: ");
  } else if (results->decode_type == RCMM) {
    Serial.print("Decoded RCMM: ");
  } else if (results->decode_type == PANASONIC) {
    Serial.print("Decoded PANASONIC - Address: ");
    Serial.print(results->address, HEX);
    Serial.print(" Value: ");
  } else if (results->decode_type == LG) {
    Serial.print("Decoded LG: ");
  } else if (results->decode_type == JVC) {
    Serial.print("Decoded JVC: ");
  } else if (results->decode_type == AIWA_RC_T501) {
    Serial.print("Decoded AIWA RC T501: ");
  } else if (results->decode_type == WHYNTER) {
    Serial.print("Decoded Whynter: ");
  }
  for (uint16_t i = 1; i < count; i++) {
    if (i % 100 == 0)
      yield();  // Preemptive yield every 100th entry to feed the WDT.
    if (i & 1) {
      //Serial.print(results->rawbuf[i] * RAWTICK, DEC);
    } 
    else {}
  }
}

void IR_action(){
  
  if (irrecv.decode(&results)) {
    qwerty=results.value;
    dump(&results);
    delay(100);
    switch(qwerty){
      case 64000: digitalWrite(Relay1_Pin,HIGH);
                  delay(100);
                  Serial.println("Relay 1 ON");
                  Firebase.setInt("Appliance/Relay1/Action",1);
                  Firebase.setInt("Appliance/Relay1/Ack",1);
                  break;
      case 13228: digitalWrite(Relay1_Pin,LOW);
                  delay(100);
                  Serial.println("Relay 1 OFF");
                  Firebase.setInt("Appliance/Relay1/Action",0);
                  Firebase.setInt("Appliance/Relay1/Ack",0);
                  break;
      case 59780: digitalWrite(Relay2_Pin,HIGH);
                  delay(100);
                  Serial.println("Relay 2 ON");
                  Firebase.setInt("Appliance/Relay2/Action",1);
                  Firebase.setInt("Appliance/Relay2/Ack",1);
                  break;
      case 58600: digitalWrite(Relay2_Pin,LOW);
                  delay(100);
                  Serial.println("Relay 2 OFF");
                  Firebase.setInt("Appliance/Relay2/Action",0);
                  Firebase.setInt("Appliance/Relay2/Ack",0);
                  break;    
      default:  Serial.println("WRONG INPUT.");  
                delay(100);
        
    }
    irrecv.resume();  // Receive the next value
    delay(100);
  }
}

void server_action(){
  num=Firebase.getInt("Number");
  int action[num],ack[num],hr1[num],mins1[num];
  long hex[num];
  
/*  
  //**Copy Data of Firebase to Controller**
  for(int i=0;i<num;i++){
    String p=String(i+1);
    action[i]=Firebase.getInt(String("Appliance/Relay"+p+"/Action"));
    ack[i]=Firebase.getInt(String("Appliance/Relay"+p+"/Ack"));
    hex[i]=Firebase.getInt(String("Appliance/Relay"+p+"/Hex"));
    hr1[i]=Firebase.getInt(String("Appliance/Relay"+p+"/Timer/Hr"));
    mins1[i]=Firebase.getInt(String("Appliance/Relay"+p+"/Timer/Min"));
  }
*/
  //**HEX ON/OFF**
  for(int i=0;i<num;i++){
    String p=String(i+1);
    hex[i]=Firebase.getInt(String("Appliance/Relay"+p+"/Hex"));
    long h=ir_relay_switch(i,hex[i]);
    String o=String(i+1);
    Firebase.setInt(String("Appliance/Relay"+o+"/Hex"),h);
    //Serial.println("HEX Operation Performed.");
  }
  
  //**Action&Ack ON/OFF**
  for(int i=0;i<num;i++){
    String p=String(i+1);
    action[i]=Firebase.getInt(String("Appliance/Relay"+p+"/Action"));
    ack[i]=Firebase.getInt(String("Appliance/Relay"+p+"/Ack"));
    if(action[i]==1 && ack[i]==0){
      relay_switch(i,action[i]);
    } else if((action[i]==0 && ack[i]==1)||(action[i]==1 && ack[i]==1)){
        relay_switch(i,action[i]);                 //digitalWrite(Relay1_Pin,LOW);
      } else if(action[i]==0 && ack[i]==0){
            //Serial.println("Nothing to do.");
            delay(100);
        }
  }

  //**Timer ON/OFF**
  for(int j=0;j<num;j++){
    String p=String(j+1);
    hr1[j]=Firebase.getInt(String("Appliance/Relay"+p+"/Timer/Hr"));
    mins1[j]=Firebase.getInt(String("Appliance/Relay"+p+"/Timer/Min"));
  }
 
  for(int i=0;i<num;i++){
    if(hr1[i]!=-1){  
      
      sched_time(i,hr1[i],mins1[i],ack[i]);
    }
  }
  
}

void firebase_init(int number){
  for(int q=1;q<=number;q++){
    String w=String(q);
    Firebase.setInt(String("Appliance/Relay"+w+"/Pin"),q);
    Firebase.setInt(String("Appliance/Relay"+w+"/Action"),0);
    Firebase.setInt(String("Appliance/Relay"+w+"/Ack"),0);
    Firebase.setInt(String("Appliance/Relay"+w+"/Hex"),0);
    Firebase.setInt(String("Appliance/Relay"+w+"/Timer/Hr"),-1);
    Firebase.setInt(String("Appliance/Relay"+w+"/Timer/Min"),0);
  }
  Serial.println("Firebase JSON Initialization Done.\nCheck your Firebase Console\n");
  delay(100);
}

void relay_switch(int z,int x){
  String p=String(z+1);
  if(z==0){
    if(x==0){
      digitalWrite(Relay1_Pin,LOW);
      delay(100);
      Serial.println(String("Relay "+p+" OFF"));
      delay(100);
      Firebase.setInt(String("Appliance/Relay"+p+"/Ack"),0);
      delay(100);
    }else if(x==1){
      digitalWrite(Relay1_Pin,HIGH);
      delay(100);
      Serial.println(String("Relay "+p+" ON"));
      delay(100);
      Firebase.setInt(String("Appliance/Relay"+p+"/Ack"),1);
      delay(100);
    }
  }else if(z=1){
    if(x==0){
      digitalWrite(Relay2_Pin,LOW);
      delay(100);
      Serial.println(String("Relay "+p+" OFF"));
      delay(100);
      Firebase.setInt(String("Appliance/Relay"+p+"/Ack"),0);
      delay(100);
    }else if(x==1){
      digitalWrite(Relay2_Pin,HIGH);
      delay(100);
      Serial.println(String("Relay "+p+" ON"));
      delay(100);
      Firebase.setInt(String("Appliance/Relay"+p+"/Ack"),1);
      delay(100);
    }
  }
}

long ir_relay_switch(int v,long c){
  long h=1;
  //Serial.println(v);
  //Serial.println(c);
  if(v==0){
    switch(c){
      case 64000: digitalWrite(Relay1_Pin,HIGH);
                  delay(100);
                  Serial.println("Relay 1 ON");
                  delay(100);
                  Firebase.setInt("Appliance/Relay1/Action",1);
                  Firebase.setInt("Appliance/Relay1/Ack",1);
                  delay(100);
                  h=0;
                  break;
      case 13228: digitalWrite(Relay1_Pin,LOW);
                  delay(100);
                  Serial.println("Relay 1 OFF");
                  delay(100);
                  Firebase.setInt("Appliance/Relay1/Action",0);
                  Firebase.setInt("Appliance/Relay1/Ack",0);
                  delay(100);
                  h=0;
                  break;
      default:  h=c;  
                delay(100);
                  
    }
  }else if(v==1){
    switch(c){
      case 59780: digitalWrite(Relay2_Pin,HIGH);
                  delay(100);
                  Serial.println("Relay 2 ON");
                  delay(100);
                  Firebase.setInt("Appliance/Relay2/Action",1);
                  Firebase.setInt("Appliance/Relay2/Ack",1);
                  delay(100);
                  h=0;
                  break;
      case 58600: digitalWrite(Relay2_Pin,LOW);
                  delay(100);
                  Serial.println("Relay 2 OFF");
                  delay(100);
                  Firebase.setInt("Appliance/Relay2/Action",0);
                  Firebase.setInt("Appliance/Relay2/Ack",0);
                  delay(100);
                  h=0;
                  break;
      default:  h=c;  
                delay(100);
        
    }
  }
  
  //Serial.println(h);
  return h;
}

void sched_time(int u,int i2,int j2, int l){
  /*
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  
  //**DATE**
  Serial.print(p_tm->tm_mday);
  Serial.print("/");
  Serial.print(p_tm->tm_mon +1);
  Serial.print("/");
  Serial.print(p_tm->tm_year +1900);
  Serial.print(" ");
  
  int i1=p_tm->tm_hour;
  int j1=p_tm->tm_min;
  int k1=p_tm->tm_sec;
  int m=!l;
  for(int i=0;i<=u;i++){
    if(i==u){
      if (i2==i1 && j2==j1 && k1<5){
        Serial.print(i1);
        Serial.print(":");
        Serial.print(j1);
        Serial.print(":");
        Serial.println(k1);
        delay(100);
        Firebase.setInt(String("Appliance/Relay"+String(i+1)+"/Action"),m);
        delay(100);
        relay_switch(i,m);
      }
    }
  }
  */
  int m=!l;
  long t2=(i2*3600000)+(j2*60000);
  
  long diff=t2-t1;
  //Serial.println(t1);
  //Serial.println(t2);
  //Serial.println(diff);
  
  //for(int i=0;i<=u;i++){
    if(diff<=1000 && diff>=-4500){
      Firebase.setInt(String("Appliance/Relay"+String(u+1)+"/Action"),m);
      delay(100);
      relay_switch(u,m);
      //Serial.print("Mission Completed");
      //t3=t2;
      //r=1;
    }
  //}
}
