// ----- Definições
#define BLYNK_PRINT Serial
#define bomba 16
#define BOTAO_BOMBA V3
#define ventilador 17
#define BOTAO_VENTILADOR V4
// ----- Credenciais do Blynk -----
#define BLYNK_TEMPLATE_ID "TMPL2dGH2zWNI"
#define BLYNK_TEMPLATE_NAME "Horta Inteligente"
#define BLYNK_AUTH_TOKEN "rV9_S-0PZZbi1zJXctG07zQ4RAjvr-zE"

// ----- Importações de bibliotecas -----
#include <DHT11.h>
#include <Blynk.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

DHT11 dht(14);  //

// ----- Variaveis Globais
float temp;  // -- DHT temperatura
float umid;  // -- DHT umidade
int valorumidade;

void modDHT11() {
  temp = dht.readTemperature();
  umid = dht.readHumidity();

  if (temp != DHT11::ERROR_CHECKSUM && temp != DHT11::ERROR_TIMEOUT) {
    valorumidade = map(umid, 4095, 0, 0, 100);  //Transforma os valores analógicos em uma escala de 0 a 100
    Blynk.virtualWrite(V0, temp);
    Blynk.virtualWrite(V1, valorumidade);
  } else {
    Serial.println(DHT11::getErrorString(temp));
  }
}

void setup() {
  // put your setup code here, to run once:
  pinMode(bomba, OUTPUT);
  pinMode(ventilador, OUTPUT);
  Blynk.begin(BLYNK_AUTH_TOKEN, "Alunos", "Alunos@123");
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();
  modDHT11();

  Serial.print(temp);
  Serial.print("        ");
  // Serial.print(umid);
  // Serial.print("||");
  // Serial.print(valorumidade);
  // Serial.print("||");
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