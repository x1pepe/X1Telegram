
// X1Telegram, por X1pepe.
// -----------------------
// x1pepe@yahoo.es
// sotanomsxbbs.org:23
// -----------------------
// Esp32CAM
// 
// Este codigo envia y recibe mensajes de Telegram hacia el puerto serie y del puerto serie hacia Telegram. Al ser realmente un cliente de Telegram
// para Arduino, no seria complicado adaptarlo a cualquier otro sistema como Spectrum, Commodore, Amstrad, Amiga, etc.
// Se ha probado con exito en Msx, Msx2, MsxVR, Minitel, PCs.
// Para funcionar en Msx tiene que ser adaptado o conectado a un cartucho Rs232 de Msx. Para funcionar en PC unicamente hay que conectarlo 
// a un puerto USB con un simple cable de telefono mobil.
// Actualmente puedes:
// - Descargar archivos automaticamente subidos al grupo, luego hay que accedes a la IP del ESP8266 para recuperar los archivos descargados.
//   el tamaño/cantidad de los archivos descargados dependerá de como se cree las particiones una ve que se flashee la memoria de la ESP8266.
// - Hacer actualizaciones on-line.
// - Provocar "acciones" en el chat de Telegram, consola puerto serie o hardware, introduciendo palabras clave dentro del chat del grupo.
//
// Al iniciar X1Telegram tambien lo hace la camara del Esp32, para verlo escribe en un navegador la IP local de tu Esp32.
// 
// Durante el desarroyo tuve que hacer muchas pruebas, creé un grupo inicial con varios BOTs y algunos humanos como:
// Xavi Rompe (Rookie Drive), Mortimer, Andrés Ortiz (BaDCat), Antxiko (Las Baldas) y FranSX. Desde aquí mi agradecimiento a todos ellos.
// 
// X1Telegram necesita las librerias FASBOT, puedes instalarlas directamente desde el IDE de Arduino  o desde el GitHub de Alex Gyver.
// Este código no se creado desde cero, se han usado las librerias de Alex Gyver FASTBOT, por favor visita su gran trabajo en:
// https://github.com/GyverLibs 

#define WIFI_SSID "xxxxxx" // Pon aqui tu red WI-FI
#define WIFI_PASS "xxxxxxxxxxxxx" // Pon aqui tu contraseña
#define BOT_TOKEN "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" // xxxxxxxxx_BOT. Crea tu propio BOT con BotFather y pon aqui el TOKEN.
#define CHAT_ID "-1001601091527" // ID del grupo "Arduino Serial Msx"

#include <FastBot.h>
FastBot bot(BOT_TOKEN);



#include "esp_camera.h"
#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "soc/soc.h" //disable brownout problems
#include "soc/rtc_cntl_reg.h"  //disable brownout problems
#include "esp_http_server.h"

//Credenciales red WIFI para la CAM
const char* ssid = "BAT_CUEVA";
const char* password = "sargamassa";

#define PART_BOUNDARY "123456789000000000000987654321"

// This project was tested with the AI Thinker Model, M5STACK PSRAM Model and M5STACK WITHOUT PSRAM
#define CAMERA_MODEL_AI_THINKER // Descomenta el modelo de tu camara.
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WITHOUT_PSRAM

// Not tested with this model
//#define CAMERA_MODEL_WROVER_KIT

#if defined(CAMERA_MODEL_WROVER_KIT)
  #define PWDN_GPIO_NUM    -1
  #define RESET_GPIO_NUM   -1
  #define XCLK_GPIO_NUM    21
  #define SIOD_GPIO_NUM    26
  #define SIOC_GPIO_NUM    27
  
  #define Y9_GPIO_NUM      35
  #define Y8_GPIO_NUM      34
  #define Y7_GPIO_NUM      39
  #define Y6_GPIO_NUM      36
  #define Y5_GPIO_NUM      19
  #define Y4_GPIO_NUM      18
  #define Y3_GPIO_NUM       5
  #define Y2_GPIO_NUM       4
  #define VSYNC_GPIO_NUM   25
  #define HREF_GPIO_NUM    23
  #define PCLK_GPIO_NUM    22

#elif defined(CAMERA_MODEL_M5STACK_PSRAM)
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    15
  #define XCLK_GPIO_NUM     27
  #define SIOD_GPIO_NUM     25
  #define SIOC_GPIO_NUM     23
  
  #define Y9_GPIO_NUM       19
  #define Y8_GPIO_NUM       36
  #define Y7_GPIO_NUM       18
  #define Y6_GPIO_NUM       39
  #define Y5_GPIO_NUM        5
  #define Y4_GPIO_NUM       34
  #define Y3_GPIO_NUM       35
  #define Y2_GPIO_NUM       32
  #define VSYNC_GPIO_NUM    22
  #define HREF_GPIO_NUM     26
  #define PCLK_GPIO_NUM     21

#elif defined(CAMERA_MODEL_M5STACK_WITHOUT_PSRAM)
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    15
  #define XCLK_GPIO_NUM     27
  #define SIOD_GPIO_NUM     25
  #define SIOC_GPIO_NUM     23
  
  #define Y9_GPIO_NUM       19
  #define Y8_GPIO_NUM       36
  #define Y7_GPIO_NUM       18
  #define Y6_GPIO_NUM       39
  #define Y5_GPIO_NUM        5
  #define Y4_GPIO_NUM       34
  #define Y3_GPIO_NUM       35
  #define Y2_GPIO_NUM       17
  #define VSYNC_GPIO_NUM    22
  #define HREF_GPIO_NUM     26
  #define PCLK_GPIO_NUM     21

#elif defined(CAMERA_MODEL_AI_THINKER)
  #define PWDN_GPIO_NUM     32
  #define RESET_GPIO_NUM    -1
  #define XCLK_GPIO_NUM      0
  #define SIOD_GPIO_NUM     26
  #define SIOC_GPIO_NUM     27
  
  #define Y9_GPIO_NUM       35
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       39
  #define Y6_GPIO_NUM       36
  #define Y5_GPIO_NUM       21
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       18
  #define Y2_GPIO_NUM        5
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     22
#else
  #error "Camera model not selected"
#endif

static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t stream_httpd = NULL;

static esp_err_t stream_handler(httpd_req_t *req){
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if(res != ESP_OK){
    return res;
  }

  while(true){
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
    } else {
      if(fb->width > 400){
        if(fb->format != PIXFORMAT_JPEG){
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if(!jpeg_converted){
            Serial.println("JPEG compression failed");
            res = ESP_FAIL;
          }
        } else {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }
      }
    }
    if(res == ESP_OK){
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if(fb){
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if(_jpg_buf){
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if(res != ESP_OK){
      break;
    }
    //Serial.printf("MJPG: %uB\n",(uint32_t)(_jpg_buf_len));
  }
  return res;
}

void startCameraServer(){
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80; //  Puerto 80


  httpd_uri_t index_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL
  };
  
  //Serial.printf("Starting web server on port: '%d'\n", config.server_port);
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &index_uri);
  }
}


