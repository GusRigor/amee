/*
 * IP DO SERVER NO PA: 192.168.4.1
 * ESTADOS NO RElÉ ESTÃO AO CONTRÁRIO
 */

#include <EEPROM.h>						 // BIBLIOTECA PARA USO DA MEMÓRIA EEPROM
#include <ESP8266WiFi.h>				 // BIBLIOTECA PADRÃO DO MÓDULO WIFI	
#include <ESP8266WebServer.h>			 // BIBLIOTECA PARA WEB SERVER 
#include <user_interface.h>			 	 // BIBLIOTECA NECESSARIA PARA ACESSAR OS TIMER`S.
#include "./vendors/EmonLib/EmonLib.cpp" // BIBLIOTECA PARA LEITURA DE ENERGIA

// ESTADOS PARA CONFIGURAR NODE
#define CLEAR_EEPROM false

#define CURRENT_CAL 195					 // VALOR DE CALIBRAÇAO DO SENSOR DE CORRENTE
#define NOISE 0.25						 // VALOR DE RUÍDO DO SENSOR DE CORRENTE
#define SENSOR_PIN A0					 // PINO DO SENSOR DE CORRENTE

#define RELAY_PIN 4						 // PINO DO RELE

#define AP_SSID "AMEE"					 // SSID DO AP
#define AP_PASS "123456789"				 // SENHA DO AP
#define API_URL "amee-api.herokuapp.com" // URL DA API

// ENDEREÇO DOS DADOS NA EEPROM
#define EEPROM_ISSET 0
#define EEPROM_ID	 10
#define EEPROM_SSID  50
#define EEPROM_PASS  80
#define EEPROM_IP  	 110
#define MAX_EEPROM_DATA 40

EnergyMonitor acs712;					 // INSTANCIA O SENSOR DE CORRENTE
ESP8266WebServer server(80);			 // INSTANCIA UM SERVIDOR NA PORTA 80
WiFiClient client;					   	 // INSTANCIA UM CLIENTE
os_timer_t powerOffTimer;				 // INSTANCIA O TIMER PARA DELIGAR O APARELHO

/*
 * DADOS INICIAIS DO NODE
 */
String ssid			= "";				// SSID DA REDE
String pass			= "";				// SENHA DA REDE
String id 			= "";				// ID DO NODE
String ip 			= "";				// IP DO NODE
String name 		= ""; 				// NOME DO EQUIPAMENTO
String tension 		= ""; 				// TENSÃO DE OPERAÇÃO DO EQUIPAMENTO
String power 		= "";				// POTÊNCIA DE OPERAÇÃO DO EQUIPAMENTO
bool state 			= false;			// STATUS DO EQUIPAMENTO
double current 		= 0; 				// VALOR DA CORRENTE SENDO LIDA
double totalCurrent = 0;				// VALOR DE TODA A CORRENTE QUE FOI LIDA DESDE QUE LIGOU O EQUIPAMENTO
double lastCurrent  = 0; 				// VALOR DA ÚLTIMA CORRENTE SALVA NO SERVIDOR

/*
 * FUNÇÕES CLIENTE
 */
bool handleCreateOnServer() {
	if (client.connect(API_URL, 80)) {
		Serial.println("CRIANDO NODE");

		String postData = "name=" + name + "&ip=" + ip + "&tension=" + tension + "&power=" + power + "";
		Serial.println(postData);

		client.println("POST /node HTTP/1.1");
		client.print("Host: ");
		client.println(API_URL);
		client.println("Cache-Control: no-cache");
		client.println("Content-Type: application/x-www-form-urlencoded");
		client.print("Content-Length: ");
		client.println(postData.length());
		client.println();
		client.println(postData);

		while (client.connected()) {
			if (client.available()) {
				String str = client.readStringUntil('{');

				id = str.substring(str.indexOf("id\":") + 5, str.indexOf("\"}"));
			}
		}

		Serial.println("CRIADO");
		client.stop();

		writeInEEPROM(EEPROM_ID, id);
		Serial.println("ID SALVO NO EEPROM");

		return true;
	}

	return false;
}

void handleGetState() {
	if (client.connect(API_URL, 80)) {
		Serial.println("RECUPERANDO ÚLTIMO ESTADO SALVO");

		String postData = "";
		String newState;

		client.print("GET /node/");
		client.print(id);
		client.println(" HTTP/1.1");
		client.print("Host: ");
		client.println(API_URL);
		client.println("Content-Type: application/x-www-form-urlencoded");
		client.print("Content-Length: ");
		client.println(postData.length());
		client.println();
		client.println(postData);

		while (client.connected()) {
			if (client.available()) {
				String str = client.readStringUntil('{');
				newState = str.substring(str.indexOf("state\":") + 7, str.indexOf(",\"tension"));
			}
		}

		Serial.print("\nESTADO RECUPERADO: ");
		Serial.println(!newState);

		state = !(newState == "false");

		digitalWrite(RELAY_PIN, state);
		current = 0;
		totalCurrent = 0;

		client.stop();
	}
}

