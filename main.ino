#include <EEPROM.h>						 // BIBLIOTECA PARA USO DA MEMÓRIA EEPROM
#include <ESP8266WiFi.h>				 // BIBLIOTECA PADRÃO DO MÓDULO WIFI	
#include <ESP8266WebServer.h>			 // BIBLIOTECA PARA WEB SERVER 
#include <user_interface.h>			 // BIBLIOTECA NECESSARIA PARA ACESSAR OS TIMER`S.
#include "./vendors/EmonLib/EmonLib.cpp" // BIBLIOTECA PARA LEITURA DE ENERGIA

// ESTADOS PARA CONFIGURAR NODE
#define CLEAR_EEPROM true

#define CURRENT_CAL 195					 // VALOR DE CALIBRAÇAO DO SENSOR DE CORRENTE
#define NOISE 0.25						 // VALOR DE RUÍDO DO SENSOR DE CORRENTE
#define SENSOR_PIN 9					 // PINO DO SENSOR DE CORRENTE

#define RELAY_PIN 4						 // PINO DO RELE

#define AP_SSID "AMEE"					 // SSID DO AP
#define AP_PASS "123456789"				 // SENHA DO AP
#define API_URL "amee-api.herokuapp.com" // URL DA API

// ENDEREÇO DOS DADOS NA EEPROM
#define EEPROM_ISSET 0
#define EEPROM_ID	 11
#define EEPROM_SSID  51
#define EEPROM_PASS  101
#define MAX_EEPROM_DATA 40

EnergyMonitor acs712;					 // INSTANCIA O SENSOR DE CORRENTE
ESP8266WebServer server(80);			 // INSTANCIA UM SERVIDOR NA PORTA 80
WiFiClient client;					   	 // INSTANCIA UM CLIENTE
os_timer_t powerOffTimer;				 // INSTANCIA O TIMER PARA DELIGAR O APARELHO

/*
 * DADOS INICIAIS DO NODE
 */
String ssid			= "Familiadoremi";	// SSID DA REDE
String pass			= "Deus12345";		// SENHA DA REDE
String id 			= "";				// ID DO NODE
String ip 			= "";				// IP DO NODE
String name 		= ""; 				// NOME DO EQUIPAMENTO
String tension 		= ""; 				// TENSÃO DE OPERAÇÃO DO EQUIPAMENTO
String power 		= "";			// POTÊNCIA DE OPERAÇÃO DO EQUIPAMENTO
bool state 			= false;			// STATUS DO EQUIPAMENTO
double lastCurrent 	= 0; 				// ÚLTIMO VALOR DA DE CORRENTE LIDO
double current 		= 0; 				// VALOR DA CORRENTE MÉDIA ATUAL
// bool toPowerOff 	= false;			// DEVE DESLIGAR QUANDO POWEROFF CHEGAR A 0?
// int powerOff    	= 0;				// TEMPO RESTANTE PARA ELE DESLIGAR

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

		EEPROM.begin(512);
		delay(100);
		EEPROM.put(EEPROM_ID, (String)id);
		delay(100);
		EEPROM.commit();
		delay(100);
		EEPROM.end();

		Serial.println("SALVO NO EEPROM");

		return true;
	}

	return false;
}

bool handleCreateLog() {
	if (client.connect(API_URL, 80)) {
		Serial.println("CRIANDO LOG");

		String postData = "id=" + id + "&state=" + state + "&currrent=" + current;
		Serial.println(postData);

		client.println("POST /log HTTP/1.1");
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
	} else {
		server.send(400, "application/x-www-form-urlencoded", "error");
	}
}

void setPowerOff(void *z) {
	state = false;

	digitalWrite(RELAY_PIN, LOW);
	// toPowerOff = false;
	// powerOff = 0;

	Serial.print("Mudou status para: false");

	handleCreateLog();
	current = 0;
}