void setup() {
   WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
   
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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 10000000; //  config.xclk_freq_hz = 20000000;

  config.pixel_format = PIXFORMAT_JPEG; 
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

   
    
  connectWiFi();
  
  // Imprime la IP local de tu ESP32
  //Serial.println(WiFi.localIP()); 
    
  // Inicia el BOT
  bot.setChatID(CHAT_ID);
  
  // Añade un Sticker. Puedes usar cualquiera que defina tu personalidad
  String sticker = F("CAACAgIAAxkBAAEG-QFjpezNSCJzqY7lZcXiRf-2MDEHiQAC1wAD9wLIDx-GnSrVbtclLAQ"); // ID de Sticker "Aspiradora animada". He usado el BOT "Get Sticker ID"
  bot.sendSticker(sticker); // Envia el Sticker

  // Envia un mensaje diciendo quien esta iniciando el BOT. Esto puede venir bien si hay varios usuarios con BOTS de puerto serie distintos
  bot.sendMessage("X1Telegram Esp32CAM by xxxxxxx_BOT!"); //Pon aqui tu nick para que sepamos quien mas hay usando X1Telegram.
  
  // conecta la funcion del controlador para que lleguen los mensajes
  bot.attach(newMsg);
}

// Controlador de mensajes
void newMsg(FB_msg& msg) {

// Actualización OTA (Over The Air). Es posible configuraciones online con posterior flasheado del ESP8266. 
// Una vez introducido el .BIN, .BIN + palabra clave, .BIN + palabra clave + ID administrador o como se quiera configurar, el proceso empezará y todos los uduarios
// conectados en ese momento a X1Telegram se actualizarán automaticamente con el .BIN enviado.
// Cuidado con esto ya que no todas los nombres de red wifi, contraseñas y TOKENs tienen por qué coincidir.

// if (msg.OTA) bot.update(); // Actualizar si se envía un archivo .BIN al grupo.
// if (msg.OTA && msg.chatID == "xxxxxxxxxx") bot.update(); // Actualizar si el archivo BIN lo envía una persona famosa (administrador).
   if (msg.OTA && msg.text == "update") bot.update(); // Actualizar si el archivo es un .BIN y ademas se envía con el texto "update".
  

// Esto imprime solo en el chat de Telegram, no en el monitor puerto serie.
// Una vez introducidas las palabras claves creará un acceso directo en la conversación para ir a dicho enlace.
// Keywords text Sotano.

if (msg.text == "Sotano") bot.sendMessage("Combines Sotano with: web, facebook, youtube, twitter");
if (msg.text == "Sotano web") bot.sendMessage("http://www.sotanomsxbbs.org/");
if (msg.text == "Sotano facebook") bot.sendMessage("https://www.facebook.com/sotano.msxbbs");
if (msg.text == "Sotano youtube") bot.sendMessage("https://www.youtube.com/@x1pepeibz");
if (msg.text == "Sotano twitter") bot.sendMessage("https://twitter.com/X1pepe1");

//Keywords HELP
if (msg.text == "Help"){
  Serial.println("Welcome to X1Telegram ! ");
  Serial.println("This is a interface to comunicate your Msx (and old computers)");
  Serial.println("throught serial port to Telegram. ");
  Serial.println();
  delay (2000);
  Serial.println("There's some KEYWORDS to get an action, so over Telegram chat type: Ascii and:");
  Serial.println("x1t, msx, patriot, elvis, geist, sid, super, alien, msxvr, space, computer, smith");
  Serial.println("linux, neo, starwars");
  Serial.println("For example: Ascii space [Send this message] and the Ascii space will be sent");
  Serial.println("to the serial port console"); 
  Serial.println("Send Ascii message to get the valid words to combine it. ");
  Serial.println();
  delay (2000);
  Serial.println("Every sent to Telegram chat sent will be downloaded to your interface IP.");
  Serial.print("Locate your downloaded files at this IP: ");
  Serial.println (WiFi.localIP());
  Serial.println("If you're using BaDCat WI-FI Msx cartridge add this IP to your REPO");
  Serial.println("to download the file to your Msx.");
  Serial.println("If you're using GR8NET Ethernet Msx cartridge add this IP to your server.");
  Serial.println("Also you can use a modern computer to do this task ;)");
  Serial.println();
  delay (2000);
  Serial.println("If you are using a Msx interface, you can ON and OFF the led built in."); 
  Serial.println("So over Telegram chat:");
  Serial.println("/ledon to get the led on, by example: /ledon [send this message]");
  Serial.println("/ledoff to get the led off, by example: /ledoff [send this message]");
  Serial.println();
  bot.sendMessage("Info sent to serial port console.");
  }
  
// Keywords STICKERS
// Esto envía un sticker en Telegram.
// Primero desbes extraer el ID del sticker desde Telegram con un BOT que lo extraiga, yo uso GET STICKER ID.
// Desde la lupa de Telegram lo buscas e inicias una conversacion con él, cada sticker que le envies, él te contestará y te 
// devolverá el ID de ése sticker :)

String sticker = F("CAACAgIAAxkBAAEFqd9jBl2uZA-mKqrEGMlB-o4pKEaLmAACNQYAAkb7rAS9TFeyNz7W0ykE");
if (msg.text == "ok") bot.sendSticker(sticker); // En este caso cuando alguien del grupo envie "ok" en el chat, el BOT de usuario enviará el sticker.

// Print only on serial console
// Keywords TEXT
if (msg.text == "msx"){ 
Serial.println("Somos la comunidad"); // esto se imprime solo en la consola del puerto serie.
 }

 // Keywords ASCII
// La idea de ésto fué la posibilidad de enviar "emojis" hacia un Msx por puerto serie dentro de un chat.
if (msg.text == "Ascii"){ 
bot.sendMessage("Combines Ascii with: x1t, msx, patriot, elvis, geist, sid, alien, msxvr, space, computer, smith, linux, neo, starwars");
bot.sendMessage("The ASCII will be printed on the Serial Console"); 
}

if (msg.text == "Ascii x1t"){ 
Serial.println("           __ __  ___  _____      _ ");                           
Serial.println("          |  |  ||_  ||_   _|___ | | ___  ___  ___  ___  _____"); 
Serial.println("          |-   -| _| |_ | | | -_|| || -_|| . ||  _|| .'||     |");
Serial.println("          |__|__||_____||_| |___||_||___||_  ||_|  |__,||_|_|_|");
Serial.println("                                         |___|");
}

if (msg.text == "Ascii msx"){
Serial.println("           '''''.     .''''.        ..'''''''''''''''''''.        .''''''.");      
Serial.println("          cNWNWNd.   ,0WNNNx.    'lk0XNNNNNNNNNNNNNNNNNNNXo.    .cKNNNNXd.");      
Serial.println("         .OMMMMMN:  .xWMMMMN:  .lXMMMMMMMMMMMMMMMMMMMMMMMMWO,  .xNMMMMXl.");       
Serial.println("         lWMMMMMMk. :XMMMMMMO. lNMMMMX0OOOOOOOOOOOOOOO0NMMMMXol0MMMMWO,");         
Serial.println("        .OMMMMMMMNc.kMMMMMMMWl.xMMMMNo........        .cKMMMMWMMMMMNo.");          
Serial.println("        lWMMMMMMMM0kNMMMMMMMM0,lWMMMMXOOOOOOOOkdc.      'kWMMMMMMW0;");            
Serial.println("       '0MMMMMMMMMMMMMMMMMMMMWo.dNMMMMMMMMMMMMMMMXd.     '0MMMMMMNc");             
Serial.println("       lWMMMWKXMMMMMMMMWKXMMMMK; ,d0XXXNNNNNNWMMMMWk.   'kWMMMMMMMK:");            
Serial.println("      '0MMMMK;oWMMMMMMMK;lWMMMWx.  ..........cKMMMMX:  :KMMMMWWMMMMNd.");          
Serial.println("      lWMMMWd '0MMMMMMWo .OMMMMNkooooooooooookNMMMMX;.xNMMMMXocOWMMMWO;");         
Serial.println("     '0MMMMK,  lNMMMMM0'  cNMMMMMMMMMMMMMMMMMMMMMMNo:OWMMMW0;  .dNMMMMXo.");       
Serial.println("     oWMMMWd.  .kMMMMWo   .OMMMMMMMMMMMMMMMMMMMWXkldXMMMMNx.     :KMMMMWk'");      
Serial.println("     ;lllll.    'llllc.    'llllllllllllllllllc:. 'lllooo;.       .cooooo:.");
}

if (msg.text == "Ascii patriot"){
Serial.println("                                                      ,lxOkl;,'....'';:oxO0");
Serial.println("                                                   .:kXKx;. ..'..       .':");
Serial.println("                                                  ;k0kl'    .'.....     ..."); 
Serial.println("                                                .okc..        ...'...   ..."); 
Serial.println("                                               .ll.      ..,;:c;'..");         
Serial.println("                                               :o.    'cxOKK0Oxc'.... ..");    
Serial.println("                                               lc .;okXWWWWWNNKl.          "); 
Serial.println("                                               .;:kNMMMMMWWWNNO:.           ");
Serial.println("                                                .xWWMMMMWWNNN0c.            ");
Serial.println("                                                ,0WWWWWWNNNX0d,.            ");
Serial.println("                                                  cNWWMMWWWNNXOxl:'.        ");
Serial.println("                                                  lNNNXK0Okxxxxdddxxd;..    ");
Serial.println("......                                           .dOo:,........:dOKK0d,.   ,ox");
Serial.println(";;;;;;;;;;;;;;;;;;;;;,,,,,,,,                    .oc. ..';,....cxOOkxdc'..'cdo");
Serial.println("...X1Telegram, ready to chat! .......::;,'...   .'..',cdc,,:dkkxdolllllloooc");
Serial.println(",,,''''''...................                      ,c;,,,,;cdkxoollllllloolc;");
Serial.println("                                                  l0kxkxxddxdlc:::cllolc:;'.");
Serial.println("                                                 ,0XXNNNNXK0kl:,,:clooll:'..");
Serial.println("                                                .kNXNNOOXNXOdlc;;::::ccc;'. ");
Serial.println("                                                cxlc:;.:0XOdl:cclc:,''...   ");
Serial.println("                                                ..   .ckXNX0xoool:'...      ");
Serial.println("                                                     cOkdlcx0kdc,'..        ");
Serial.println("                                                    .,;;cok0x:'...          ");
Serial.println("                                                      .cdkkd:'.             ");
Serial.println("                                                        ,oxxo;.             ");
Serial.println("                                                         .xN0o'  ");
}

if (msg.text == "Ascii elvis"){
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMWNWWWWWWWWWWMMMMMMMMMMMMMMMMMMMMMMMMNK0OOOKWMMM");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMXd:,;:;,,,;;;;:oONNKOkkOXMMMMMMMMMMMMMMWWNNWWMMMM");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMN0XXk,               .''.  .lKMMMMMMMMMMMMMMMMMMMMMMMM");
Serial.println("MMMMMMMMMMN0OKXXXXXKOockOc                       ;oollldkKNMMMMMMMMMMMMMMMMM");
Serial.println("MMMMMMMMNk:. ....... .lk:                   .'.          .,oXMMMMMMMMMMMMMMM");
Serial.println("MMMMMMMKc    .,;.    ,;.                   .'.    ..        'dXMMMMMMMMMMMMM");
Serial.println("MMMMW0kxc'.:c:;.                         .:;   ..:o'          'oXMMMMMMMMMMM");
Serial.println("MMMWx..oKdoOc                       ..:ox0x'..;;cl.             :XMMMMMMMMMM");
Serial.println("MMWk.  ...                      .clx0XWMMW0xxxlcdc.              lNMMMMMMMMM");
Serial.println("Wk,.                     ...;ollOWMMMMMMMMWWN0kOxl'              .dWMMMMMMMM");
Serial.println("X:                     .oKxdNMMMMMMMMMMMMMMMMMMMNOc.              .xWMMMMMMM");
Serial.println("0l:.                  .d0k0NMMMMMMMMMMMMMMMMMMMMNk,                .dNMMMMMM");
Serial.println("Ocdl.                 :kkKWMMMMMMMMMMMMMMMMMMMMMMMNo.         .;c;. .OMMMMMM");
Serial.println("XxONd.               .kNWMMMMMMMMMMMMMMMMMMMMMMMMMMNc       .l0XOkxc'oNMMMMM");
Serial.println("MNKNX:   ..          cNMMMMMMMMMMWNXXXXKXWMMMMMMMMMMXx'    'OWWWWX0Kk;cKMMMM");
Serial.println("MMWWWo   c0kc.    c:.kMMMMMMMWKkl:'.....'xNMMMMMMMMMMM0,  .OWMWXNWMWWx.cNMMM");
Serial.println("MMMMWd .;xKNWx.  .kKONMMMWKxc,.      ...;dXMMMMMMMMMMMMO' ,KMMNklo0WMO..dWMM");
Serial.println("MMMMM0;,kNNWMK;  .dMMMMM0:.          .;coONMMMMMMMMMMMMWk..oWMMMNOkXMO. .OMM");
Serial.println("MMMMMW0dkXWMMN:   'lkXWM0;...     ...,l0WMMMMMMMMMMMMMMMNo .OWMMMMMMMk.  :XM");
Serial.println("MMMMMMWXOxONMWl      'oXNXXXKOdl'  ..,dNMMMMMMMMMMMMMMMMM0, ,0MMMMMMWo   .xM");
Serial.println("MMMMMMMMMN0XWMd.       :XMMMMMMWKkkKXNMMMMMMMMMMMMMMMMMMMWx,:0MMMMMMX;    oW");
Serial.println("MMMMMMMMMMMMMMk.       '0MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWNWMMXxxk0K:   .xM");
Serial.println("MMMMMMMMMMMMMMX:       .kMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWkccOWO'  ;XM");
Serial.println("MMMMMMMMMMMMMMMXxc.    .xMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWMMWk. oWM");
Serial.println("MMMMMMMMMMMMMMMMMMXo.  .xMMMMMMMMMMNK0KWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWx.lWM");
Serial.println("MMMMMMMMMMMMMMMMMMMWo  .xMMMMWK00XNNWKk0NMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWkoKM");
Serial.println("MMMMMMMMMMMMMMMMMMMMK,  :kOkd;',:dKWMMWXXWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWKKW");
Serial.println("MMMMMMMMMMMMMMMMMMMMWO.       .OWWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMMMMMMMMMWO'    .cxkxdxkOO0KOkXWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMKc    ,:.  .;coxOkxONMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMNo.    .:xKWMWWMMMMMMMMMMMMMMMMMMMMWNNMMMMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMWk'   .cxkdl:oXMMMMMMMMMMMMMMMMMMXkONMMMMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMK:      .,cxXMMMMMMMMMMMMMMMMNxlkNMMMMMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMMNd.   .xWWMMMMMMMMMMMMMMMWNk;;OWMMMMMMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMWXd' .OMMMMMMMMMMMMMWN0dc' 'OWMMMMMMMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMXo.;dO0kOKXWWNNKxc'.  .c0WMMMMMMMMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMW0l;,.  ..,;;;;.    .dWMMMMMMMMMMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWWXkl:cloxO0X0o'   ;kNMMMMMMMMMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMNxl:cxNMMMMMMMMMMMMMMMMMMM");
}

