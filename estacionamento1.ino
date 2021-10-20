/*------------------------ BIBLIOTECAS ------------------------*/

extern "C"{
#include "user_interface.h" // Biblioteca TIMER por software
}
#include <SPI.h> // Biblioteca SPI
#include <MFRC522.h> // Biblioteca Leitor RF RC522
#include <ESP8266WiFi.h> // Biblioteca do Módulo Wifi
#include <ESP8266SMTP.h> // Biblioteca do Modo SMTP para enviar email
#include <Servo.h>  
/*------------------------ DEFINES ------------------------*/

#define RST_PIN    D3    // Define o pino reset do leitor RF
#define SS_PIN     D8 

/*------------------------ OBJETOS ------------------------*/
MFRC522 mfrc522(SS_PIN, RST_PIN); //Cria um objeto leitor de Cartao RF
Servo servo; // Declara o objeto do tipo servo


os_timer_t Timer1; // Variavel do timer
os_timer_t Timer2; // Variavel do timer

/*------------------------ VARIAVEIS GLOBAIS ------------------------*/
bool       usuario1 = false;
bool       usuario2 = false;
int        valor1=0; // Valor a ser cobrado do primeiro usuario cadastrado
int        valor=0;
int        valor2=0;
const char* ssid = "nomedarede";                // Nome do Wifi
const char* password = "senha";           // Senha Wifi
uint8_t connection_state = 0;           // Variavel auxiliar para controlar estado da conecxão
uint16_t reconnect_interval = 10000;    // tempo de 10 segundos para uma reconexão

String email = "";

String nome="";


/*------------------------ ESCOPO DAS FUNÇÕES ------------------------*/
void enviaEmail(String email,String nome);
void cancelaSobe(); // Declaração da função que controla a cancela
void cancelaDesce(); // Declaração da função que controla a cancela


void tCallback(void *tCall);
void usrInit(void);
void tCallback2(void *tCall);
void usrInit2(void);
/*------------------------ CONFIGURAÇÃO DO TIMER   ------------------------*/
void tCallback(void *tCall){ // Função de chamada da interrupção
  if(usuario1==true){ // Se o usuario1 estiver no estacionamento some o valor 
   valor1 = valor1 + 10;
   }

  
    }


void tCallback2(void *tCall){ // Função de chamada da interrupção
  if(usuario2==true){ // Se o usuario1 estiver no estacionamento some o valor 
   valor2 = valor2 + 10;
   }

  
    }



void usrInit(void){  // Interrupção do usuario 1 - TImer 1
    os_timer_setfn(&Timer1, tCallback, NULL);
    os_timer_arm(&Timer1, 10000, true); // timer conta ate 10 segundos
}

void usrInit2(void){  // Interrupção do usuario 2 - TImer 2
    os_timer_setfn(&Timer2, tCallback2, NULL);
    os_timer_arm(&Timer2, 10000, true); // timer conta ate 10 segundos
}

/*----------------------------- CONECTANDO AO WIFI -----------------------------*/
uint8_t WiFiConnect(const char* ssID, const char* nPassword)
{
  static uint16_t attempt = 0;
  Serial.print("Conectando em:   ");
  Serial.println(ssID);
  WiFi.begin(ssID, nPassword);  // COnecta ao wifi usando a biblioteca do WIFI

  uint8_t i = 0;
  while(WiFi.status() != WL_CONNECTED && i++ < 50) {  // Faz o controle da concexão, tempo e etc
    delay(200);
    
  }
  ++attempt;
  Serial.println("");
  if(i == 51) {
    Serial.print(F("Tempo de Conexão estourou na tentativa numero: "));
    Serial.println(attempt);
    if(attempt % 2 == 0)
      Serial.println(F("Verifique se o ponto de acesso esta disponível ou SSID e senha estão corretos\r\n"));
    return false;
  }
  Serial.println(F("Conexão estabilizada"));
  Serial.print(F("Seu endereço IP: "));
  Serial.println(WiFi.localIP());
  return true;
}

void Awaits(uint16_t interval)
{
  uint32_t ts = millis();
  while(!connection_state){
    delay(50);
    if(!connection_state && millis() > (ts + interval)){
      connection_state = WiFiConnect(ssid, password);
      ts = millis();
    }
  }
}

/*----------------------------- SETUP -----------------------------*/

