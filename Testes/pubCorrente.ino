#include <ESP8266WiFi.h>
#include <PubSubClient.h> 
 
const char* ssid = "TrocoSenhaPorCerveja"; //Aqui o nome da sua rede local wi fi
const char* password =  "renatobike"; // Aqui a senha da sua rede local wi fi
const char* mqttServer = "soldier.cloudmqtt.com"; // Aqui o endereço do seu servidor fornecido pelo site 
const int mqttPort = 10622 // Aqui mude para sua porta fornecida pelo site
const char* mqttUser = "ubnuccyu"; //  Aqui o nome de usuario fornecido pelo site
const char* mqttPassword = "aVnZYSCwhNmA"; //  Aqui sua senha fornecida pelo site
char EstadoSaida = '0';  
WiFiClient espClient;
PubSubClient client(espClient);
#define mqtt_topico_pub "/Corrente"

/*Usar essa função para uma futura implementação do código
 *quando ele começar a fazer subscribe no tópico
 * 
void mqtt_callback(char* topic, byte* payload, unsigned int length);
*/

void publicaComando() {
  if (!client.connected()) {
    Serial.println("MQTT desconectado! Tentando reconectar...");
    reconectar();
  }
  
  client.loop();
  
  //Publicando no MQTT
  Serial.println("Fazendo a publicacao...");
  client.publish(mqtt_topico_pub,"String com informação do consumo" );
}


void setup() {
 
  Serial.begin(115200);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
 
  client.setServer(mqttServer, mqttPort);
   
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
   
}

/*
void callback(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);

       String msg;
    
 
    //obtem a string do payload recebido
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
    }
  

        
    //toma ação dependendo da string recebida:
    //verifica se deve colocar nivel alto de tensão na saída.
    //IMPORTANTE: o Led já contido na placa é acionado com lógica invertida (ou seja,
    //enviar HIGH para o output faz o Led apagar / enviar LOW faz o Led acender)
 
 
    //verifica se deve colocar nivel alto de tensão na saída se enviar L e digito, ou nivel baixo se enviar D e digito no topíco LED

    
    

       if (msg.equals("L1"))
    {
        digitalWrite(D1, HIGH);
        EstadoSaida = '0';
    }
    if (msg.equals("D1"))
    {
        digitalWrite(D1, LOW);
        EstadoSaida = '1';
    }
         if (msg.equals("L2"))
    {
        digitalWrite(D2, HIGH);
        EstadoSaida = '0';
    }
    if (msg.equals("D2"))
    {
        digitalWrite(D2, LOW);
        EstadoSaida = '1';
    }

        if (msg.equals("L3"))
    {
        digitalWrite(D3, HIGH);
        EstadoSaida = '0';
    }
    if (msg.equals("D3"))
    {
        digitalWrite(D3, LOW);
        EstadoSaida = '1';
    }

        if (msg.equals("L4"))
    {
        digitalWrite(D4, HIGH);
        EstadoSaida = '0';
    }
    if (msg.equals("D4"))
    {
        digitalWrite(D4, LOW);
        EstadoSaida = '1';
    }

        if (msg.equals("L5"))
    {
        digitalWrite(D5, HIGH);
        EstadoSaida = '0';
    }
    if (msg.equals("D5"))
    {
        digitalWrite(D5, LOW);
        EstadoSaida = '1';
    }
        if (msg.equals("L6"))
    {
        digitalWrite(D6, HIGH);
        EstadoSaida = '0';
    }
    if (msg.equals("D6"))
    {
        digitalWrite(D6, LOW);
        EstadoSaida = '1';
    }
          if (msg.equals("L7"))
    {
        digitalWrite(D7, HIGH);
        EstadoSaida = '0';
    }
    if (msg.equals("D7"))
    {
        digitalWrite(D7, LOW);
        EstadoSaida = '1';
    }

     if (msg.equals("L8"))
    {
        digitalWrite(D0, HIGH);
        EstadoSaida = '0';
    }
    if (msg.equals("D8"))
    {
        digitalWrite(D0, LOW);
        EstadoSaida = '1';
  
   }

        // Liga todos os leds se enviar LT no topico LED
        
        if (msg.equals("LT"))
    {
        digitalWrite(D0,HIGH);
        digitalWrite(D1,HIGH);
        digitalWrite(D2,HIGH);
        digitalWrite(D3,HIGH);
        digitalWrite(D4,HIGH);
        digitalWrite(D5,HIGH);
        digitalWrite(D6,HIGH);
        digitalWrite(D7,HIGH);
        digitalWrite(D8,HIGH);

    }
        // Desliga todos os leds se enviar DT no topico LED

        if (msg.equals("DT"))
    {
        digitalWrite(D0,LOW);
        digitalWrite(D1,LOW);
        digitalWrite(D2,LOW);
        digitalWrite(D3,LOW);
        digitalWrite(D4,LOW);
        digitalWrite(D5,LOW);
        digitalWrite(D6,LOW);
        digitalWrite(D7,LOW);
        digitalWrite(D8,LOW);
        delay(1000);

       }
     }
   
 
  Serial.println();
  Serial.println("-----------------------");
 
}
*/


void loop() {
  //Colocar a lógica do programa consumo.ino 
  publicaComando();
}