if (msg.text == "Ascii geist"){
Serial.println("       ,;;;;;,;:;;;;,;;;;;;;;:;,;:;,;:;,;:;,;:;;;;;;:;,;:;;:;,;::'     ");
Serial.println("  .lx.'k000000000000000000000K00000000000000000000000000000000000k, ld."); 
Serial.println("  .xK,,0MMMMMMMMMMMMMMMMMMMMMMMMMMWNXNNXXXWMMMMMMMMMMMMMMMMMMMMMMNc.kO.");
Serial.println("  .xK,,KMMMMMMMMMMMMMMMMMMMMMMMMW0o;::;;;:xKNMMMMMMMMMMMMMMMMMMMMN:.kO.");
Serial.println("  .xK,'0MMMMMMMWOOWXkKWMMMMMMMMMk.  ..  ....oWMMMMMMMMMMNOON0dONMN:.k0'");
Serial.println("  .xK,,KMMMMMMMXl:x: ,xxkXMMMMMMx.          oWMMMMMMW0dxl..co,lXMN:.k0'");
Serial.println("  .xK,'0MMMMMMMNd'.    'dKWMMMMMx.          oWMMMMMMWOl'    .:xNMNc.k0'");
Serial.println("  .xK,,KMMMMMMMXd:c' .l0MMMMMMMMx.          oWMMMMMMMMMO;. ,c:dNMNc.kO'");
Serial.println("  .xK,,KMMMMMMMMMMMd.oWMMMMMMMMMx.         .dWMMMMMMMMMMO'.xMMMMMN:.kO'");
Serial.println("  .xK,'0MMMMMMMMMMMd.cNMMMMMMMMWd.          oNMMMMMMMMMWk..xMMMMMN:.kO'");
Serial.println("  .xK,,KMMMMMMMMMMMx..;OMMWWWW0c.   ..  ... .:0WMMMMWWNx' .kMMMMMN:.kO.");
Serial.println("  .xK,'0MMMMMMMMMMMO'  c0o;',,.      .......  .:kXKk:,'...,OMMMMMN:.kO'");
Serial.println("  .xK,,KMMMMMMMMMMMWKc...              .....    ...  ..cKKXWMMMMMN:.xO.");
Serial.println("  .xK,'0MMMMMMMMMMMMMN0x:,. .'.                 .''ck0KNMMMMMMMMMN:.k0'");         
Serial.println("  .xK,,KMMMMMMMMMMMMMMMMWWKdk0:               .:ONWWMMMMMMMMMMMMMN:.x0'");
Serial.println("  .xK,'0MMMMMMMMMMMMMMMMMMMMWO,              .xWMMMMMMMMMMMMMMMMMN:.k0'");
Serial.println("  .xK,,KMMMMMMMMMMMMMMMMMMMMNKd:.            .kMMMMMMMMMMMMMMMMMMN:.k0'");
Serial.println("  .xK,,KMMMMMMMMMMMMMMMMMMMMXl;l,.,c.       .oOXWMMMMMMMMMMMMMMMMN:.k0'");
Serial.println("  .xK;,KMMMMMMMMMMMMMMMMMMMMNo.   ,c.     'ldl;lXMMMMMMMMMMMMMMMMN:.k0'");
Serial.println("  .xK,;KMMMMMMMMMMMMMMMMWMMWWK:        .:odl..lXWMWMMMWMMMWWMMMMMN:.xO'");
}