void handleChangeNodeIP() {
	if (client.connect(API_URL, 80)) {
		Serial.println("MUDANDO IP DO NODE NO SERVER");

		String postData = "ip=" + ip;
		Serial.println(postData);

		client.print("PUT /node/");
		client.print(id);
		client.println(" HTTP/1.1");
		client.print("Host: ");
		client.println(API_URL);
		client.println("Cache-Control: no-cache");
		client.println("Content-Type: application/x-www-form-urlencoded");
		client.print("Content-Length: ");
		client.println(postData.length());
		client.println();
		client.println(postData);

		while (client.connected()) {
			delay(100);
		}

		Serial.println("IP ATUALIZADO COM SUCESSO!");
		client.stop();
	}
}

void handleUpdateCurrent() {
	if (client.connect(API_URL, 80)) {
		Serial.println("GRAVANDO VALOR DA CORRENTE LIDA...");

		String postData = "current=" + String(current, 2);
		Serial.println(postData);

		client.print("PUT /node/");
		client.print(id);
		client.println(" HTTP/1.1");
		client.print("Host: ");
		client.println(API_URL);
		client.println("Cache-Control: no-cache");
		client.println("Content-Type: application/x-www-form-urlencoded");
		client.print("Content-Length: ");
		client.println(postData.length());
		client.println();
		client.println(postData);

		while (client.connected()) {
			delay(100);
		}

		Serial.println("GRAVADO");
		client.stop();

		lastCurrent = current;
	}
}

void handleChangeServerState() {
	if (client.connect(API_URL, 80)) {
		Serial.println("MUDANDO ESTADO DO NODE NO SERVER");

		String queryData = "?totalCurrent=" + String(totalCurrent, 2) + "&state=";
		String postData = "";

		if (state) {
			queryData += "true";
		} else {
			queryData += "false";
		}

		Serial.println(queryData);

		client.print("POST /node/change-state/");
		client.print(id);
		client.print(queryData);
		client.println(" HTTP/1.1");
		client.print("Host: ");
		client.println(API_URL);
		client.println("Cache-Control: no-cache");
		client.println("Content-Type: application/x-www-form-urlencoded");
		client.print("Content-Length: ");
		client.println(postData.length());
		client.println();
		client.println(postData);

		while (client.connected()) {
			delay(100);
		}

		Serial.println("ESTADO MUDADO");
		client.stop();
	}
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
	} else {
		server.send(400, "application/x-www-form-urlencoded", "error");
	}
}

void setPowerOff(void *z) {
	if (state == true) {
		return;
	}

	state = false;
	digitalWrite(RELAY_PIN, HIGH);

	Serial.print("Mudou status para: false");
	handleChangeServerState();

	current = 0;
	totalCurrent = 0;
}

void handleChangeStatus() {
	server.sendHeader("Access-Control-Allow-Origin", "*");

	bool newState = (server.arg("state") == "true");

	if (newState == state) {
		return;
	}

	state = newState;
	digitalWrite(RELAY_PIN, !state);

	Serial.print("Mudou status para: ");
	Serial.println(state);
	
	handleChangeServerState();
	server.send(200);

	current = 0;
	totalCurrent = 0;
}

/*
 *	FUNÇÕES PARA GRAVAR E LER DADOS DA EEPROM
 */
void writeInEEPROM(int add, String data) {
	EEPROM.begin(512);
	int _size = data.length();
	int i = 0;

	for(i=0;i<_size;i++) {
		EEPROM.write(add+i,data[i]);
		delay(20);
	}

	EEPROM.write(add+_size,'\0');
	delay(25);
	EEPROM.commit();
	delay(250);
	EEPROM.end();
	delay(100);
}

String readInEEPROM(int add) {
	EEPROM.begin(512);
	int i, len = 0;
	char data[MAX_EEPROM_DATA];
	unsigned char k;

	while(k != '\0' && len<MAX_EEPROM_DATA) {    
		k=EEPROM.read(add+len);
		data[len]=k;
		len++;
		delay(10);
	}

	EEPROM.end();
	
	return String(data);
}

void handleSetPowerOff() {
	server.sendHeader("Access-Control-Allow-Origin", "*");

	int secs = server.arg("seconds").toInt();
	long int ms = secs * 1000;

	os_timer_arm(&powerOffTimer, ms, false);

	Serial.print("Desligará em:");
	Serial.print(ms/1000);
	Serial.print("s");
}

