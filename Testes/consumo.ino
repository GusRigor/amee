
#include "EmonLib.h" //INCLUSÃO DE BIBLIOTECA

#define CURRENT_CAL 12.50 //VALOR DE CALIBRAÇÃO (DEVE SER AJUSTADO EM PARALELO COM UM MULTÍMETRO MEDINDO A CORRENTE DA CARGA)
#define tensao 110
const int pinoSensor = A2; //PINO ANALÓGICO EM QUE O SENSOR ESTÁ CONECTADO
float ruido = 0.08; //RUÍDO PRODUZIDO NA SAÍDA DO SENSOR (DEVE SER AJUSTADO COM A CARGA DESLIGADA APÓS CARREGAMENTO DO CÓDIGO NO ARDUINO)
unsigned long d_tempo = 0;
double consumo;
double consumo_Wh;

EnergyMonitor emon1; //CRIA UMA INSTÂNCIA
 
void setup(){  
  Serial.begin(9600); //INICIALIZA A SERIAL
  emon1.current(pinoSensor, CURRENT_CAL); //PASSA PARA A FUNÇÃO OS PARÂMETROS (PINO ANALÓGIO / VALOR DE CALIBRAÇÃO)
}
 
void loop(){
  emon1.calcVI(20,100); //FUNÇÃO DE CÁLCULO (20 SEMICICLOS / TEMPO LIMITE PARA FAZER A MEDIÇÃO)
  double currentDraw = emon1.Irms; //VARIÁVEL RECEBE O VALOR DE CORRENTE RMS OBTIDO
  currentDraw = currentDraw-ruido; //VARIÁVEL RECEBE O VALOR RESULTANTE DA CORRENTE RMS MENOS O RUÍDO
  
  if(currentDraw < 0){ //SE O VALOR DA VARIÁVEL FOR MENOR QUE 0, FAZ 
      currentDraw = 0; //VARIÁVEL RECEBE 0
  }

  if(millis()- d_tempo > 1000){
    consumo += (currentDraw*tensao)/3600000; //CONSUMO PELA VARIAÇÃO DE UM SEGUNDO. CONVERSÃO PARA kWh
    consumo_Wh += (currentDraw*tensao)/3600; //CONSUMO PELA VARIAÇÃO DE UM SEGUNDO. CONVERSÃO PARA Wh
    d_tempo = millis();
    Serial.println("Rodei - consumo"); 
  }

  
    Serial.print("Corrente medida: "); //IMPRIME O TEXTO NA SERIAL
    Serial.print(currentDraw); //IMPRIME NA SERIAL O VALOR DE CORRENTE MEDIDA
    Serial.println("A"); //IMPRIME O TEXTO NA SERIAL

    Serial.print("Potencia calculada: "); //IMPRIME O TEXTO NA SERIAL
    Serial.print(currentDraw*tensao); ////IMPRIME NA SERIAL O VALOR DA CORRENTE MEDIDA * A TENSÃO NOMINAL DA TOMADA
    Serial.println("W"); //IMPRIME O TEXTO NA SERIAL 

    Serial.print("Consumo: "); //IMPRIME O TEXTO NA SERIAL
    Serial.print(consumo_Wh); ////IMPRIME NA SERIAL O VALOR DA CORRENTE MEDIDA * A TENSÃO NOMINAL DA TOMADA
    Serial.println("Wh");

    Serial.print("Consumo: "); //IMPRIME O TEXTO NA SERIAL
    Serial.print(consumo); ////IMPRIME NA SERIAL O VALOR DA CORRENTE MEDIDA * A TENSÃO NOMINAL DA TOMADA
    Serial.println("kWh");

     
}