if (msg.text == "Ascii sid"){
Serial.println("WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWN");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMNKKNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMW");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMK:.;0MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMW");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMX:..c0NMNNWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMW");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWo ...oNKdkXNWWMMMMMWMMMMMMMMMMMMMMMMMMMMMMW");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMWKXMMWKXO'   .ok'.':lkNMXKWWX0XWMMMMMMMMMMMMMMMMMMMW");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMXc:KNo...     ..     'dl'.;:'cXMMMMMMMMMMMMMMMMMMMMW");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMNXWO. ck'                      .xNKOKWMMMMMMMMMMMMMMMMW");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMKx0k' ..                       ,d:. .kWMMMMMMMMMMMMMMMW");
Serial.println("MMMMMMMMMMMMMMMMMMMMMNo...                                 .:lk0OKWMMMMMMMMMW");
Serial.println("MMMMMMMMMMMMMMMMMMMMMX;                                       .,o0NNMWWWWMMMW");
Serial.println("MMMMMMMMMMMMMMMMMMMMW0'                                        :Od,;odxXWMMMW");
Serial.println("MMMMMMMMMMMMMMMMMMMMWXc                                         .    ;0NNMMMW");
Serial.println("MMMMMMMMMMMMMMMXdxXWXx,                                              ;ldKMMMW");
Serial.println("MMMMMMMMMMMMMMMXkc:l;                                                 .kWWNWN");
Serial.println("MMMMMMMMMMNKOdll;.       .;dOOxlc:::cloxkkkxl'                        .:dockN");
Serial.println("MMMMMMMMMMWWKd:;;,.     ;ONWMMMMMMMMMMMMMMMMWNOdol;.                      ;KN");
Serial.println("MMMMMMMMMMMMWWWWWKo.  .dNMMMMMMMMMMMMMMMMMMMMMMMMMNOl,                    .;;");
Serial.println("MMMMMMMMMMMMMWX0kkc  'kWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMNd.                     ");
Serial.println("MMMMMMMMMMWNKk:. .. 'OWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWo                     ");
Serial.println("MMMMMMMMMMWN0c''.  .kWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMK;                     ");
Serial.println("MMMMMMMMMMMMNXXx. .xWWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWo                      ");
Serial.println("MMMMMMMMMMMMMNx'  ,OKXNWMMMMMMMMMMMMMMMMMMMMMMMMMMMNk:.                      ");
Serial.println("MMMMMMMMMMMMXc   .''..':xNMMMMWN0kxdoooodxOKNMMMMMMWo.                       ");
Serial.println("MMMMMMMMMMMWk:' ,OXKkdl..xMMMMXc.   ..':odddoldONWWMXl.                      ");
Serial.println("MMMMMMMMMMMMNkdxKWXkol;..dMMMMW0;  'xOKWMWWWXx'.'lKMXl.                      ");
Serial.println("MMMMMMMMMMMMMMMMMKlccoooxXMMMMMMXkdodxkkkdcl0O,  .o0x.                       ");
Serial.println("MMMMMMMMMMMMMMMMMNNWMMMMMMMMMMMMMMN000Oc.  .,..::'..',.                      ");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWMNOxk00OKWWXl'dk.                      ");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMNXWK,                      ");
Serial.println("MMMMMMMMMMMMMMMMMMMMXKKXKOOKNWWMMMMMMMMMMMMMMMMMMMMWMWo                      ");
Serial.println("MMMMMMMMMMMMMMMMMMMWx'...  .;lKMMMMMMMMMMMMMMMMMMMMN0d,                      ");
Serial.println("MMMMMMMMMMMMMMMMMMMMN0xdool, ,KMMMMMMMMMMMMMMMMMWKd,.                        ");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMWXx0WMMMMMMMMMMMMMMMMXo.                           ");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMX:                             ");
Serial.println("MMMMMMMMMMMMMMMMMMMN0KXXXNWMMMMMMMMMMMMMMMMMMMx.                            .");
Serial.println("MMMMMMMMMMMMMMMMMMK:.....';oONMMMMMMMMMMMMMMMMd                            :O");
Serial.println("MMMMMMMMMMMMMMMMMW0ddddddddoc:xNMMMMMMMMMMMMMN:                            :k");
Serial.println("MMMMMMMMMMMMMMMMMWW0xOOkkxOKX0KWMMMMMMMMMMMMWk.                            ..");
Serial.println("MMMMMMMMMMMMMMMMMWWk'......cKMMMMMMMMMMMMMMWO'                          .:xOk");
Serial.println("MMMMMMMMMMMMMMMMMMMWX0OO00KXWMMMMMMMMMMMWWNd.                         .ckKWMW");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWXx,                       .ll''oXMMMW");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMNKOxd:.                      ,od0WWXXNWMMMW");
Serial.println("MMMMMMMMMMMMMMMMMMKdooc:lkXNNNKOxc,.                         .dXMMMMMMMMMMMMW");
Serial.println("MMMMMMMMMMMMMMMMMMXxc'    .''..                               ;0MMMMMMMMMMMMW");
Serial.println("MMMMMMMMMMMMMMMMMMMMWNOc.                         ':c'         ,0WMMMMMMMMMMW");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMWWx.                      .oXWMk.         .xWWWMMMMMMMN");
}

