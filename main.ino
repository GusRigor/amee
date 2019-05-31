#include <EEPROM.h>						 // BIBLIOTECA PARA USO DA MEMÓRIA EEPROM
#include <ESP8266WiFi.h>				 // BIBLIOTECA PADRÃO DO MÓDULO WIFI	
#include <ESP8266WebServer.h>			 // BIBLIOTECA PARA WEB SERVER 
#include "./vendors/EmonLib/EmonLib.cpp" // BIBLIOTECA PARA LEITURA DE ENERGIA
#include "./html/credentials.h"			 // BIBLIOTECA COM A PÁGINA HTML

#define CURRENT_CAL 195
#define NOISE 0.25
#define SENSOR_PIN A0

#define RELAY_PIN D2

#define AP_SSID "AMEE"
#define AP_PASS "123456789"
#define API_URL "amee-api.herokuapp.com"

EnergyMonitor acs712;						// INSTANCIA O SENSOR DE CORRENTE
ESP8266WebServer server(80);				// INSTANCIA UM SERVIDOR NA PORTA 80
WiFiClient client;							// INSTANCIA UM CLIENTE

/*
 * DADOS INICIAIS DO NODE
 */
String ssid			= "Familiadoremi";	// SSID DA REDE
String pass			= "Deus12345";		// SENHA DA REDE
String id 				= "";				// ID DO NODE
String ip 			= "";				// IP DO NODE
String name 		= ""; 				// NOME DO EQUIPAMENTO
String tension 		= ""; 				// TENSÃO DE OPERAÇÃO DO EQUIPAMENTO
String power 		= ""; 				// POTÊNCIA DE OPERAÇÃO DO EQUIPAMENTO
bool status 		= true;				// STATUS DO EQUIPAMENTO
double lastCurrent 	= 0; 				// ÚLTIMO VALOR DA DE CORRENTE LIDO
double current 		= 0; 				// VALOR DA CORRENTE MÉDIA ATUAL
//DATE powerOff    	= 0;				//  TEMPO RESTANTE PARA ELE DESLIGAR

/*
 * FUNÇÕES CLIENTE
 */
bool handleCreateOnServer() {
	Serial.println(API_URL);

	if (client.connect(API_URL, 80)) {
		Serial.println("[connected]");

		String postData = "name=" + name + "&ip=" + ip + "&tension=" + tension + "&power=" + power + "";

		client.println("POST /node HTTP/1.1");
		client.print("Host: ");
		client.println(API_URL);
		client.println("Cache-Control: no-cache");
		client.println("Content-Type: application/x-www-form-urlencoded");
		client.print("Content-Length: ");
		client.println(postData.length());
		client.println();
		client.println(postData);

		long interval = 2000;
		unsigned long currentMillis = millis(), previousMillis = millis();

		while(!client.available()) {
			if ((currentMillis - previousMillis) > interval ) {
				client.stop();     
				return false;
			}

			currentMillis = millis();
		}

		Serial.println("[Response:]");
		while (client.connected()) {
			if (client.available()) {
				String str = client.readStringUntil('{');
   				Serial.println(str);

				id = str.substring(str.indexOf("id\":") + 5, str.indexOf("\"}"));
			}
		}

		client.stop();
		Serial.println("\n[Disconnected]");
		Serial.println(id);
		return true;
	}

	Serial.println("connection failed!]");
	return false;
}

/*
 * ROTAS NODE
 */
void handleShowCredentialsForm() {
	server.send(200, "text/html", CREDENTIALS_HTML);
}

void handleCreateNode() {
	server.sendHeader("Access-Control-Allow-Origin", "*");

	ssid = server.arg("ssid");
	pass = server.arg("pass");
	name = server.arg("name");
	tension = server.arg("tension");
	power = server.arg("power");

	if (networkConnect()) {
		if (handleCreateOnServer()) {
			server.send(200);
		}
	}

	server.send(200, "application/x-www-form-urlencoded", "error");
}

