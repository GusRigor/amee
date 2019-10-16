////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Arquivo:   MQTT_WiFiManager_PushButton.ino
//Tipo:      Exemplo de uso das bibliotecas WiFiManager e PubSubClient para ESP8266 na IDE do Arduino
//Autor:     Marco Rabelo para o canal Infortronica Para Zumbis (www.youtube.com/c/InfortronicaParaZumbis)
//Descricao: Curso de ESP8266 - Utilizando MQTT e gerenciador de wifi para fazer um interruptor wifi.
//Video:     https://youtu.be/oX4ttJEULmA
////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
Equivalencia das saidas Digitais entre NodeMCU e ESP8266 (na IDE do Arduino)
NodeMCU - ESP8266
D0 = 16;
D1 = 5;
D2 = 4;
D3 = 0;
D4 = 2;
D5 = 14;
D6 = 12;
D7 = 13;
D8 = 15;
D9 = 3;
D10 = 1;
*/

#include <FS.h>                 //Esta precisa ser a primeira referência, ou nada dará certo e sua vida será arruinada. kkk
#include <ESP8266WiFi.h>        //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>        //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>        //https://github.com/bblanchon/ArduinoJson
#include <PubSubClient.h>

#define DEBUG                   //Se descomentar esta linha vai habilitar a 'impressão' na porta serial

//Coloque os valores padrões aqui, porém na interface eles poderão ser substituídos.
#define servidor_mqtt             "soldier.cloudmqtt.com"  //URL do servidor MQTT
#define servidor_mqtt_porta       "10622"  //Porta do servidor (a mesma deve ser informada na variável abaixo)
#define servidor_mqtt_usuario     "ubnuccyu"  //Usuário
#define servidor_mqtt_senha       "aVnZYSCwhNmA"  //Senha
#define mqtt_topico_pub           "Corrente/sensor2"    //Tópico para publicar o comando de inverter o pino do outro ESP8266
#define tensao 110
//Declaração do pino que será utilizado
#define pino                      2                   //Pino que será lido e caso seja alterado seu status, deverá ser enviado um comando para o outro ESP.

const int sensorIn = A0;
int mVperAmp = 100;
double Vpp = 0;
double Vp = 0;
double Vrms = 0;
double Irms = 0;
double consumo = 0.00001;
double getIrms();
double getVPP();

char valueStr[15];
String strtemp = "";

WiFiClient espClient;                                 //Instância do WiFiClient
PubSubClient client(espClient);                       //Passando a instância do WiFiClient para a instância do PubSubClient

bool statusAnt      =             0;                  //Variável que armazenará o status anterior do pino
bool precisaSalvar  =             false;              //Flag para salvar os dados


double getIrms();
double getVPP();

//Função para imprimir na porta serial
void imprimirSerial(bool linha, String mensagem){
  #ifdef DEBUG
    if(linha){
      Serial.println(mensagem);
    }else{
      Serial.print(mensagem);
    }
  #endif
}

//Função de retorno para notificar sobre a necessidade de salvar as configurações
void precisaSalvarCallback() {
  imprimirSerial(true, "As configuracoes tem que ser salvas.");
  precisaSalvar = true;
}

//Função que reconecta ao servidor MQTT
void reconectar() {
  //Repete até conectar
  while (!client.connected()) {
    imprimirSerial(false, "Tentando conectar ao servidor MQTT...");
    
    //Tentativa de conectar. Se o MQTT precisa de autenticação, será chamada a função com autenticação, caso contrário, chama a sem autenticação. 
    bool conectado = strlen(servidor_mqtt_usuario) > 0 ?
                     client.connect("ESP8266Client", servidor_mqtt_usuario, servidor_mqtt_senha) :
                     client.connect("ESP8266Client");

    if(conectado) {
      imprimirSerial(true, "Conectado!");
    } else {
      imprimirSerial(false, "Falhou ao tentar conectar. Codigo: ");
      imprimirSerial(false, String(client.state()).c_str());
      imprimirSerial(true, " tentando novamente em 5 segundos");
      //Aguarda 5 segundos para tentar novamente
      delay(5000);
    }
  }
}

void desconectar(){
  imprimirSerial(true, "Fechando a conexao com o servidor MQTT...");
  client.disconnect();
}

//Função que envia os dados de umidade e temperatura.
void publicaComando() {
  if (!client.connected()) {
    imprimirSerial(true, "MQTT desconectado! Tentando reconectar...");
    reconectar();
  }
  
  client.loop();
  
  //Publicando no MQTT
  imprimirSerial(true, "Fazendo a publicacao...");
  consumo += (getIrms()*tensao)/3600; //CONSUMO PELA VARIAÇÃO DE 5 SEGUNDOS. CONVERSÃO PARA Wh
    strtemp = String(consumo, 1);
    strtemp.toCharArray(valueStr, 15);
    Serial.println("Enviando para MQTT..");
    client.publish(mqtt_topico_pub,valueStr);
}