if (msg.text == "Ascii alien"){
Serial.println("                           ;oo:.                  ;ooc.");                        
Serial.println("                          .kMW0'                  .xMM0'");                        
Serial.println("                           '::codd:            ;ddol::,.");                        
Serial.println("                             'OMMk.           dMMX;");                            
Serial.println("                          .cxxONMMXkxxxxxxxxxxxKMMW0xxl.");                        
Serial.println("                          .kMMMWWWWMMMMMMMMMMMMWWWWWMM0,");                        
Serial.println("                       :kk0NMMKl,,oXMMMMMMMMMMWd,,:0MMN0kkl.");                    
Serial.println("                      .xMMMMMM0,  ;KMMMMMMMMMMNc  .kMMMMMMO'");                    
Serial.println("                   ;OO0NMMMMMMWKO0KWMMMMMMMMMMWX000NMMMMMMN0O0l");                 
Serial.println("                   lWMMNXXXWMMMMMMMMMMMMMMMMMMMMMMMMMMWNXXNWMMx.");                
Serial.println("                   lWMNo..,OMMMMMMMMMMMMMMMMMMMMMMMMMMK:..cKMMx.");                
Serial.println("                   lWMN:  .kMMWXKKKKKKKKKKKKKKKKKKKNMM0'  ,0MMx.");                
Serial.println("                   lWMN:  .kMM0;..................'kWW0'  '0MMx.");                
Serial.println("                   :O0k,  .l00x;....'..    .......,d0Od.  .d00l ");                
Serial.println("                              ,kNNNNNNo    cXNNNNN0:");                            
Serial.println("                              .oOOOOOOc    ;kOOOOOx'");
}