void handleCreate() {
    EEPROM.begin(512);

	if (EEPROM.read(0) == 1) { // O ID JÄ ESTÁ GRAVADO NA EEPROM
		EEPROM.get(1, ssid);
  		EEPROM.get(1+sizeof(ssid), pass);
	}  else {


		// AQUI, CRIAR O NODE NO SERVIDOR E SALVAR O ID DELE
	}

	EEPROM.end();
}

void handleChange() {

}

void handleGetNode() {

}

void handleChangeStatus() {

}

void handleGetCurrent() {

}

void handleSetPowerOff() {

}

void handleDestroyPowerOff() {

}

/*
 * FUNÇÃO QUE CONECTA À REDE
 */
boolean networkConnect() {
	Serial.println("Criando server");
	
	WiFi.enableSTA(true);
    WiFi.begin(ssid, pass);

	Serial.println(WiFi.status());

	while (WiFi.status() == WL_DISCONNECTED || WiFi.status() == WL_IDLE_STATUS) {
        delay(100);
		Serial.print(".");
    }

	Serial.println("");

	if (WiFi.status() == WL_CONNECTED) {
		server.begin();

		ip = WiFi.localIP().toString();

		Serial.print("IP do node: ");
		Serial.println(ip);

		return true;
	}

	return false;
}

/*
 * FUNÇÃO QUE CHECA SE HOUVE UMA PRIMEIRA INICIALIZAÇÃO (SSID, PASS E ID ESTÃO SETADOS NA EEPROM)
 */
bool modifiedEEPROM() {
	return false;
	int r;

	EEPROM.begin(4);
	r = EEPROM.read(0);
	EEPROM.end();

	Serial.print("R = ");
	Serial.println(r);

	if (r == 255) {
		Serial.println("==== HEHE ====");
		return false;
	}

	return true;
}

void setup(){
	Serial.begin(115200);
	Serial.flush();
    Serial.println();
	
	Serial.println("Checando EEPROM");
	if (modifiedEEPROM() == true) { // CHECA SE OS DADOS DE SSID E PASSWORD DA REDE ESTÃO SALVOS NA EEPROM
		networkConnect();
	} else { // SE NÃO, CRIA UM PONTO DE ACESSO PARA O USUÁRIO SALVAR OS DADOS
		Serial.println("Criando ponto de acesso");
		WiFi.mode(WIFI_AP_STA);
		WiFi.softAP(AP_SSID, AP_PASS);
		Serial.println("AP criado");
		
		server.on("/", HTTP_GET, handleShowCredentialsForm);
		server.on("/config", HTTP_POST, handleCreateNode);

		// ROTAS DO NODE
		// server.on("/equipment", HTTP_GET, handleGetNodeEquipment);
		// server.on("/equipment", HTTP_POST,  handleCreateEquipment);
		// server.on("/equipment", HTTP_PUT, handleChangeEquipment);
		// server.on("/equipment", HTTP_DELETE, handleDeleteEquipment);
		// server.on("/status", HTTP_POST, handleChangeStatus);
		// server.on("/current", HTTP_GET, handleGetCurrent);
		// server.on("/power-off", HTTP_POST, handleSetPowerOff);
		// server.on("/power-off", HTTP_DELETE, handleDestroyPowerOff);

		server.begin();
		Serial.println("Server iniciado");
		Serial.print("IP: ");
		Serial.println(WiFi.localIP());
	}

	acs712.current(SENSOR_PIN, CURRENT_CAL);
	pinMode(RELAY_PIN, OUTPUT);
}

bool firstLoop = true; // PULAREMOS O PRIMEIRO LOOP DEVIDO O RUÍDO NO BOOT DO NODE
void loop(){
	server.handleClient();

	if (firstLoop) { // CASO ESTEJMOS NO PRIMEIRO LOOP:
		delay(1000);

		firstLoop = false;
		return;
	}

	delay(100);
}