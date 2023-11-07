
// -- Bibliotecas importadas
#include <SoftwareSerial.h> // Bluetooth
#include <DHT11.h> // Sensor de Temperatura / Humidade de Ar
#include <Wire.h>
#include <LiquidCrystal_I2C.h> // LCD

// -- Definições de portas
#define umidSoloAnalogPort A0 //Atribui o pino A0 (Umidade do Solo) a variável umidade - leitura analógica do sensor - 
#define rxBT 0
#define txBT 1
#define dht11Port 2
#define bombAgua 10
#define vent 12
#define end 0x27
#define col 16
#define lin 2

// -- Instancias das Bibliotecas
SoftwareSerial bt(rxBT, txBT);
DHT11 dht(dht11Port);
LiquidCrystal_I2C lcd(end, col, lin);

// -- Variaveis Globais
float temp; // -- DHT
float humid; // -- DHT
int valorumidade; // -- Declaração da variável que armazenará o valor da umidade lida - saída analogica
int valorumidadeArDigital; // -- Declaração da variável que armazenara a saída digital do sensor de umidade do solo
String status;

// -- Função Responsavel pela leitura dos dados de temperatura e umidade relativa do ar.
void modLM393(){
  valorumidade = analogRead(umidSoloAnalogPort); //Realiza a leitura analógica do sensor e armazena em valorumidade
  valorumidade = map(valorumidade, 1023, 315, 0, 100); //Transforma os valores analógicos em uma escala de 0 a 100

  //Se esse valor for igual a 0, será mostrado no monitor serial que o solo está úmido e o led verde se acende
  if (valorumidade < 40) {
    status = "Seco";
    digitalWrite(bombAgua, LOW);
    delay(7000);
    digitalWrite(bombAgua, HIGH);
  } else if((valorumidade >=40) && (valorumidade <=69)){
    status = "Umido";
  } else if(valorumidade >=70){
    status = "Molhado";
  }
}

// -- Função Responsavel pela leitura dos dados de temperatura e umidade relativa do ar.
void modDHT11() {
  temp = dht.readTemperature();
  humid = dht.readHumidity();
  
  if (temp != DHT11::ERROR_CHECKSUM && temp != DHT11::ERROR_TIMEOUT){
    if(temp >= 27){
      digitalWrite(vent, LOW);
      delay(7000);
      digitalWrite(vent, HIGH);
    }
  }else{
    Serial.println(DHT11::getErrorString(temp));
  }
}

void modLCD(){
  lcd.clear(); // LIMPA O DISPLAY

  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print(" ");
  lcd.print((char)223);
  lcd.print("C");
  lcd.setCursor(0, 1); // POSICIONA O CURSOR NA PRIMEIRA COLUNA DA LINHA 2
  lcd.print("Umidade AR: ");
  lcd.print(humid);
  lcd.print("%");
  delay(5000);
  lcd.clear(); // LIMPA O DISPLAY

  lcd.print("Umid Solo: ");
  lcd.print(valorumidade);
  lcd.print("%");
  lcd.setCursor(0, 1); // POSICIONA O CURSOR NA PRIMEIRA COLUNA DA LINHA 2
  lcd.print("Status: ");
  lcd.print(status);
}

void setup(){
  Serial.begin(9600); // -- Inicia comunicação com o serial
  bt.begin(9600); // -- Inicia comunicação com o bluetooth
  lcd.init(); // -- Inicia a comunicação com o display
  lcd.backlight(); // -- Liga a iluminação do display
  pinMode(vent, OUTPUT); // -- Define ventuinha como saida
  pinMode(bombAgua, OUTPUT); // -- Define bomba dagua como saida
  digitalWrite(vent, HIGH);
  digitalWrite(bombAgua, HIGH);
}

void loop() {
  modDHT11(); // -- Sensor de Temperatura e umidade do ar
  modLM393(); // -- Sensor Umidade Solo
  modLCD(); // -- Display LCD Cod.
  Serial.print(temp);
  Serial.print("||");
  Serial.print(humid); 
  Serial.print("||");
  Serial.print(valorumidade); //Imprime no monitor serial o valor de umidade em porcentagem
  Serial.print("||");
  Serial.println(status);
  delay(5000);
}
