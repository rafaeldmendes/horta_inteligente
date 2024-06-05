// ----- Credenciais do Blynk -----
#define BLYNK_TEMPLATE_ID "TMPL2dGH2zWNI"
#define BLYNK_TEMPLATE_NAME "Horta Inteligente"
#define BLYNK_AUTH_TOKEN "rV9_S-0PZZbi1zJXctG07zQ4RAjvr-zE"

// // ----- Importações de bibliotecas -----
#include "esp_camera.h"
#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_http_server.h"
#include <DHT.h>
#include <Blynk.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <app_httpd.cpp>

//
// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//
//            You must select partition scheme from the board menu that has at least 3MB APP space.
//            Face Recognition is DISABLED for ESP32 and ESP32-S2, because it takes up from 15
//            seconds to process single frame. Face Detection is ENABLED if PSRAM is enabled as well

// ===================
// Select camera model
// ===================
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE  // Has PSRAM
//#define CAMERA_MODEL_ESP32S3_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
//#define CAMERA_MODEL_M5STACK_UNITCAM // No PSRAM
//#define CAMERA_MODEL_M5STACK_CAMS3_UNIT  // Has PSRAM
#define CAMERA_MODEL_AI_THINKER  // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM
//#define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM
// ** Espressif Internal Boards **
//#define CAMERA_MODEL_ESP32_CAM_BOARD
//#define CAMERA_MODEL_ESP32S2_CAM_BOARD
//#define CAMERA_MODEL_ESP32S3_CAM_LCD
//#define CAMERA_MODEL_DFRobot_FireBeetle2_ESP32S3 // Has PSRAM
//#define CAMERA_MODEL_DFRobot_Romeo_ESP32S3 // Has PSRAM
#include "camera_pins.h"

// ----- Variaveis Globais
// char ssid[] = "Alunos"; char pass[] = "Alunos@123";
const char* ssid = "Eu";
const char* pass = "Ricardo10";

DHT dht(12, DHT11);  //DHT dht(33, DHT11);



float temp;  // -- DHT temperatura
float umid;  // -- DHT umidade
int valorumidade;
int valorumidade2;  // Umidade Solo

// ----- Definições
#define BLYNK_PRINT Serial
#define bomba 14  //#define bomba 16
#define BOTAO_BOMBA V3
#define ventilador 15  // #define ventilador 17
#define BOTAO_VENTILADOR V4
#define umidSoloAnalogPort 13  //#define umidSoloAnalogPort 32

// INICIO ----- Definições das portas codigo da CAM ------
// #define PWDN_GPIO_NUM 32
// #define RESET_GPIO_NUM -1
// #define XCLK_GPIO_NUM 0
// #define SIOD_GPIO_NUM 26
// #define SIOC_GPIO_NUM 27
// #define Y9_GPIO_NUM 35
// #define Y8_GPIO_NUM 34
// #define Y7_GPIO_NUM 39
// #define Y6_GPIO_NUM 36
// #define Y5_GPIO_NUM 21
// #define Y4_GPIO_NUM 19
// #define Y3_GPIO_NUM 18
// #define Y2_GPIO_NUM 5
// #define VSYNC_GPIO_NUM 25
// #define HREF_GPIO_NUM 23
// #define PCLK_GPIO_NUM 22
// FIM ----- Definições das portas codigo da CAM ------


#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t stream_httpd = NULL;

static esp_err_t stream_handler(httpd_req_t* req) {
  camera_fb_t* fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t* _jpg_buf = NULL;
  char* part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK) {
    return res;
  }

  while (true) {
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
    } else {
      if (fb->width > 400) {
        if (fb->format != PIXFORMAT_JPEG) {
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if (!jpeg_converted) {
            Serial.println("JPEG compression failed");
            res = ESP_FAIL;
          }
        } else {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }
      }
    }
    if (res == ESP_OK) {
      size_t hlen = snprintf((char*)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char*)part_buf, hlen);
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, (const char*)_jpg_buf, _jpg_buf_len);
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if (fb) {
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if (_jpg_buf) {
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if (res != ESP_OK) {
      break;
    }
  }
  return res;
}

void startCameraServer();
void setupLedFlash(int pin);

// void startCameraServer(){
//   httpd_config_t config = HTTPD_DEFAULT_CONFIG();
//   config.server_port = 80;

//   httpd_uri_t index_uri = {
//     .uri       = "/",
//     .method    = HTTP_GET,
//     .handler   = stream_handler,
//     .user_ctx  = NULL
//   };

//   if (httpd_start(&stream_httpd, &config) == ESP_OK) {
//     httpd_register_uri_handler(stream_httpd, &index_uri);
//   }
// }

void setup() {
  Serial.begin(115200);  // a declaração do serial tem que vir sempre em primeiro
  // put your setup code here, to run once:
  pinMode(bomba, OUTPUT);
  pinMode(ventilador, OUTPUT);
  pinMode(umidSoloAnalogPort, INPUT);
  dht.begin();

  Serial.setDebugOutput(true);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG;  // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if (config.pixel_format == PIXFORMAT_JPEG) {
    if (psramFound()) {
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    // Best option for face detection/recognition
    config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // Inicia a câmera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }


  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);        // flip it back
    s->set_brightness(s, 1);   // up the brightness just a bit
    s->set_saturation(s, -2);  // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  if (config.pixel_format == PIXFORMAT_JPEG) {
    s->set_framesize(s, FRAMESIZE_QVGA);
  }

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)
  s->set_vflip(s, 1);
#endif

// Setup LED FLash if LED pin is defined in camera_pins.h
#if defined(LED_GPIO_NUM)
  setupLedFlash(LED_GPIO_NUM);
#endif

  // Conecta o WiFi
  WiFi.begin(ssid, pass);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    Serial.print(WiFi.status());
  }

  Serial.println("");
  Serial.println("WiFi connected");

  Serial.print("Camera Stream Ready! Go to: http://");
  Serial.print(WiFi.localIP());

  // Start streaming web server
  startCameraServer();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  //desativa o detector de brownout
}

void loop() {
  Blynk.run();
  modDHT11();
  modLM393();  // Sensor Umidade Solo
  delay(500);
}

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