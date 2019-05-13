#include <ESP8266WiFi.h>				 // BIBLIOTECA PADRÃO DO MÓDULO WIFI
#include <ArduinoJson.h>				 // BIBLIOTECA PARA USAR DADOS EM JSON (FACILITA NAS REQUISIÇÕES)
#include "./vendors/EmonLib/EmonLib.cpp" // BIBLIOTECA PARA LEITURA DE ENERGIA

#define CURRENT_CAL 195	// VALOR DE CALIBRAÇÃO DO SENSOR DE ENERGIA
#define NOISE 0.25		// RUÍDO PRODUZIDO NA SAÍDA DO SENSOR
#define SENSOR_PIN A0	// PINO DO SENSOR DE CORRENTE ACS712
#define NETWORK_SSID ""   // SSID DA REDE
#define NETWORK_PASS ""	// SENHA DA REDE
#define 

EnergyMonitor acs712;	// INSTANCIA O SENSOR DE CORRENTE
WiFiServer server(80);  // INSTANCIA O SERVIDOR NA PORTA 80 (PADRÃO DOS BROWSERS)

/*
 * FUNÇÃO QUE CONECTA À REDE
 */
void networkConnect() {
	Serial.print("Conectando à rede...");
    WiFi.begin(NETWORK_SSID, NETWORK_PASS);

	while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }

	Serial.println("");
    Serial.println("Conectado!");

	IPAddress ip(192, 168, 7, 7);
    IPAddress gateway(192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);

    Serial.print("Configurando IP fixo para : ");
    Serial.println(ip);

	WiFi.config(ip, gateway, subnet);
    server.begin();

    Serial.print("Server em: ");
    Serial.println(WiFi.localIP());
}

void setup(){  
	Serial.begin(115200);
	Serial.flush();
    Serial.println();
	
	networkConnect();
	acs712.current(SENSOR_PIN, CURRENT_CAL);
}

/*
 * FUNÇÃO QUE CALCULA A CORRENTE MÉDIA USANDO 1000 AMOSTRAS
 * RETORNA A CORRENTE EM mA
 */
double calcCurrent() {
	double totalSum = 0;
	double currentDraw = 0;
	bool firstMeasure = false;

	while (!firstMeasure) { // IGNORA A PRIMEIRA MEDIÇÃO PARA EVITAR O ERRO RUÍDO NO BOOT
		for (int i = 0; i <= 1000; i++) {
			acs712.calcVI(20,100); //FUNÇÃO DE CÁLCULO (20 SEMICICLOS / TEMPO LIMITE PARA FAZER A MEDIÇÃO)
			totalSum += (acs712.Irms - NOSIE); //VARIÁVEL RECEBE O VALOR DE CORRENTE RMS OBTIDO

			if (totalSum < 0) {
				totalSum = 0;
			}

			delay(10);
		}

		currentDraw = totalSum / 1000;
		currentDraw = currentDraw * 1000;

		firstMeasure = true;
	}

	return currentDraw;
}

/*
 * DEFININDO ALGUMAS VARIÁVEIS DE CONTROLE
 */

bool measureCurrent = true; // DEFINE SE O VALOR DA CORRENTE DEVE SER LIDO	
double current = 0; 		// PARA ARMAZENAR O VALOR DA CORRENTE

void loop(){
	if (calcTheCurrent) {
		current = calcCurrent();

		Serial.print("Corrente medida: "); //IMPRIME O TEXTO NA SERIAL
		Serial.print(current); //IMPRIME NA SERIAL O VALOR DE CORRENTE MEDIDA
		Serial.println("mA"); //IMPRIME O TEXTO NA SERIAL
	}

	// WiFiClient client = server.available();

    // if (!client) {
    //     return;
    // }

    // Serial.println("Cliente conectado!");

    // String req = client.readString();

    // DynamicJsonDocument data(200);
    // deserializeJson(data, req.substring(req.indexOf("{")));
}