void handleChangeStatus() {
	server.sendHeader("Access-Control-Allow-Origin", "*");

	if (server.arg("state") == "false") {
		state = false;
		digitalWrite(RELAY_PIN, LOW);
	} else {
		state = true;
		digitalWrite(RELAY_PIN, HIGH);
	}

	// toPowerOff = false;
	// powerOff = 0;

	Serial.print("Mudou status para: ");
	Serial.println(server.arg("state"));
	server.send(200);

	handleCreateLog();
	current = 0;
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

	EEPROM.write(add+_size,'\0');   //Add termination null character for String Data
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

void handleTeste() {
	server.sendHeader("Access-Control-Allow-Origin", "*");
	Serial.print("\nTestando... ");

	String test = server.arg("teste");
	String a = "";
	
	a = readInEEPROM(0);
	Serial.print("Last: ");
	Serial.println(a);

	writeInEEPROM(0, test);

	a = readInEEPROM(0);
	Serial.print("Now: ");
	Serial.println(a);
}

void handleSetPowerOff() {
	server.sendHeader("Access-Control-Allow-Origin", "*");

	int ms = server.arg("seconds").toInt();
	ms = ms * 1000;
	// toPowerOff = true;

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
		server.on("/teste", HTTP_POST, handleTeste);
		server.begin();

		ip = WiFi.localIP().toString().c_str();

		if (!modifiedEEPROM()) {
			Serial.println("SALVANDO REDE NA EEPROM");
			EEPROM.begin(512);
			delay(100);
			EEPROM.put(EEPROM_ISSET, 1);
			delay(100);
			EEPROM.put(EEPROM_SSID, ssid);
			delay(100);
			EEPROM.put(EEPROM_PASS, pass);
			delay(100);
			EEPROM.commit();
			delay(100);
			EEPROM.end();
			delay(100);
		}

		Serial.print("\nip:");
		Serial.println(ip);

		return true;
	}

	Serial.println("Erro na conexão!");
	return false;
}

/*
 * FUNÇÃO QUE CHECA SE HOUVE UMA PRIMEIRA INICIALIZAÇÃO (SSID, PASS E ID ESTÃO SETADOS NA EEPROM)
 */
bool modifiedEEPROM() {
	return true;
	EEPROM.begin(512);
	delay(100);
	
	int val;

	EEPROM.get(EEPROM_ISSET, val);
	Serial.print("CONTROLE DA EEPROM: ");
	Serial.println(val);

	Serial.println("read() da EEPROM:");
	for (int i = 0 ; i < EEPROM.length(); i++) {
		Serial.print(EEPROM.read(i));
		Serial.print(", ");
		delay(10);
	}

	EEPROM.end();

	return true;
	
	return val == 1;
}

void setup() {
	Serial.begin(115200);
	Serial.flush();

	acs712.current(9, CURRENT_CAL);
	pinMode(4, OUTPUT);
	os_timer_setfn(&powerOffTimer, setPowerOff, NULL);

	if (!CLEAR_EEPROM) {
		EEPROM.begin(512);
		delay(10);
		Serial.print("LIMPANDO.");
		for (int i = 0 ; i < EEPROM.length(); i++) {
			EEPROM.write(i, -1);
			delay(10);
			Serial.print(".");
		}
		EEPROM.commit();
		delay(10);
		EEPROM.end();
		delay(10);

		Serial.println("EEPROM LIMPA!");
	}

	WiFi.mode(WIFI_AP_STA);
	WiFi.enableAP(false);
	WiFi.enableSTA(false);
	
	if (modifiedEEPROM()) { // CHECA SE OS DADOS DE SSID E PASSWORD DA REDE ESTÃO SALVOS NA EEPROM
		Serial.println("DADOS SALVOS NA EEPROM:");
		
		// EEPROM.begin(512);
		// delay(100);
		// EEPROM.get(EEPROM_SSID, ssid);
		// delay(100);
  		// EEPROM.get(EEPROM_PASS, pass);
		// delay(100);
		// EEPROM.get(EEPROM_ID, id);
		// delay(100);
		// EEPROM.end();

		// Serial.println(ssid);
		// Serial.println(pass);
		// Serial.println(id);

		if (networkConnect()) {
			return;
		}
	} else {
		// SE NÃO, CRIA UM PONTO DE ACESSO PARA O USUÁRIO SALVAR OS DADOS
		WiFi.enableAP(true);
		WiFi.softAP(AP_SSID, AP_PASS);

		Serial.println("PA CRIADO");

		server.on("/config", HTTP_POST, handleCreateNode);
		server.begin();
	}

	Serial.print("ID: ");
	Serial.println(id);
}

/*
 * FUNÇÃO PARA CALCULAR A CORRENTE
 */
void calcCurrent() {
	Serial.print("LENDO CORRENTE TOTAL: ");
	Serial.print(current);
	Serial.println("mA");

	double readedCurrent, currentDraw = 0;
	acs712.calcVI(20, 100); //FUNÇÃO DE CÁLCULO (20 SEMICICLOS / TEMPO LIMITE PARA FAZER A MEDIÇÃO)

	readedCurrent = (acs712.Irms - NOISE); //VARIÁVEL RECEBE O VALOR DE CORRENTE RMS OBTIDO
	
	if (readedCurrent < 0) {
		readedCurrent = 0;
	}

	current += readedCurrent;
}

void loop(){
	server.handleClient();

	if (state) {
		calcCurrent();
	}

	delay(100);
}