if (msg.text == "Ascii msxvr"){
Serial.println("      ....   ....     .............     ......   ;ooooooooooooooooooooooooooo,");  
Serial.println("     :KNNd. ;KNXo.  oOKXXXXXXXXXXX0l. .o0KXKd.  .xKdookNMMMXxoooooooooood0NMMd");  
Serial.println("    .kMMMX:.kMMMX; 0MWXOkkkOOOOO0NMWklOWMMXl.   .xK;  .kMMWo             .:KMd"); 
Serial.println("    :NMMMMkdNMMMMx KMWOcccc:,.   :0WMMMWNO,     .xMO'  ;XMO.  ;dlcccccc'   dMd");  
Serial.println("   .kMMWMMWWMMWWMX  XWMMMMMMNO;   cNMMMKk;      .xMWx.  oX:  ,KO'         ;0Md");  
Serial.println("   :NMNxOMMMMXx0MMk    lllo0MMK, cKMMWMWW0;     .xMMNl  .;. .kMO.  .;,   .kMMd");  
Serial.println("  .kMMO.:NMMMx.cNMW0xxxxxxkXMM0cdNMNx:xNMMNo.   .xMMMX:     oWMk.  ;KNo.  'OMd");  
Serial.println("  :XWNc .xWWX; .kWWWWWWWWWWNKxd0NWKc. .ckKWNk'  .xMMMM0c;;;oXMMKl;;oNMNx;;;dNd");  
Serial.println("  .'''   .''.   .'''''''''''. .,''.      .'',.   :ddddddddddddddddddddddddddd;");                          
}

if (msg.text == "Ascii space"){
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMMMNNNNNWMMWNXNXXXNWMMMWNNNXXXNMMMMMWNXNXNMMMWNXXXXNXNWMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMWk,....;xXNl......:kNMK:.....cKMMNx;....,dNWd'......oNMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMK;  .;.  cXo   ,:. .xMk.     .OMXc  .;.  .k0'  .cllo0MMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMWd. .dd'':0O.  l0,  lWo  .,. .xM0'  ,Oo''cKo   lNWWMMMMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMMXc  'odd0WN:  ;0:  cXc  :k;  oMk.  cWWNNWK,   .,xWMMMMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMMMNk:''. .lKd   .. .dK; .l0:  cWd  .xKkx0Wd. .:odKMMMMMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMMMMXxoxx.  o0'  ,ooOW0'  .'.  :Xl  ,k;  lK:  :XMMMMMMMMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMMMMXo..,.  lXl  lWMMMx. .dk:  ,0o  .,. ,Ox.  ,lckWMMMMMMMMMMMMMMM");
Serial.println("MMMMMMMMMMMMMMMMMWKxllldXM0olkWMMM0olkNMKoldXNklllokNWOlllllo0WMMMMMMMMMMMMMMM");
Serial.println("WWWMMWWWWMWWWWWMWWWWMMWWWWMMWWWWWWMMMWWWWWWWWWMMMWWWWWWWWWWWWWWWWWWMMMMMMWWWWW");
Serial.println("x;,:OXo,c0Xo,,oKd,,oXWx,,:ONo,,,,cKMWd,,,,,,,;dXWd,,,,;;;oXx,,,,,,:OWN0d:;,,,c");
Serial.println("Xl. .xO, .od.  oO,  lXx.  oN:     dWNc   .c:  .dK,  .oOOOKk.  ;:   d0:. ,c.  ;");
Serial.println("MWk' .oO;  ';. .x0,  oO,  cXc  .. ,KWl   .kl  .kk.  ,kOKWX;  :x, .oO;  :KkccdX");
Serial.println("MMMK:  cOc   .  .k0; .o:  ;Kl  :: .dWo   'kc  ;Kl  .,::kNo  .,'.:ONk...';lKMMM");
Serial.println("MMMMNd. ,ko. ':. '0K: .'  .Oo  ';  ;Xd   ,k:  lK;  cNWWMO. .o,  oK0kOO;  :KMMM");
Serial.println("MMMMMWO; .xd. :x:.;KX:    .xo  'o; .dd.  .:. 'Ok.  ;od0K; .dl  cKl .;:';dXMMMM");
Serial.println("MMMMMMMXkdON0dxKNOxKMXkdddxKKxd0WKxd0KxdddddkXMKxddddxKXxdOXOdkXW0dddkKWMMMMMM");
Serial.println("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM");
}

if (msg.text == "Ascii computer") {
Serial.println("                                WKkxooddxkXW");
Serial.println("                            NKOxoooxO0OxdoooxOK");
Serial.println("                       WNKkdolokOO00000000Okdlld0");
Serial.println("                   WWKkdoodxk000000000000OkkkOOxdd0");
Serial.println("                 N0kxoooxO00000000000Okxdoc;';dO00xc");
Serial.println("               N0dllxO0000000000OOOdoc,''     'dxco:");
Serial.println("              k:dkkkkO000000Okxo:;'          'l;,o:");
Serial.println("              x;x000OOkkkkko:;'              'ooox:");
Serial.println("              x;x0000000Ox:                  'ol;l:");
Serial.println("              x;x00000000x'  C:>             'c'.c:");
Serial.println("              x;x00000000x'                  'oodk:");
Serial.println("              x;x00000000x'                  :kOdlcx");
Serial.println("              x;x00000000x'              ,::clc:,.:kK");
Serial.println("              x;x00000000x'         ,;::lc::,,,,;::::dX");
Serial.println("              k,cxO000000x,     ,;cc::;,,,,,;:clcccc:cxKW");
Serial.println("              0:',:coxO00kdc,:c:cc;;,',;;:lccllc:;:lloodxKW");
Serial.println("              x',;;'';:cooodl::,',;;:c:cllc:;;clllxOxoloooxKW");
Serial.println("              x;dkoc:,,,'''',,,;::llcc:::::cloxkOOOOOOxolodokKW");
Serial.println("              Kolk000Oxdc::cc:clcccc:;:cllddkOOOOOOOOOOOOxolododKW");
Serial.println("             Wo;k00000000000xo:;lo::lodOOOOOOOOOOOOOO0kdoll:clc;;kW");
Serial.println("            Wx:ldxO000000OocdxllooloxOOOOOOOOOO0Okdolc:;:clldOx;oW");
Serial.println("             WNKxdoldkO00k;cO00kolooloxkkOOOOkxolc:::cloxkkkxolo0W");
Serial.println("                 WX0kdoood;:O0000koloolloxdolcc:;clodxOOdoooxOKN");
Serial.println("                      NOko:coxO0000koloool:;:clodk0kdoooxkKW");
Serial.println("                         WN0xdoodxO00kollclldOOkxdooxOXWW");
Serial.println("                            WXKkdoldkkkkkkkxoooxOXN");
Serial.println("                                 X0kdooooooxOKW");
Serial.println("                                     NX0kXW");
}


