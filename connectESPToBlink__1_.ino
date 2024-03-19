
#define BLYNK_PRINT Serial

// #define BLYNK_TEMPLATE_ID "TMPL267NMVsc_"
// #define BLYNK_TEMPLATE_NAME "Quickstart Template"
// #define BLYNK_AUTH_TOKEN "IYBvRpEnE_2EvkxKHsJcmHUvimeczRIZ"

#define BLYNK_TEMPLATE_ID "TMPL2dGH2zWNI"
#define BLYNK_TEMPLATE_NAME "Horta Inteligente"
#define BLYNK_AUTH_TOKEN "rV9_S-0PZZbi1zJXctG07zQ4RAjvr-zE"

#include <DHT11.h>
#include <Blynk.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// char ssid[] = "Eu";
// char pass[] = "Ricardo10";

char ssid[] = "Alunos"; char pass[] = "Alunos@123";

#define dht11Port 25 //
DHT11 dht(dht11Port); //

float temp; // -- DHT
float humid; // -- DHT
int valorumidade;
void modDHT11() {
  temp = dht.readTemperature();
  humid = dht.readHumidity();
  
  if (temp != DHT11::ERROR_CHECKSUM && temp != DHT11::ERROR_TIMEOUT){
  valorumidade = map(humid, 1023, 315, 0, 100); //Transforma os valores analógicos em uma escala de 0 a 100

    Blynk.virtualWrite(V0, temp);
    Blynk.virtualWrite(V1, valorumidade);
    // if(temp >= 27){
    //   digitalWrite(vent, LOW);
    //   delay(7000);
    //   digitalWrite(vent, HIGH);
    // }
  }else{
    Serial.println(DHT11::getErrorString(temp));
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();
  modDHT11();

  Serial.print(temp);
  Serial.print("||");
  Serial.print(humid); 
  Serial.print("||");
  Serial.print(valorumidade); 
  Serial.print("||");
  delay(500);
}
