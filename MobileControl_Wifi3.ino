/*
 *  Version WiFi
 *  Cómo acceder a diferentes opciones
 *  Para manejar las salidas: IP_ADDRESS/OnOff?handle=1 (MOBILE 1)
 *  Para prender todos handle=0 y apagar todos handle=9
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define MaxAttempts 20


// sin uso porque no recibe un String el digitalWrite
/*#define MOBILE1 5 // es el GPIO15 la salida D1 
#define MOBILE2 4 // es el GPIO13 la salida D2 
#define MOBILE3 0 // es el GPIO12 la salida D3
#define MOBILE4 2 // es el GPIO14 la salida D4*/

// usado para mapear los GPIO
byte MOBILES[]= {5,4,0,2};

byte actualState[]={0,0,0,0};
byte inByte=0;
boolean configured=false;
boolean started=false;
//String mobile="MOBILE";

ESP8266WebServer server(80);

void setup(){ 
  // for serial use
  Serial.begin(115200);
  while (!Serial){}
  Serial.println("Ingrese SSID PASS");
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
  WiFi.disconnect();// sino queda el ultimo usado en la flash
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
  WiFi.mode(WIFI_STA);//Cambiar modo del Wi-Fi a Station y no AP
  delay(100);
  WiFi.begin(ssid, pass); // si no se hace el disconnect del setup toma la ultima config de la flash
  Serial.print("Conectando a Wifi");
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
    Serial.println("No se pudo conectar a la red " + ssid + ". SSID o PASS erróneos. Vuelva a ingresar SSID PASS");
    return false;
  }
}

// esta funcion permite manejar los 4 moviles de manera independiente
String manageMobileAlone(byte in){
  String msg;
  char buf[1];
  in-=48;
  // apagar o prender todos juntos
  if (in==0 || in==9){
    in==9?in=1:in=0;
    for (int i=0;i<4;i++){
      actualState[i]=in;
      digitalWrite(MOBILES[i], !actualState[i]);
    } 
  }
  // prender 1 movil independientemente del resto
  else if (in>=1 && in<=4){
        actualState[in-1]=1;
        digitalWrite(MOBILES[in-1], !actualState[in-1]);
  }
  // apagar 1 movil independientemente del resto
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

/////// SERVIDOR WEB ///////////////////////////////////////////////////////
boolean initServer(){
  server.on("/OnOff", handleMobiles);
  server.begin();
  return true;
}

//root page can be accessed only if authentication is ok
void handleMobiles() {
  Serial.println("Mobiles Management");
  String header;
  String content = "<html><body><H2>Mobiles management</H2><br>";
  if (server.hasArg("handle")) {
    // 192.168.0.117/OnOff?handle=1
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
