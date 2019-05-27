#include <ESP8266WiFi.h>				 // BIBLIOTECA PADRÃO DO MÓDULO WIFI
#include <ESP8266WebServer.h>			 // BIBLIOTECA PARA WEB SERVER 
#include "./vendors/EmonLib/EmonLib.cpp" // BIBLIOTECA PARA LEITURA DE ENERGIA

#define CURRENT_CAL 195						// VALOR DE CALIBRAÇÃO DO SENSOR DE ENERGIA
#define NOISE 0.25							// RUÍDO PRODUZIDO NA SAÍDA DO SENSOR
#define SENSOR_PIN A0						// PINO DO SENSOR DE CORRENTE ACS712
#define RELAY_PIN D2						// PINO DO MÓDULO RELÉ
#define AP_SSID "AMEE"						// SSID PARA O MODO DE ACCESS POINT
#define AP_PASS "4M33_P455"					// PASSWORD PARA MODO DE ACCESS POINT

EnergyMonitor acs712;			// INSTANCIA O SENSOR DE CORRENTE
ESP8266WebServer server(80);    // INSTANCIA O SERVIDOR NA PORTA 80 (PADRÃO DOS BROWSERS)


/*
 * DADOS INICIAIS DO NODE
 */
String ssid			= "Familiadoremi"	// SSID DA REDE
String pass			= "Deus12345"		// SENHA DA REDE
int id 				= 0;				// ID DO NODE
String ip 			= "";				// IP DO NODE
bool status 		= true;				// STATUS DO EQUIPAMENTO
double lastCurrent 	= 0; 				// ÚLTIMO VALOR DA DE CORRENTE LIDO
double current 		= 0; 				// VALOR DA CORRENTE MÉDIA ATUAL
DATE powerOff    	= 0;				//  TEMPO RESTANTE PARA ELE DESLIGAR

/*
 * ROTAS NODE
 */
void handleSaveCredentials() {
	server.send(200, "text/html", )
}

void handleCreate() {
    EEPROM.begin(512);

	if (EEPROM.read(0) === 1) { // O ID JÄ ESTÁ GRAVADO NA EEPROM
		EEPROM.get(1, ssid);
  		EEPROM.get(1+sizeof(ssid), password);
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
void networkConnect() {
    WiFi.begin(ssid, pass);

	while (WiFi.status() != WL_CONNECTED) {
        delay(100);
    }

	server.begin();

	ip = WiFi.localIP().toString();

    Serial.print("Server em: ");
	Serial.println(ip);

	// ROTAS DO NODE
	server.on("/", HTTP_POST,  handleCreate);
	server.on("/", HTTP_PUT, handleChange);
	server.on("/", HTTP_GET, handleGetNode);
	server.on("/status", HTTP_POST, handleChangeStatus);
	server.on("/current", HTTP_GET, handleGetCurrent);
	server.on("/power-off", HTTP_POST, handleSetPowerOff);
	server.on("/power-off", HTTP_DELETE, handleDestroyPowerOff);
}

/*
 * FUNÇÃO QUE CHECA SE HOUVE UMA PRIMEIRA INICIALIZAÇÃO (SSID, PASS E ID ESTÃO SETADOS NA EEPROM)
 */
bool checkEEPROM() {
	bool r = false;

	EEPROM.begin(4);
	EEPROM.get(0, r);
	EEPROM.end();

	return r;
}

void setup(){
	Serial.begin(115200);
	Serial.flush();
    Serial.println();
	
	if (checkEEPROM()) { // CHECA SE OS DADOS DE SSID E PASSWORD DA REDE ESTÃO SALVOS NA EEPROM
		networkConnect();
	} else { // SE NÃO, CRIA UM PONTO DE ACESSO PARA O USUÁRIO SALVAR
		WiFi.softAP(AP_SSID, AP_PASS);
		server.on("/", handleSaveCredentials);
		server.begin();
	}

	acs712.current(SENSOR_PIN, CURRENT_CAL);
	pinMode(RELAY_PIN, OUTPUT);
}

/*
 * DEFININDO ALGUMAS VARIÁVEIS DE CONTROLE
 */
bool firstLoop = true; // PULAREMOS O PRIMEIRO LOOP DEVIDO O RUÍDO NO BOOT DO NODE
void loop(){
	if (firstLoop) { // CASO ESTEJMOS NO PRIMEIRO LOOP:
		delay(1000);
		firstLoop = false;
		return;
	}

	if (WiFi.status() == WL_CONNECTED) {
		server.handleClient();
	}

	delay(100);
}