//BIBLIOTECA PARA LEITURA DE ENERGIA
#include "./vendors/EmonLib/EmonLib.cpp"

//VALOR DE CALIBRAÇÃO DO SENSOR DE ENERGIA
#define CURRENT_CAL 195

//RUÍDO PRODUZIDO NA SAÍDA DO SENSOR
#define RUIDO 0.25

const int pinoSensor = A0;

EnergyMonitor acs712;

void setup(){  
	Serial.begin(115200);

	//PASSA PARA A FUNÇÃO OS PARÂMETROS (PINO ANALÓGIO / VALOR DE CALIBRAÇÃO)
	acs712.current(pinoSensor, CURRENT_CAL);
}

void loop(){
	double totalSum = 0;
	double currentDraw = 0;

	for (int i = 0; i <= 300; i++) {
		acs712.calcVI(20,100); //FUNÇÃO DE CÁLCULO (20 SEMICICLOS / TEMPO LIMITE PARA FAZER A MEDIÇÃO)
		totalSum += (acs712.Irms - RUIDO); //VARIÁVEL RECEBE O VALOR DE CORRENTE RMS OBTIDO

		if (totalSum < 0) {
			totalSum = 0;
		}

		delay(10);
	}

	currentDraw = totalSum / 300;
	currentDraw = currentDraw * 1000;

	Serial.print("Corrente medida: "); //IMPRIME O TEXTO NA SERIAL
	Serial.print(currentDraw); //IMPRIME NA SERIAL O VALOR DE CORRENTE MEDIDA
	Serial.println("mA"); //IMPRIME O TEXTO NA SERIAL
}