void setup()
{

  pinMode(16, OUTPUT);
  pinMode(4, OUTPUT);
  Serial.begin(9600);
 servo.attach(D1); // PWM saindo do pino D1

  servo.write(70); // Inicia o servo em 70 // Abaixada
  SPI.begin();      // Inicia  SPI bus
  mfrc522.PCD_Init();   // Inicia o lietor de cartao MFRC522
  usrInit(); // Inicia a interrupção do timer1 do usuario 1
   usrInit2(); // Inicia a interrupção do timer2 do usuario 2
  delay(2000);
 
  
  connection_state = WiFiConnect(ssid, password);

  if(!connection_state) {         // Se nao esta conectado ao wifi
    Awaits(reconnect_interval);         // constantly trying to connect
  }

Serial.println("Aproxime o seu cartao do leitor...");  // Se wifi conectado e estavel aproxime emite a mensagem para aproximar o cartao RFID
Serial.println();

  
}

void loop()
{
  // Procura por cartao RFID
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Seleciona o cartao RFID
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }

  String conteudo= "";
  byte letra;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
   //  Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");  // Printa UID do cartao rfid
   //  Serial.print(mfrc522.uid.uidByte[i], HEX);                  // Printa UID
     conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ")); // Adiciona o valor lido numa string conteudo lido para uma string como  0 ou em branco
     conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));               //// // Concateta conteudo lido para uma string em Hexadecimal
  }
  Serial.println();
  Serial.print("Mensagem : ");
  conteudo.toUpperCase();
  


   

   if((conteudo.substring(1)=="33 16 53 73")&&(usuario1==false)){ //UID 2 - Cartao chaveiro campus

    Serial.println("Seja bem-vindo NOme !");
    Serial.println();
     cancelaSobe();
    delay(3000);
     cancelaDesce();
     usuario1=true;
    
    }
    else if((conteudo.substring(1)=="33 16 53 73")&&(usuario1==true)){
      Serial.print(" Volte sempre, será enviado um e-mail com o recibo do valor  cobrado de seu cartao de crédito. ");
        cancelaSobe();
    valor=valor1; // Atualiza o valor do usuario 1 para mandar o e-mail
    email="email@gmail.com";
    nome= "nome";
    enviaEmail(email,nome);
       cancelaDesce();
       valor1=0;
       valor=0;
       usuario1=false;
      

    
        
      
      }

     else if((conteudo.substring(1)=="90 C1 64 A3")&&(usuario2==false)){ //UID 3 - Cartao chaveiro pendrive

    Serial.println("Seja bem-vindo nome ! ");
    Serial.println();
     cancelaSobe();
     delay(3000);
     cancelaDesce();
     usuario2=true;
    
    
    }

    else if((conteudo.substring(1)=="90 C1 64 A3")&&(usuario2==true)){
 Serial.print(" Volte sempre, será enviado um e-mail com o recibo do valor cobrado de seu cartao de crédito. ");
       cancelaSobe();
       valor=valor2;
    email="email";
    nome= " Marco";
    enviaEmail(email,nome);
     cancelaDesce();
      valor2=0;
       valor=0;
       
       usuario2=false;
       
       
      
      }
  else{
    Serial.println("Cliente Desconhecido!");
    Serial.println();
     delay(3000);
    
    }  
  
  
  
  }



void enviaEmail(String email, String nome){
  
    
  SMTP.setEmail("email@gmail.com") // Email que ira mandar o e-mail  // Com as configurações SMTP realizadas no e-mail
    .setPassword("senha")  // Senha do e-mail que ira mandar o e-mail
    .Subject("Entrada no Estacionamento") // Assunto
    .setFrom("Estacionamento LabArq") // Nome do remetente
    .setForGmail();           // simply sets port to 465 and setServer("smtp.gmail.com");           
                                 
  if(SMTP.Send(email, nome+" Foi cobrado em seu cartão de crédito o valor de R$ "+valor)) {  // Emails separados por virgula e texto a ser enviado.
    Serial.println(F("E-mail enviado!"));
  } else {
    Serial.print(F("Erro ao Enviar e-mail: "));
    Serial.println(SMTP.getError());
  } 
  
  
  }


  /*------------------  FUNCÇÕES DE CONTROLE SERVO ------------------------------*/

void cancelaSobe(){
  
   servo.write(0); // Faz a cancela subir
    digitalWrite(4, LOW);
   digitalWrite(16, HIGH);
  
  }


  void cancelaDesce(){
     servo.write(70); // Faz a cancela descer
      digitalWrite(4, HIGH); // D2
   digitalWrite(16, LOW);  // D0
    
    }

// caso queira saber mais entre em contato comigo!☺ https://www.facebook.com/marco1825 ou https://www.linkedin.com/in/marco-ant%C3%B4nio-5a420418a/
 
