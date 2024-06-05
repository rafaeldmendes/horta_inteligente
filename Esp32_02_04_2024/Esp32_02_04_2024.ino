// ----- Definições
#define BLYNK_PRINT Serial
#define bomba 16
#define BOTAO_BOMBA V3
#define ventilador 17
#define BOTAO_VENTILADOR V4
#define umidSoloAnalogPort 32

// ----- Credenciais do Blynk -----
#define BLYNK_TEMPLATE_ID "TMPL2dGH2zWNI"
#define BLYNK_TEMPLATE_NAME "Horta Inteligente"
#define BLYNK_AUTH_TOKEN "rV9_S-0PZZbi1zJXctG07zQ4RAjvr-zE"

// ----- Importações de bibliotecas -----
#include <DHT.h>
#include <Blynk.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

DHT dht(33, DHT11);

// ----- Variaveis Globais
// char ssid[] = "Alunos"; char pass[] = "Alunos@123";
char ssid[] = "Eu";
char pass[] = "Ricardo10";
float temp;  // -- DHT temperatura
float umid;  // -- DHT umidade
int valorumidade;
int valorumidade2;  // Umidade Solo

void modDHT11() {
  temp = dht.readTemperature();
  umid = dht.readHumidity();

  if (isnan(temp) || isnan(umid)) {
    Serial.println("Erro na leitura do DHT");
  } else {
    Blynk.virtualWrite(V0, temp);
    Blynk.virtualWrite(V1, umid);
  }
}

void modLM393() {
  Serial.print(" VALOR modLM393 ========>>>> ");  //Imprime no monitor serial o valor de umidade em porcentagem

  // -- Função Responsavel pela leitura dos dados de temperatura e umidade relativa do solo.

  valorumidade2 = analogRead(umidSoloAnalogPort);  //Realiza a leitura analógica do sensor e armazena em valorumidade
  Serial.print(" VALOR INICIAL ========>>>> ");    //Imprime no monitor serial o valor de umidade em porcentagem
  Serial.print(valorumidade2);                     //Imprime no monitor serial o valor de umidade em porcentagem


  valorumidade2 = map(valorumidade2, 2000, 900, 0, 100);  //Transforma os valores analógicos em uma escala de 0 a 100

  Serial.print("||");
  Serial.print(valorumidade2);  //Imprime no monitor serial o valor de umidade em porcentagem

  Blynk.virtualWrite(V2, valorumidade2);


  if (valorumidade2 < 40) {  //Se esse valor for igual a 0, será mostrado no monitor serial que o solo está úmido e o led verde se acende
    Serial.print("||");
    Serial.println("seco");
  } else if ((valorumidade2 >= 40) && (valorumidade2 <= 69)) {
    Serial.print("||");
    Serial.println("umido");
  } else if (valorumidade2 >= 70) {
    Serial.print("||");
    Serial.println("Molhado");
  }
}

void setup() {
  Serial.begin(9600);  // a declaração do serial tem que vir sempre em primeiro
  // put your setup code here, to run once:
  pinMode(bomba, OUTPUT);
  pinMode(ventilador, OUTPUT);
  pinMode(umidSoloAnalogPort, INPUT);
  dht.begin();
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();
  modDHT11();
  modLM393();  // Sensor Umidade Solo
  delay(500);
}

BLYNK_WRITE(BOTAO_BOMBA) {
  int estado = param.asInt();
  Serial.print("Bomba");
  Serial.print(estado);
  if (estado == 1) {
    digitalWrite(bomba, HIGH);
  } else {
    digitalWrite(bomba, LOW);
  }
}

BLYNK_WRITE(BOTAO_VENTILADOR) {
  int estado = param.asInt();
  Serial.print("Ventilador");
  Serial.print(estado);
  if (estado == 1) {
    digitalWrite(ventilador, HIGH);
  } else {
    digitalWrite(ventilador, LOW);
  }
}