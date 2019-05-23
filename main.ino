#include <ESP8266WiFi.h>				 // BIBLIOTECA PADRÃO DO MÓDULO WIFI
#include <ESP8266WebServer.h>			 // BIBLIOTECA PARA WEB SERVER 
#include "./vendors/EmonLib/EmonLib.cpp" // BIBLIOTECA PARA LEITURA DE ENERGIA
#include "./routes.cpp"					 // ROTAS DO SERVIDOR

#define CURRENT_CAL 195						// VALOR DE CALIBRAÇÃO DO SENSOR DE ENERGIA
#define NOISE 0.25							// RUÍDO PRODUZIDO NA SAÍDA DO SENSOR
#define SENSOR_PIN A0						// PINO DO SENSOR DE CORRENTE ACS712
#define RELAY_PIN D2						// PINO DO MÓDULO RELÉ
#define NETWORK_SSID "Familiadoremi"   		// SSID DA REDE
#define NETWORK_PASS "Deus12345"			// SENHA DA REDE

EnergyMonitor acs712;			// INSTANCIA O SENSOR DE CORRENTE
ESP8266WebServer server(80);    // INSTANCIA O SERVIDOR NA PORTA 80 (PADRÃO DOS BROWSERS)

/*
 * DADOS INICIAIS DO NODE
 */
int id 				= 0;
String ip 			= "";
bool status 		= true;
double lastCurrent 	= 0; // ARMAZENA O ÚLTIMO VALOR DA DE CORRENTE LIDO
double current 		= 0; // ARMAZENA O VALOR DA CORRENTE MÉDIA ATUAL
// DATE powerOff    	= 0;		


/*
 * FUNÇÃO QUE CONECTA À REDE
 */
void networkConnect() {
    WiFi.begin(NETWORK_SSID, NETWORK_PASS);

	while (WiFi.status() != WL_CONNECTED) {
        delay(100);
    }

	server.begin();

	ip = WiFi.localIP().toString();

    Serial.print("Server em: ");
	Serial.println(ip);

	// ROTAS DO NODE
	server.on("/", HTTP_POST,  handleCreate);
	// server.on("/", HTTP_PUT, handleChange);
	// server.on("/", HTTP_GET, handleGetNode);
	// server.on("/status", HTTP_POST, handleChangeStatus);
	// server.on("/current", HTTP_GET, handleGetCurrent);
	// serevr.on("/power-off", HTTP_POST, handleSetPowerOff);
	// server.on("/power-off", HTTP_DELETE, handleDestroyPowerOff);
}

void handleCreate() {
    Serial.println("ok");
}

void setup(){  
	Serial.begin(115200);
	Serial.flush();
    Serial.println();
	
	networkConnect();

	acs712.current(SENSOR_PIN, CURRENT_CAL);
	pinMode(RELAY_PIN, OUTPUT);
}

/*
 * DEFININDO ALGUMAS VARIÁVEIS DE CONTROLE
 */
bool firstLoop = true;		// PULAREMOS O PRIMEIRO LOOP DEVIDO O RUÍDO NO BOOT DO NODE
void loop(){
	if (firstLoop) { // CASO ESTEJMOS NO PRIMEIRO LOOP:
		delay(1000);
		firstLoop = false;
		return;
	}

	server.handleClient();

	Serial.println("Se funcionar aqui é sucesso!!");
	delay(100);

	// WiFiClient client = server.();
	// while (!client) { // ENQUANTO NÃO HOUVER NENHUMA REQUIÇÃO:
	// 	if (status) { // SE O EQUIPAMENTO ESTIVER LIGADO:

	// 		Serial.println("Lendo Corrente...");
	// 		acs712.calcVI(20, 100); //FUNÇÃO DE CÁLCULO (20 SEMICICLOS / TEMPO LIMITE PARA FAZER A MEDIÇÃO)

	// 		current = (acs712.Irms - NOISE); //VARIÁVEL RECEBE O VALOR DE CORRENTE RMS OBTIDO

	// 		if (current < 0) {
	// 			current = 0;
	// 		}

	// 		if (lastCurrent > 0) {
	// 			current = ((current + lastCurrent) / 2);
	// 		}

	// 		lastCurrent = current;
	// 	}

	// 	delay(10);
	// 	client = server.available();
	// }

	// handleClientReq(client.readString());
	// client.stop();
}