//Função inicial (será executado SOMENTE quando ligar o ESP)
void setup() {
  #ifdef DEBUG
    Serial.begin(115200);
  #endif
  imprimirSerial(true, "...");

  //Fazendo o pino ser de entrada.
  pinMode(pino, INPUT_PULLUP);

  //Fazeno o pino 0 ser de saída e colocando em BAIXO
  pinMode(0, OUTPUT);
  digitalWrite(0, LOW);
  
  //Formatando a memória interna
  //(descomente a linha abaixo enquanto estiver testando e comente ou apague quando estiver pronto)
  SPIFFS.format();

  //Iniciando o SPIFSS (SPI Flash File System)
  imprimirSerial(true, "Iniciando o SPIFSS (SPI Flash File System)");
  if (SPIFFS.begin()) {
    imprimirSerial(true, "Sistema de arquivos SPIFSS montado!");
    if (SPIFFS.exists("/config.json")) {
      //Arquivo de configuração existe e será lido.
      imprimirSerial(true, "Abrindo o arquivo de configuracao...");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        imprimirSerial(true, "Arquivo de configuracao aberto.");
        size_t size = configFile.size();
        
        //Alocando um buffer para armazenar o conteúdo do arquivo.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
            //Copiando as variáveis salvas previamente no aquivo json para a memória do ESP.
            imprimirSerial(true, "arquivo json analisado.");
            strcpy(servidor_mqtt, json["servidor_mqtt"]);
            strcpy(servidor_mqtt_porta, json["servidor_mqtt_porta"]);
            strcpy(servidor_mqtt_usuario, json["servidor_mqtt_usuario"]);
            strcpy(servidor_mqtt_senha, json["servidor_mqtt_senha"]);
            strcpy(mqtt_topico_pub, json["mqtt_topico_pub"]);

        } else {
          imprimirSerial(true, "Falha ao ler as configuracoes do arquivo json.");
        }
      }
    }
  } else {
    imprimirSerial(true, "Falha ao montar o sistema de arquivos SPIFSS.");
  }
  //Fim da leitura do sistema de arquivos SPIFSS

  //Parâmetros extras para configuração
  //Depois de conectar, parameter.getValue() vai pegar o valor configurado.
  //Os campos do WiFiManagerParameter são: id do parâmetro, nome, valor padrão, comprimento
  WiFiManagerParameter custom_mqtt_server("server", "Servidor MQTT", servidor_mqtt, 40);
  WiFiManagerParameter custom_mqtt_port("port", "Porta", servidor_mqtt_porta, 6);
  WiFiManagerParameter custom_mqtt_user("user", "Usuario", servidor_mqtt_usuario, 20);
  WiFiManagerParameter custom_mqtt_pass("pass", "Senha", servidor_mqtt_senha, 20);
  WiFiManagerParameter custom_mqtt_topic_pub("topic_pub", "Topico para publicar", mqtt_topico_pub, 30);

  //Inicialização do WiFiManager. Uma vez iniciado não é necessário mantê-lo em memória.
  WiFiManager wifiManager;

  //Definindo a função que informará a necessidade de salvar as configurações
  wifiManager.setSaveConfigCallback(precisaSalvarCallback);
  
  //Adicionando os parâmetros para conectar ao servidor MQTT
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_pass);
  wifiManager.addParameter(&custom_mqtt_topic_pub);

  //Busca o ID e senha da rede wifi e tenta conectar.
  //Caso não consiga conectar ou não exista ID e senha,
  //cria um access point com o nome "AutoConnectAP" e a senha "senha123"
  //E entra em loop aguardando a configuração de uma rede WiFi válida.
  if (!wifiManager.autoConnect("AutoConnectAP", "senha123")) {
    imprimirSerial(true, "Falha ao conectar. Excedeu o tempo limite para conexao.");
    delay(3000);
    //Reinicia o ESP e tenta novamente ou entra em sono profundo (DeepSleep)
    ESP.reset();
    delay(5000);
  }

  //Se chegou até aqui é porque conectou na WiFi!
  imprimirSerial(true, "Conectado!! :)");

  //Lendo os parâmetros atualizados
  strcpy(servidor_mqtt, custom_mqtt_server.getValue());
  strcpy(servidor_mqtt_porta, custom_mqtt_port.getValue());
  strcpy(servidor_mqtt_usuario, custom_mqtt_user.getValue());
  strcpy(servidor_mqtt_senha, custom_mqtt_pass.getValue());
  strcpy(mqtt_topico_pub, custom_mqtt_topic_pub.getValue());

  //Salvando os parâmetros informados na tela web do WiFiManager
  if (precisaSalvar) {
    imprimirSerial(true, "Salvando as configuracoes");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["servidor_mqtt"] = servidor_mqtt;
    json["servidor_mqtt_porta"] = servidor_mqtt_porta;
    json["servidor_mqtt_usuario"] = servidor_mqtt_usuario;
    json["servidor_mqtt_senha"] = servidor_mqtt_senha;
    json["mqtt_topico_pub"] = mqtt_topico_pub;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      imprimirSerial(true, "Houve uma falha ao abrir o arquivo de configuracao para incluir/alterar as configuracoes.");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
  }

  imprimirSerial(false, "IP: ");
  imprimirSerial(true, WiFi.localIP().toString());

  //Informando ao client do PubSub a url do servidor e a porta.
  int portaInt = atoi(servidor_mqtt_porta);
  client.setServer(servidor_mqtt, portaInt);
}

//Função de repetição (será executado INFINITAMENTE até o ESP ser desligado)
void loop() {

    publicaComando();
    desconectar();
  delay(100);
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
  if(Irms <= 0.3)
  {
    return 0.0;
  }
  return Irms;
}
