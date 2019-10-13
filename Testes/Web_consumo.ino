#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
//Lendo corrente
#define tensao 110
const int sensorIn = A0;
int mVperAmp = 100;
double Vpp = 0;
double Vp = 0;
double Vrms = 0;
double Irms = 0;
double consumo;
unsigned long d_tempo = 0;
//Lendo corrente

//Conexão WiFi
const char* ssid = "TrocoSenhaPorCerveja";
const char* password = "renatobike";
//Conexão WiFi

ESP8266WebServer server(80);

const int led = 2;

// Atualiza Página
void handleRoot() {
  digitalWrite(led, 1);

  String textoHTML;

  textoHTML = "<p>Ola!! Aqui &eacute; o <b>ESP8266</b> falando!</p> ";
  textoHTML += "<p>Corente: "; textoHTML += getIrms(); textoHTML += "A. </p>";
  textoHTML += "<p>consumo: "; textoHTML += consumo_kWh(); textoHTML += "kWh. </p>";
  

   
  server.send(200, "text/html", textoHTML);
  digitalWrite(led, 0);
}
// Atualiza Página

// Se não acha Página
void handleNotFound(){
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}
// Se não acha Página


void setup(void){
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();
}

double getVPP()
{
  double result;

    int readValue;
    int maxValue = 0;
    int minValue = 1024;

    uint32_t start_time = millis();
    while((millis()-start_time) < 1000)
    {
      readValue = analogRead(sensorIn);
      if(readValue > maxValue)
      {
        maxValue = readValue;
      }
      if(readValue < minValue)
      {
        minValue = readValue;
      }
    }
    result = ((maxValue - minValue)* 5.0)/1024.0;
    return result;
}

double getIrms()
{
  Vpp = getVPP();
  Vp = Vpp/2.0;
  Vrms = Vp*0.707;
  Irms = ((Vrms*1000)/mVperAmp) - 0.09;
  if(Irms == 0.3)
  {
    return 0.0;
  }
  return Irms;
}

double consumo_kWh()
{
 if(millis()- d_tempo > 1000){
    consumo += (getIrms()*tensao)/3600000; //CONSUMO PELA VARIAÇÃO DE UM SEGUNDO. CONVERSÃO PARA kWh
    d_tempo = millis(); 
  }
  return consumo; 
}  
