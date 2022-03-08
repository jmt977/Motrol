/*
 *  WiFi Version 
 *  endpoint IP_ADDRESS/OnOff?handle=1 (MOBILE 1)
 *  Use handle=0 to turn off all and handle=9 to turn on all
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define MaxAttempts 20


// GPIO mapping ports
byte MOBILES[]= {5,4,0,2};
// actual state of mobiles
byte actualState[]={0,0,0,0};
// parameter received by the serial port 
byte inByte=0;
// flags 
boolean configured=false;
boolean started=false;

ESP8266WebServer server(80);

void setup(){ 
  // for serial use
  Serial.begin(115200);
  while (!Serial){}
  Serial.println("Enter SSID PASS");
  // preparing digital outputs
  pinMode(MOBILES[0],OUTPUT);
  pinMode(MOBILES[1],OUTPUT);
  pinMode(MOBILES[2],OUTPUT);
  pinMode(MOBILES[3],OUTPUT);
  // reset outputs
  digitalWrite(MOBILES[0], !actualState[0]);
  digitalWrite(MOBILES[1], !actualState[1]);
  digitalWrite(MOBILES[2], !actualState[2]);
  digitalWrite(MOBILES[3], !actualState[3]);
  // reset Wifi connection 
  WiFi.disconnect();
}

void loop(void) {
  if (Serial.available() && !configured){
      String data = Serial.readStringUntil('\n');
      String ssid,pass;
      byte i = 0;
      while (i<data.length() && data[i]!=' '){
        ssid+=data[i];
        i++;
      }
      i++;
      while (i<=data.length()){
           pass+=data[i];
           i++;
      }
      if (wifiConnect(ssid, pass))
        configured=true;
  }
  else  
    if (configured && !started){
      if (initServer())
        started=true;
  }
  if (configured && started)
    server.handleClient();
}

boolean wifiConnect(String ssid, String pass){
  WiFi.mode(WIFI_STA);
  delay(100);
  WiFi.begin(ssid, pass);
  Serial.print("Connecting Wifi");
  byte attempts=0;
  while (attempts<MaxAttempts && WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    attempts++;
  }
  if (attempts<MaxAttempts){
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    return true;
  }
  else{
    Serial.println(" ");
    Serial.println("Can't connect to " + ssid + ". SSID o PASS wrong. Try again SSID PASS");
    return false;
  }
}

// this function allows mannaging four mobiles separatly
String manageMobileAlone(byte in){
  String msg;
  char buf[1];
  in-=48;
  // turn on or turn off all together
  if (in==0 || in==9){
    in==9?in=1:in=0;
    for (int i=0;i<4;i++){
      actualState[i]=in;
      digitalWrite(MOBILES[i], !actualState[i]);
    } 
  }
  // turn on mobile 1 to 4 regardless of the rest
  else if (in>=1 && in<=4){
        actualState[in-1]=1;
        digitalWrite(MOBILES[in-1], !actualState[in-1]);
  }
  // turn off mobile 1 to 4 regardless of the rest
  else if (in>=5 && in<=8){
        actualState[in-5]=0;
        digitalWrite(MOBILES[in-5], !actualState[in-5]);
  } 
  else {
    msg="WRONG COMMNAD <br><br>";
  }
  msg+="ACTUAL OUTPUTS STATE: ";
  for (int i=3;i>=0;i--){
      msg+=(actualState[i]); 
      msg+=" ";               
  }  
  return msg;
}

/////// WEB  SERVER ///////////////////////////////////////////////////////
boolean initServer(){
  server.on("/OnOff", handleMobiles);
  server.begin();
  return true;
}

void handleMobiles() {
  Serial.println("Mobiles Management");
  String header;
  String content = "<html><body><H2>Mobiles management</H2><br>";
  if (server.hasArg("handle")) {
    inByte = (server.arg("handle")).charAt(0);
    String msg=manageMobileAlone(inByte);
    Serial.println(msg);
    content +=  msg + "</body></html>";
    server.send(200, "text/html", content);
    server.sendHeader("Location", "/");
    server.send(301);
    return;
    }
}