void handleDestroyPowerOff() {
	os_timer_disarm(&powerOffTimer);
	Serial.println("Não desligará mais!");
}

/*
 * FUNÇÃO QUE CONECTA À REDE
 */
boolean networkConnect() {
	WiFi.enableSTA(true);
	WiFi.begin(ssid, pass);

	Serial.println(ssid);
	Serial.println(pass);

	Serial.print("Tentando conectar-se a rede .");
	while (WiFi.status() == WL_DISCONNECTED || WiFi.status() == WL_IDLE_STATUS) {
		Serial.print(".");
        delay(100);
    }

	if (WiFi.status() == WL_CONNECTED) {
		Serial.println("\nConectado!");
		server.on("/status", HTTP_POST, handleChangeStatus);
		server.on("/power-off", HTTP_POST, handleSetPowerOff);
		server.on("/power-off", HTTP_DELETE, handleDestroyPowerOff);
		server.begin();

		ip = WiFi.localIP().toString().c_str();

		if (!modifiedEEPROM()) {
			writeInEEPROM(EEPROM_ISSET, "1");
			writeInEEPROM(EEPROM_SSID, ssid);
			writeInEEPROM(EEPROM_PASS, pass);
			writeInEEPROM(EEPROM_IP, ip);
		}

		Serial.print("\nip:");
		Serial.println(ip);

		String lastIp = "";

		lastIp = readInEEPROM(EEPROM_IP);

		if (lastIp != ip) {
			handleChangeNodeIP();
			writeInEEPROM(EEPROM_IP, ip);
		}

		return true;
	}

	Serial.println("Erro na conexão!");
	return false;
}

/*
 * FUNÇÃO QUE CHECA SE HOUVE UMA PRIMEIRA INICIALIZAÇÃO (SSID, PASS E ID ESTÃO SETADOS NA EEPROM)
 */
bool modifiedEEPROM() {	
	String val = "";
	val = readInEEPROM(EEPROM_ISSET);

	return (val.toInt() == 1);
}

void setup() {
	Serial.begin(115200);
	Serial.flush();

	acs712.current(SENSOR_PIN, CURRENT_CAL);
	pinMode(4, OUTPUT);
	digitalWrite(4, LOW);
	os_timer_setfn(&powerOffTimer, setPowerOff, NULL);

	if (CLEAR_EEPROM) {
		EEPROM.begin(512);
		delay(10);
		Serial.print("\nLIMPANDO EEPROM.");
		for (int i = 0 ; i < EEPROM.length(); i++) {
			EEPROM.write(i, -1);
			delay(10);
			Serial.print(".");
		}

		EEPROM.commit();
		delay(10);
		EEPROM.end();
		delay(10);

		Serial.println("\nEEPROM LIMPA!");
	}

	WiFi.mode(WIFI_AP_STA);
	WiFi.enableAP(false);
	WiFi.enableSTA(false);
	
	if (modifiedEEPROM()) { // CHECA SE OS DADOS DE SSID E PASSWORD DA REDE ESTÃO SALVOS NA EEPROM
		Serial.println("DADOS SALVOS NA EEPROM:");
		
		ssid = readInEEPROM(EEPROM_SSID);
		pass = readInEEPROM(EEPROM_PASS);
		id = readInEEPROM(EEPROM_ID);

		Serial.println(ssid);
		Serial.println(pass);
		Serial.println(id);

		if (networkConnect()) {
			handleGetState();
			return;
		}
	} else {
		// SE NÃO, CRIA UM PONTO DE ACESSO PARA O USUÁRIO SALVAR OS DADOS
		WiFi.enableAP(true);
		WiFi.softAP(AP_SSID, AP_PASS, 1, 1);

		server.on("/config", HTTP_POST, handleCreateNode);
		server.begin();

		Serial.print("PA CRIADO");
	}
}

/*
 * FUNÇÃO PARA CALCULAR A CORRENTE
 */
void calcCurrent() {
	double readedCurrent = 0;
	readedCurrent = acs712.calcIrms(1500) - NOISE; // NUMERO DE AMOSTRAS

	// readedCurrent = (acs712.Irms - NOISE); //VARIÁVEL RECEBE O VALOR DE CORRENTE RMS OBTIDO
	
	if (readedCurrent < 0) {
		readedCurrent = 0;
	}

	current = (readedCurrent * 1000);
	totalCurrent += current;

	Serial.print("LENDO CORRENTE: ");
	Serial.print(current);
	Serial.println("mA");

	if ((lastCurrent - 100) > current || (lastCurrent + 100) < current) {
		void handleUpdateCurrent();
	}
}

void loop(){
	server.handleClient();

	if (state) {
		calcCurrent();
	}

	delay(100);
}