if (msg.text == "Ascii smith") {                                                                                
Serial.println("                               .;oxOKKXNNXK0x;");                                  
Serial.println("                             'o0WMMMMMMMMMMMMNl.");                                
Serial.println("                           'dXMWKkxxxk0KKKKKK0l.");                                
Serial.println("                          :XMMNd.      .......");                                  
Serial.println("                        .lNMMMO.");                                                
Serial.println("                        ,KMMMMK; ");                                               
Serial.println("                        cWMMMMMO.");                                               
Serial.println("                        lWMMMMWO.");                                               
Serial.println("                        ;KWMMMK;    ........    ..'''.");                          
Serial.println("                        'ld0WMO,,okO0KXXXXXKk;,dKNNNWXc");                         
Serial.println("                         .;OXNO. :0WMMMMMMMW0'.kWMMMWO,");                         
Serial.println("                          'coKK;  .:dk00OkOd.  .oxl:,.");                          
Serial.println("                          ..oNM0c'.   .   ..");                                    
Serial.println("                          ..lXMMMXc       'codc.");                                
Serial.println("                            :XMMMW0c.      ..:;.");                                
Serial.println("                             oWMMMMWk,;lo:''''''.");                               
Serial.println("                            ,KMMMMMMNNWNOooddxx:.");                              
Serial.println("                             .oNNNMMMMMMWKdoooc;ld,");                             
Serial.println("                           .'..:c;lxKWMMWNK0XKo.;XNx;.");                          
Serial.println("                        .,o0Nd.     .;o0XNMXd;' .OMMWXOxoc;'.");                  
Serial.println("                    .':d0NMMMNl        ..,:;..   dMMMMMMMMWNXOxl:'.");             
Serial.println("                 .:dONMMMMMMMMX:          .'.    lWMMMMMMMMMMMMMMNKkl,.");         
Serial.println("             .:oONWMMMMMMMMMMMMK;      'cd0k,    ,KMMMMMMMMMMMMMMMMMMW0d'");      
Serial.println("         .:oOXWMMMMMMMMMMMMMMMMM0,    .kWMMWx.   .dMMMMMMMMMMMMMMMMMMMMMO'");      
Serial.println("      .ckXWMMMMMMMMMMMMMMMMMMMMMMx.   :NMMM0,     cNMMMMMMMMMMMMMMMMMMMMN:");      
}


if (msg.text == "Ascii linux") {                                                                              
Serial.println("                                .,coxkOOOOkxoc;.");                                
Serial.println("                             .;dKWMMMMMMMMMMMMWKx;.");                             
Serial.println("                            ,kNMMNK0KNMMMMMMMMMMMWO;");                            
Serial.println("                           ;KMMXo'...,dNMNOxddxKWMMX:");                           
Serial.println("                          .OMMNc    .ckKX0l.   .cKMMO'");                          
Serial.println("                          ;XMMX;    .kNKKXk.    .kMMN:");                          
Serial.println("                          oWMMWk'  .:kXNWXx;. .;kNMMMd");                          
Serial.println("                         :KMMMNO;  .xNMMMMNx. .:kNMMMK:");                         
Serial.println("                      .:xNMMW0:.    .;k00x;      ;OWMMNk:.");                      
Serial.println("                   .,oKWMMMWk.         ..         .dWMMMWKo,.");                   
Serial.println("                 .:ONNKXWMMO.                      .xWMMXKWNO:.");                 
Serial.println("                 ;kxc,.lNMX:                        ,KMWo.,cdx;");                 
Serial.println("                       ,KMO'                        .kMX;");                       
Serial.println("                       .oNO.                        .xWd.");                       
Serial.println("                        .dK:                        ,Od.");                        
Serial.println("                        .lKXkOOOxdl;.      .;ldxOOOkKKl.");                        
Serial.println("                      'dXWMMMMMMMMMWXo.  .oKWMMMMMMMMMWXx,");                      
Serial.println("                     .OMMMMMMMMMMMMMMWo..lWMMMMMMMMMMMMMMO.");                     
Serial.println("                     .cOO00O00O0O00OOd;..,dOO000000O00OOOc.");                     
Serial.println("                         .. .. ....         ...... . .."); 
}

if (msg.text == "Ascii neo") {
Serial.println("                        .,;:o0NNNNKOoc,..");                                       
Serial.println("                       :KWWMMMMMMMMWMMWX0x'");                                     
Serial.println("                      cXMMNOxxxxoclxKWMMMMO.");                                    
Serial.println("                     .kMWNl         .;kNMMWo");                                    
Serial.println("                     '0MWWd.           oXWM0'");                                   
Serial.println("                     '0MWNo.           :0NMX:");                                   
Serial.println("                     '0MMNxc;'. .,;ccc,lOXMWl");                                   
Serial.println("                     .kMMMMWWXc;OWWMMWWX0XWNc");                                   
Serial.println("                     .kMWMMWMK,.oNMMWN0c'o0l.");                                   
Serial.println("                     ,KMMKolOO'  '::,'.  ,c.");                                    
Serial.println("                     ,KMMK,'OXo;.");                                               
Serial.println("                      :OWW0OWNkl'        ..");                                     
Serial.println("                       '0MMMMKo;;,.      .. ");                                    
Serial.println("                        oNMMMN0d;''  .,.");                                        
Serial.println("                        .oNMKl,.   .:ON0x:");                                      
Serial.println("                         'OWNOdlcld0WWWMMXc");                                     
Serial.println("                         ;XMWMMMMMMMMMMMMMK:");                                    
Serial.println("                         dMMMMMMMMMMMMMMMMMXl.");                                  
Serial.println("                         dMMMMMMMMMMMMMMMMWMWXkoc:,'..");                          
}

