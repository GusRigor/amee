#include <EEPROM.h>						 // BIBLIOTECA PARA USO DA MEMÓRIA EEPROM
#include <ESP8266WiFi.h>				 // BIBLIOTECA PADRÃO DO MÓDULO WIFI	
#include <ESP8266WebServer.h>			 // BIBLIOTECA PARA WEB SERVER 
#include "./vendors/EmonLib/EmonLib.cpp" // BIBLIOTECA PARA LEITURA DE ENERGIA

#define CURRENT_CAL 195					 // VALOR DE CALIBRAÇAO DO SENSOR DE CORRENTE
#define NOISE 0.25						 // VALOR DE RUÍDO DO SENSOR DE CORRENTE
#define SENSOR_PIN A0					 // PINO DO SENSOR DE CORRENTE

#define RELAY_PIN D2					 // PINO DO RELE

#define AP_SSID "AMEE"					 // SSID DO AP
#define AP_PASS "123456789"				 // SENHA DO AP
#define API_URL "amee-api.herokuapp.com" // URL DA API

EnergyMonitor acs712;					 // INSTANCIA O SENSOR DE CORRENTE
ESP8266WebServer server(80);			 // INSTANCIA UM SERVIDOR NA PORTA 80
WiFiClient client;					   	 // INSTANCIA UM CLIENTE

/*
 * DADOS INICIAIS DO NODE
 */
String ssid			= "Familiadoremi";	// SSID DA REDE
String pass			= "Deus12345";		// SENHA DA REDE
String id 			= "";				// ID DO NODE
String ip 			= "";				// IP DO NODE
String name 		= ""; 				// NOME DO EQUIPAMENTO
String tension 		= ""; 				// TENSÃO DE OPERAÇÃO DO EQUIPAMENTO
String power 		= ""; 				// POTÊNCIA DE OPERAÇÃO DO EQUIPAMENTO
bool state 			= false;			// STATUS DO EQUIPAMENTO
double lastCurrent 	= 0; 				// ÚLTIMO VALOR DA DE CORRENTE LIDO
double current 		= 0; 				// VALOR DA CORRENTE MÉDIA ATUAL
bool toPowerOff 	= false;			// DEVE DESLIGAR QUANDO POWEROFF CHEGAR A 0?
int powerOff    	= 0;				// TEMPO RESTANTE PARA ELE DESLIGAR

/*
 * FUNÇÕES CLIENTE
 */
bool handleCreateOnServer() {

	if (client.connect(API_URL, 80)) {
		Serial.println("CRIANDO NODE");

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

		while (client.connected()) {
			if (client.available()) {
				String str = client.readStringUntil('{');

				id = str.substring(str.indexOf("id\":") + 5, str.indexOf("\"}"));
			}
		}

		Serial.println("CRIADO");

		client.stop();

		EEPROM.begin(512);
		EEPROM.put(211, id);
		EEPROM.commit();
		EEPROM.end();

		Serial.println("SALVO NO EEPROM");

		return true;
	}

	return false;
}

bool handleCreateLog() {
	if (client.connect(API_URL, 80)) {
		Serial.println("CRIANDO NODE");

		String postData = "id=" + id + "&state=" + state + "&currrent=" + current;

		client.println("POST /log HTTP/1.1");
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

		while (client.connected()) {
			if (client.available()) {
				String str = client.readStringUntil('{');

				id = str.substring(str.indexOf("id\":") + 5, str.indexOf("\"}"));
			}
		}

		client.stop();

		return true;
	}

	return false;
}

/*
 * ROTAS NODE
 */
void handleCreateNode() {
	server.sendHeader("Access-Control-Allow-Origin", "*");

	ssid = server.arg("ssid");
	pass = server.arg("pass");
	name = server.arg("name");
	tension = server.arg("tension");
	power = server.arg("power");

	Serial.println("CONECTANDO-SE À REDE");
	if (networkConnect()) {
		if (handleCreateOnServer()) {
			server.send(200);
		}
	}

	server.send(200, "application/x-www-form-urlencoded", "error");
}

