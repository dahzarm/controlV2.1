#include <Arduino.h>
#include <Scheduler.h>

/*Versão 2.1 de controladora de catraca
 * 
 * Utilização
 * 
 * D1 | 5  -> aciona solenoide 1
 * D2 | 4  -> aciona solenoide 2
 * D6 | 12  -> entrada de sensor de giro 1
 * D5 | 14  -> entrada de sensor de giro 2
 * SD3| 10 -> entrada pulso 1 para passagem
 * SD2| 9  -> entrada pulso 2 para passagem
 * 
 */

#define ledPin BUILTIN_LED
#define bobina1 5      //   D1
#define bobina2 4      //   D2
#define sensor1 12     //   D6      0      //   D3  
#define sensor2 14     //   D5
#define comando1 10    //   SD3
#define comando2 9     //   SD2
#define buzzer 16       //   USER WAKE
#define serialSpeed 115200  


// variaveis auxiliares para armazenar estado das GPIOs
uint8_t varCom1 = 0;
uint8_t varCom2 = 0;
uint8_t varSen1 = 0;
uint8_t varSen2 = 0;


int byPass = 6000;                //tempo de passagem 
uint8_t flagLed = 0;              //mostra o estado do led
uint8_t sentido = 0;              //sentido de giro da catraca                    0 = acionamento 1 entrada     1 = acionamento 2 entrada
uint8_t numSensores = 0;          //quantas entradas de sensores terá a catraca   0 = 1 acionamento 1 = 2 acionamentos
uint8_t numAcionamentos = 0;      //quantas bobinas diferentes terá a catraca     0 = 1 bobina      1 = 2 bobinas      

/*
 * 
 *      TAREFA PARALELA ------------------------------------------------------------------------------
 * 
 */

class PrintBlinkLed : public Task {
protected:
    void setup() {
        pinMode(ledPin, OUTPUT);      // Configuração do pino led
      
    }
    void loop()  {

        
        digitalWrite(ledPin, LOW);   // Desaciona led builtin
        flagLed = 0;
        delay(1000);
        digitalWrite(ledPin, HIGH);   // Aciona led builtin
        flagLed = 1;
        delay(1000);
    }
} blinkLed_task;

/*
 * 
 * TAREFA PRINCIPAL DE CONTROLE DA CATRACA -----------------------------------------------------
 * 
 */


class ControleCatraca : public Task {
protected:
    void setup() {
      
        Serial.begin(serialSpeed);    // Adicionado posteriormente
        startPinMode();
      
    }
    void loop(){
      //Serial.print("Comando 1: ");
 // Serial.print(com1);

  varCom1 = digitalRead(comando1);
  varCom2 = digitalRead(comando2);
  varSen1 = digitalRead(sensor1);
  varSen2 = digitalRead(sensor2);

  digitalWrite(bobina1, HIGH);   // desliga a bobina 1
  digitalWrite(bobina2, HIGH);   // desliga a bobina 2

  // teste para acionamento do comando 1 que deixa a passagem livre pelo sensor 1
  if(varCom1)
    {
      desacionaBobina(1);
    
    }

   // teste para acionamento do comando 2 que deixa a passagem livre pelo sensor 2 
   if(varCom2)
    {
      desacionaBobina(2);
    }

  // Leitura dos sensores, acionamento das bobinas especificas e desacionametno caso o sensor desative ou acione pelo varCom1 ou varCom2

   while(varSen1)
    {
      varSen1 = digitalRead(sensor1);
      varCom1 = digitalRead(comando1);
      Serial.print("Ativado sensor 1 e acionando a bobina 1..\n");
      digitalWrite(bobina1, LOW);   // Aciona a bobina 1
      delay(500);
      if(varCom1)
        {
           digitalWrite(bobina1, HIGH);   // Desliga a bobina 1
           Serial.print("Acesso pelo comando 1 dentro do campo de leitura do sensor..\n");
           delay(byPass);
           resetPorts();
           break;
      }
    }
    
   while(varSen2)
    {
      varSen2 = digitalRead(sensor2);
      varCom2 = digitalRead(comando2);
      Serial.print("Ativado sensor 2 e acionando a bobina 2..\n");
      digitalWrite(bobina2, LOW);   // Liga a bobina 2
      delay(500);
      if(varCom2)
        {
           digitalWrite(bobina2, HIGH);   // Desliga a bobina 2
           Serial.print("Acesso pelo comando 2 dentro do campo de leitura do sensor..\n");
           delay(byPass);
           resetPorts();
           break;
        }
      
      }
      
  digitalWrite(bobina1, HIGH);   // desliga a bobina 1
  digitalWrite(bobina2, HIGH);   // desliga a bobina 2
      
      }
      
