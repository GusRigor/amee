#include <EEPROM.h>
#include "EmonLib.h" //INCLUSÃO DE BIBLIOTECA

#define espacoEEPROM 1000 //Reserva o espaço da EEPROM
#define CURRENT_CAL 12.70 //VALOR DE CALIBRAÇÃO (DEVE SER AJUSTADO EM PARALELO COM UM MULTÍMETRO MEDINDO A CORRENTE DA CARGA)
const int pinoSensor = A2; //PINO ANALÓGICO EM QUE O SENSOR ESTÁ CONECTADO
float ruido = 0.08; //RUÍDO PRODUZIDO NA SAÍDA DO SENSOR (DEVE SER AJUSTADO COM A CARGA DESLIGADA APÓS CARREGAMENTO DO CÓDIGO NO ARDUINO)

void EEPROMWriteInt(int address, int value); //2 Bytes
int  EEPROMReadInt(int address);
 
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
    Serial.print("Corrente medida: "); //IMPRIME O TEXTO NA SERIAL
    Serial.print(currentDraw); //IMPRIME NA SERIAL O VALOR DE CORRENTE MEDIDA
    Serial.println("A"); //IMPRIME O TEXTO NA SERIAL

    Serial.print("Potencia calculada: "); //IMPRIME O TEXTO NA SERIAL
    Serial.print(currentDraw*110); ////IMPRIME NA SERIAL O VALOR DA CORRENTE MEDIDA * A TENSÃO NOMINAL DA TOMADA
    Serial.println("W"); //IMPRIME O TEXTO NA SERIAL
}

void EEPROMWriteInt(int address, int value) {
   byte hiByte = highByte(value);
   byte loByte = lowByte(value);

   EEPROM.write(address, hiByte);
   EEPROM.write(address + 1, loByte);   
}

int EEPROMReadInt(int address) {
   byte hiByte = EEPROM.read(address);
   byte loByte = EEPROM.read(address + 1);
   
   return word(hiByte, loByte); 
}