//star wars
if (msg.text == "Ascii starwars") {                                                                              
Serial.println("          ....       .;;                                      .;dkd,      .");     
Serial.println("        .lOKKOo'     .oc    .;llcc:.      .''':cccc'''.       'kKO0x'   .,'");     
Serial.println("       .:kkkkkkl.    .oc   .:xxOkxkc.     ,dloxk0kxdld;      .lKKO0Kc. .;,.");     
Serial.println("       .lOOxxk0d.    .oc    'oOXXOd'      .,':x0X0kc,,.    .;okOOOOOxl;;,");       
Serial.println("      ..:k0OOOOl..   .oc    .;oOOo;.        .,lxkkl,.     .dNMMMMMMMMKdl'");       
Serial.println("     'ddxNWNNWWOxx,  .oc  .oxkOkkkkko.    .;xKNNNNNKk:.   ,0MMMMMMMMKdkKc");       
Serial.println("    .oNkdKK00KKkkNx. .lc .dKXKxxkOKXKd.   ;KOkXWMWNkOKc   :XMMMMMMMKdOWNl");       
Serial.println("    .dNkoOK00K0xxNO. .;,.,O0O0kk0NW00O'   cXOkXWMWNkONo.  lNMMMMMMKdOWMWo.");      
Serial.println("    .dWklk0OO0OoxNO. .,;cd0xoxxkKNXO0O'   cXOkXXXXXOONo. .dWMMMMMMXXWMMWd.");      
Serial.println("    .dNxlkO00OkodXk. .''....ckxkOOOdxx.   :0xdOOkkOdx0c. .kMMMMMMMMMMMMWx.");      
Serial.println("    .;ocdXKxd0Nkcl:.  ..   .oXXXXXKxoc.   'olkNWNNNOoo,  ,0MMMMMMMMMMMMMk.");      
Serial.println("       .dWX;'0MO.           cK0oo0Kc.       .xWMMMMO'    :KMMMMMMMMMMMMMO'");      
Serial.println("       .oNK;'OWk.           ;00;;00:        .xWMMMMO'    lNMMMMMMMMMMMMM0,");      
Serial.println("       .lKO,'xXx.           ;0O;;OO;        .xMMMMMO'   .dWMMMMMMMMMMMMMK;");      
Serial.println("       .dWK;,OWO.           ;0O,,OO;        .xWMMMMO'   .kMMMMMMMMMMMMMMX:");      
Serial.println("       .oXO,'kNx.           ;00;,kO;        .c00O0Ko.   ,0MMMMMMMMMMMMMMNc");      
Serial.println("       .cOx'.o0o.           ,kx''xx'         'ddcdx,    .:llllx0OoxKklllc'");      
Serial.println("        ...  ...             ..  ..           .. ..           ... ...");           
Serial.println("         .;cllllllllllllllllllllllllc.   .,lllllllc'    'lllllllllllc:,. ");       
Serial.println("        .xWMMMMWX000000000XWMMMNK000x, ..cKMMW00WMW0:. .oWMMMWKxxxxONWN0c.");      
Serial.println("         ,dKWMMW0d:.......oNMMMO,.... .o0XMMXo.'xNMMXl..oWMMMWOolllkXNXO:.");      
Serial.println(" .,,,,,,,;lkXMMMWNk,      lNMMMO.    .dNMMMMXxddkNMMMNo'oWMMMWK0NMMWKxl:;,,,,.");  
Serial.println(" .lKXXXXXXXXXNNNNNKk;     cXNNNx.   .dXNNNNOl::::oKNNNXxxXNNNXl.,lkKNXXXXXXXKl."); 
Serial.println("  ,dxxdo:,,:oxxxd:...,;;;,'';ldo:;;;ll:,,,'.';;;;:odxxxdol:,,'..';:odxxxxxxxd;."); 
Serial.println("  .dNWWXo..oXWWWNk'.lXWWNd..cKWWNKXWWXd.   .dWWWWNKOOOOXNX0d' ;ONWWWWNXXXXXXKl."); 
Serial.println("   .dNMMNkkNMMMMMW0xXMMWx..oXMMNo'cKMMWkc' .xWMMMNxccclOWMW0: 'dKWMMMXkl;''''.");  
Serial.println("    .oNMMMMMMNOKMMMMMMWk'.dNMMW0oclOWMMWWO;.xWMMMWXXWWWW0xl;....;o0WMMWXk:.");     
Serial.println("     .lXMMMMNd.;0WMMMWO,'kWMMW0dooodOWMMMWKokWMMMXl,lkXWNKKKKKKKKKXWMMMWNx.");     
Serial.println("      .,cccc;.  ':ccc:'.':ccc:.     .;ccccc::cccc:.  ..;ccccccccccccccc:,."); 
}


  
   // Doy formato a como se veran los mensajes
   Serial.println(); //esto deja un espacio entre el usuario y la hora
   Serial.print(msg.username); // imprime nombre de usuario y texto del mensaje
   Serial.print(": "); // imprime ":"
   Serial.println(msg.text); // imprime el mensaje saltando una linea
   
   
  // Tiempo en los mensajes
  FB_Time t(msg.unix, 3); 
  Serial.print(t.timeString()); //imprime la hora
  Serial.print(' '); //deja un espacio
  Serial.println("");
  //Serial.println(t.dateString()); // Imprime la fecha mes, dia y año
  //Serial.println(msg.toString()); // Esto imprime en puerto serie TODA la informacion sobre el usuario y mensaje, incluso la ID del usuario.
 }

void loop() {
bot.tick();   // marcar en bucle
  


// Esto hace que se pueda escribir desde el serial de Arduino (o desde cualquier otro) hacia Telegram.
// Gracias a Xavi Rompe ahora una vez escrita la cadena de caracteres la imprime en la consola del puerto serie y la envia, 
// antes solo la enviaba sin imprimir.
// Está en mi lista TO DO, hacer que se imprima caracter a caracter y no la cadena completa.

if (Serial.available()) {
String teststr = Serial.readString();
Serial.print("xxxxxxx BOT: "); // Aquí defino manualmente el nombre de mi BOT. Nombra manualmente el tuyo :)
Serial.println(teststr);
bot.sendMessage(teststr);
Serial.println();
}

 
  // Esto define el tiempo entre mensajes, en este caso los mensajes se imprimen con un espacio de 1 seg. entre ellos
  static uint32_t tmr;
  if (millis() - tmr >= 1000) { // cambio a 3 seg. para que de tiempo a escribir
    tmr = millis();
    // fecha y hora de salida
    FB_Time t = bot.getTime(3); 
    } 
}

void connectWiFi() {
  delay(1000); // Temporizo 1 seg.
  Serial.begin(9600); // Establece el puerto serie a 9600 bds.
  Serial.println();
  

 

  // Conecta a tu red WI-FI con tu contraseña 
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    
    
    // Mientras se conecta a la WI-FI imprime ".", obviamente se puede poner cualquier otra cosa: Connecting People ... o cualquier cosa XD
    Serial.print(".");
    delay(500);
    if (millis() > 15000) ESP.restart(); // Esto es el tiempo que tiene el ESP8266 para conectar a la red WI-FI
    
  }
  
  Serial.println("Connected to WI-FI"); // Si no hay errores en la conexion WI-FI imprime "Connected to WI-FI"



// Esto será mostrado si se resetea con el boton reset del ESP32
  
Serial.println("Connected to WI-FI"); // Si no hay errores en la conexion WI-FI imprime "Connected to WI-FI"
delay(500);
Serial.println();
Serial.println();
Serial.println("X1TELEGRAM * X1pepe (c)2022           ");
Serial.println("---------------------------           ");
Serial.println();
Serial.println("Mail: x1pepe@yahoo.es                 ");
Serial.println("Telegram: @X1pepe                     ");
Serial.println();
Serial.println();
Serial.print("Streaming at local IP: "); 
Serial.println(WiFi.localIP()); 
Serial.println();
Serial.println("Ready to chat...");
Serial.println();
Serial.println();
Serial.println();
Serial.println();

 // Start streaming web server
  startCameraServer();
  
}



    