 /*
 * Rotina para desacionar as bobinas e gerar um delay para a passagem da pessoa
 * 
 */

void desacionaBobina(int num){

  //junção da string com o numero da bobina selecionada, ex: bobina2
  int auxCont = 0;
  uint8_t flagSen1 = 0;
  uint8_t flagSen2 = 0;
  
  String stringAux;
  stringAux = "bobina";
  stringAux = stringAux + num;
  if(stringAux == "bobina1") {
    digitalWrite(bobina1, HIGH);   // Desliga a bobina 1
    Serial.print("Acesso pelo comando 1, desligada a bobina 1..\n");
    }
  if(stringAux == "bobina2"){
    digitalWrite(bobina2, HIGH);   // Desliga a bobina 2
    Serial.print("Acesso pelo comando 2, desligada a bobina 2..\n");
    }
  resetPorts();       // limpa flags das variaveis 


  
  // Controle do tempo de passagem e do buzzer
  while( auxCont <= byPass) {       // compara a variavel auxiliar com o tempo de byPass
    varSen2 = digitalRead(sensor2);
    varSen1 = digitalRead(sensor1);
    //Acrescenta o acionamento do buzzer na passagem
    digitalWrite(buzzer, HIGH);     
    Serial.print("Acionando buzzer..\n");
    delay(500);
    digitalWrite(buzzer, LOW);
    Serial.print("Desligando buzzer..\n");
    delay(500);
    auxCont = auxCont + 1500;

    //Verifica se passou pelos sensores no giro
    if(varSen1 != 0){
        flagSen1 = 1;
        Serial.print("Flag 1 ativado..\n");
      }
    if(varSen2 != 0){
        flagSen2 = 1;
        Serial.print("Flag 2 ativado..\n");
      }

    if(flagSen1 and flagSen2) {
      delay(1000);
      Serial.print("Reset do acionamento..\n");
      resetPorts();
      break;
      }
        
    }
  }

/*
 * 
 * SETAR OS PINOS COMO ENTRADA OU SAIDA
 */

void startPinMode(){
   
  pinMode(ledPin, OUTPUT);    
  pinMode(bobina1, OUTPUT);
  pinMode(bobina2, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(sensor1, INPUT);
  pinMode(sensor2, INPUT);
  pinMode(comando1, INPUT);
  pinMode(comando2, INPUT);  
  }
/*
 * 
 * RESET DOS BUFFERS DAS VARIAVEIS       *************
 * 
 */

void resetPorts()
  {
      varCom2 = 0;
      varCom1 = 0;
      varSen1 = 0;
      varSen2 = 0;
  }
private:
    uint8_t state;
} controle_task;

/*
 * 
 *   START PROGRAMA PRINCIPAL --------------------------------------------------------------------------------------------------
 * 
 */


void setup() {

  
  Serial.println("");

  delay(1000);

  Scheduler.start(&blinkLed_task);
  Scheduler.start(&controle_task);
 // Scheduler.start(&mem_task);

  Scheduler.begin();


   
}

/*
 * 
 *      LOOP PRINCIPAL
 */
   
void loop() { }