void handleChangeStatus() {
	if (server.arg("state") == "false") {
		state = false;
	} else {
		state = true;
	}

	handleCreateLog();

	digitalWrite(RELAY_PIN, state);

	if (state == false) {
		current = 0;
		lastCurrent = 0;
		toPowerOff = false;
		powerOff = 0;
	}
}

void handleSetPowerOff() {

}

void handleDestroyPowerOff() {
	powerOff = 0;
}

/*
 * FUNÇÃO QUE CONECTA À REDE
 */
boolean networkConnect() {
	WiFi.enableSTA(true);
    WiFi.begin(ssid, pass);

	while (WiFi.status() == WL_DISCONNECTED || WiFi.status() == WL_IDLE_STATUS) {
        delay(100);
    }

	if (WiFi.status() == WL_CONNECTED) {
		server.on("/status", HTTP_POST, handleChangeStatus);
		server.on("/power-off", HTTP_POST, handleSetPowerOff);
		server.begin();

		ip = WiFi.localIP().toString();

		Serial.println("SALVANDO REDE NA EEPROM");
		EEPROM.begin(512);
		EEPROM.put(0, 1);
		EEPROM.put(1, ssid);
		EEPROM.put(111, pass);
		EEPROM.commit();
		EEPROM.end();

		Serial.println("CONECTADO");

		return true;
	}

	return false;
}

/*
 * FUNÇÃO QUE CHECA SE HOUVE UMA PRIMEIRA INICIALIZAÇÃO (SSID, PASS E ID ESTÃO SETADOS NA EEPROM)
 */
bool modifiedEEPROM() {
	int r;

	EEPROM.begin(4);
	r = EEPROM.read(0);
	EEPROM.end();


	if (r == 255) {
		return false;
	}

	return true;
}

void setup(){
	Serial.begin(115200);
	Serial.flush();

	acs712.current(SENSOR_PIN, CURRENT_CAL);
	pinMode(RELAY_PIN, OUTPUT);
	
	if (modifiedEEPROM() == true) { // CHECA SE OS DADOS DE SSID E PASSWORD DA REDE ESTÃO SALVOS NA EEPROM
		Serial.println("SALVO NA EEPROM");
		
		EEPROM.begin(512);
		EEPROM.get(1, ssid);
  		EEPROM.get(111, pass);
		EEPROM.get(211, id);
		EEPROM.end();

		Serial.println(ssid);

		if (networkConnect()) {
			return;
		}
	}
	
	// SE NÃO, CRIA UM PONTO DE ACESSO PARA O USUÁRIO SALVAR OS DADOS
	WiFi.mode(WIFI_AP_STA);
	WiFi.softAP(AP_SSID, AP_PASS);

	Serial.println("PA CRIADO");

	server.on("/config", HTTP_POST, handleCreateNode);
	server.begin();
}

/*
 * FUNÇÃO PARA CALCULAR A CORRENTE
 */
void calcCurrent() {
	Serial.println("LENDO CORRENTE");
	double readedCurrent, totalSum, currentDraw = 0;

	for (int i = 0; i <= 20; i++) {
		acs712.calcVI(20, 100); //FUNÇÃO DE CÁLCULO (20 SEMICICLOS / TEMPO LIMITE PARA FAZER A MEDIÇÃO)

		readedCurrent = (acs712.Irms - NOISE); //VARIÁVEL RECEBE O VALOR DE CORRENTE RMS OBTIDO
		
		if (readedCurrent < 0) {
			readedCurrent = 0;
		}

		totalSum += readedCurrent;

		delay(10);
	}

	currentDraw = totalSum / 200;
}

bool firstLoop = true; // PULAREMOS O PRIMEIRO LOOP DEVIDO O RUÍDO NO BOOT DO NODE
void loop(){
	server.handleClient();

	if (id != "" && state) {
		calcCurrent();
	}

	if (toPowerOff && powerOff < 1) {
		digitalWrite(RELAY_PIN, 0);
		toPowerOff = false;
		state = false;

	}

	delay(100);
}