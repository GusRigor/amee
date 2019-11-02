#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#define tensao 110

const char* ssid = "TrocoSenhaPorCerveja";
const char* password =  "renatobike";
const char* mqttServer = "soldier.cloudmqtt.com";
const int mqttPort = 10622;
const char* mqttUser = "ubnuccyu";
const char* mqttPassword = "aVnZYSCwhNmA";


//Variáveis para a leitura da corrente
const int sensorIn = A0;
int mVperAmp = 100;
double Vpp = 0;
double Vp = 0;
double Vrms = 0;
double Irms = 0;
double consumo = 0.00001;
unsigned long d_tempo = 0;
double getIrms();
double getVPP();

char valueStr[15];
String strtemp = "";
//Variáveis para a leitura da corrente

//Funções para leitura da corrente
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
  if(Irms <= 0.3)
  {
    return 0.0;
  }
  return Irms;
}
//Funções para leitura da corrente



 
WiFiClient espClient;
PubSubClient client(espClient);
 
void setup() {
 
  Serial.begin(115200);
  pinMode(2, OUTPUT); 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
 
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
 
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {
 
      Serial.println("connected");  
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
  if(millis() - d_tempo > 1000){
    consumo += (getIrms()*tensao)/3600; //CONSUMO PELA VARIAÇÃO DE 5 SEGUNDOS. CONVERSÃO PARA Wh
    strtemp = String(consumo, 1);
    strtemp.toCharArray(valueStr, 15);
    Serial.println("Enviando para MQTT..");  
    d_tempo = millis();
    client.publish("Corrente/Sensor2", valueStr);
  }
  
  client.subscribe("Corrente/rele2");
 
}
 
void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String strMensagem = String((char*)payload);
  strMensagem.toLowerCase();
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  
  if(strMensagem == "liga"){
    Serial.println("Colocando o pino em stado ALTO...");
    digitalWrite(2, HIGH); //Pino D4=2
  }
  if(strMensagem == "desliga"){
    Serial.println( "Colocando o pino em stado BAIXO...");
    digitalWrite(2, LOW); //Pino D4=2
  }
   
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
 
  Serial.println();
  Serial.println("-----------------------");
 
}
 
void loop() {
  client.loop();
  if(millis() - d_tempo > 1000){
    consumo = getIrms(); //CONSUMO PELA VARIAÇÃO DE 5 SEGUNDOS. CONVERSÃO PARA Wh
    strtemp = String(consumo, 1);
    strtemp.toCharArray(valueStr, 15);
    Serial.print("Enviando para MQTT.. Corrente em Amper: ");
    Serial.println(valueStr);
    d_tempo = millis();
    client.publish("Corrente/Sensor2", valueStr);
  }
}
