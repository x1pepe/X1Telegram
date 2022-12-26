// X1Telegram, por X1pepe.
// --------------------------
// Optimizado para ESP8266 NodeMCU 12E + Mitel, aunque puede funcionar en cualquier ESP8266 y ESP32.
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
// Durante el desarroyo tuve que hacer muchas pruebas, creé un grupo inicial con varios BOTs y algunos humanos como:
// Xavi Rompe (Rookie Drive), Mortimer (, Andrés Ortiz (BaDCat), Antxiko (Las Baldas) y FranSX. Desde aquí mi agradecimiento a todos ellos.
// 
// X1Telegram necesita las librerias FASBOT, puedes instalarlas directamente desde el IDE de Arduino  o desde el GitHub de Alex Gyver.
// Este código no se creado desde cero, se han usado las librerias de Alex Gyver FASTBOT, por favor visita su gran trabajo en:
// https://github.com/GyverLibs 



#define WIFI_SSID "xxxxxx" // Pon aqui tu red WI-FI
#define WIFI_PASS "xxxxxxxxxxx" // Pon aqui tu contraseña


// Crea tu propio BOT en Telegram. Con la lupa, busca BotFather y entabla un chat con él, luego escribe "/newbot" y crea un BOT, para usarlo como usuario en X1Telegram.
// Una vez creado el BOT y dentro del chat con BotFather escribe "/SETPRIVACY", selecciona tu BOT y lo dejas en DISABLED.
#define BOT_TOKEN "xxxxxxxxxxxxx:xxxxxxxxxxxxxxxxxxxxxxxxxxxxx" // Ya puedes pegar aqui abajo el TOKEN de tu BOT.

// Puedes usar éste grupo para tus pruebas o añadir otro cualquiera. Yo uso el IDBot para extraer el Token.
// Para que X1Telegram funcione tu BOT debe ser administrador del grupo donde se vaya a añadir.
#define CHAT_ID "-1001601091527" // ID del grupo "X1Telegram". 
 
// Inicia el BOT
#include <FastBot.h>
FastBot bot(BOT_TOKEN);

// para comprobar archivos (administrador de archivos web)
#include <LittleFS.h>

#ifdef ESP8266
#include <ESP8266WebServer.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureBearSSL.h>
ESP8266WebServer server(80);
#else
#include <WebServer.h>
WebServer server(80);
#endif


// Pines del NodeMCU8266
const int ledPin = 16; // Pin de configuración del Relé o LED usado
int ledStatus = 0;    // Iniciamos el estado a 0 – apagado.


void setup() {
  connectWiFi();

// ESP01 selecciono LedPin como salida (pin2)
pinMode(ledPin, OUTPUT); // Inicializa ledPin digital como salida.
delay(10);
digitalWrite(ledPin, HIGH); // Inicializa el Pin en OFF. 
  
// Inicia el BOT
bot.setChatID(CHAT_ID);
    
  //Inicio visible al chat de Telegram.
  // Añade un Sticker. Puedes usar cualquiera que defina tu personalidad
  String sticker = F("CAACAgQAAxkBAAEGY0Njbrfwy1f4hIUEPXm1MfTlSumm_wACPgwAAjVKMVIeTfBrM61zfCsE"); // ID de Sticker "X1Telegram". He usado el BOT "Get Sticker ID"
  bot.sendSticker(sticker); // Envia el Sticker

  // Envia un mensaje diciendo quien esta iniciando el BOT. Esto puede venir bien si hay varios usuarios con BOTS de puerto serie distintos
  bot.sendMessage("X1Telegram iniciado por X1Telegram_BOT!"); //Pon aqui tu nick para que sepamos quien mas hay usando X1Telegram.

  // conecta la funcion del controlador
  bot.attach(newMsg);

   setWebface();
   }

// Controlador de mensajes
void newMsg(FB_msg& msg) {

// Actualización OTA (Over The Air). Es posible configuraciones online con posterior flasheado del ESP8266. 
// Una vez introducido el .BIN, .BIN + palabra clave, .BIN + palabra clave + ID administrador o como se quiera configurar, el proceso empezará y todos los uduarios
// conectados en ese momento a X1Telegram se actualizarán automaticamente con el .BIN enviado.
// Cuidado con esto ya que no todas los nombres de red wifi, contraseñas y TOKENs tienen por qué coincidir.

// if (msg.OTA) bot.update(); // Actualizar si se envía un archivo .BIN al grupo.
// if (msg.OTA && msg.chatID == "xxxxxxxxxxx") bot.update(); // Actualizar si el archivo BIN lo envía una persona famosa (administrador).
   if (msg.OTA && msg.text == "update") bot.update(); // Actualizar si el archivo es un .BIN y ademas se envía con el texto "update".
  

// Esto imprime solo en el chat de Telegram, no en el monitor puerto serie.
// Una vez introducidas las palabras claves creará un acceso directo en la conversación para ir a dicho enlace.
// Keywords text Sotano.

if (msg.text == "Sotano") bot.sendMessage("Combines Sotano with: web, facebook, youtube, twitter");
if (msg.text == "Sotano web") bot.sendMessage("http://www.sotanomsxbbs.org/");
if (msg.text == "Sotano facebook") bot.sendMessage("https://www.facebook.com/sotano.msxbbs");
if (msg.text == "Sotano youtube") bot.sendMessage("https://www.youtube.com/@x1pepeibz");
if (msg.text == "Boixos twitter") bot.sendMessage("https://twitter.com/X1pepe1");

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


// Keywords LED
// Esto activa y desactiva la salida digital correspondiente del NodeMCU12E, obviamente se pueden usar otras. 
// En este caso la el PIN 16 va asociado al led que ya viene incluido en la placa. 
// La idea no fué sólo la de encender y apagar un led si no conectar algun relé para activar/desactivar o domotizar otro tipo de cosas:
// la puerta de un garaje, luces de casa, etc ...

 digitalWrite(ledPin, HIGH);
 if (msg.text == "/ledon") {
 digitalWrite(ledPin, LOW);   // Activa, enciende el LED. Pone el PIN16 en HIGH.
 bot.sendMessage( "Led en ON"); // Esto se imprime solo en Telegram.
 Serial.println("Led en ON"); // Esto se imprime solo en la consola del puerto serie. 
 }
   
 if (msg.text == "/ledoff"){ // Desactiva, apaga el led. Pone el PIN16 en LOW.
 digitalWrite(ledPin, HIGH);    // Apaga el LED. Pone el PIN16 en LOW.
 bot.sendMessage( "Led en OFF"); //Esto se imprime solo en Telegram.
 Serial.println("Led en OFF"); //Esto se imprime solo en la consola del puerto serie.
 }  
    
 if (msg.text == "/led"){ // Informa de los comandos disponibles LED.
 bot.sendMessage( "/ledon  : Enciende el led");
 bot.sendMessage( "/ledoff : Apaga el led");
 }
 
 
   // Doy formato a como se veran los mensajes
   Serial.println(); //esto deja un espacio entre el usuario y la hora
   Serial.print(msg.username); // imprime nombre de usuario y texto del mensaje
   Serial.print(": "); // imprime ":"
   Serial.println(msg.text); // imprime el mensaje saltando una linea
   
   
  // Tiempo en los mensajes
    FB_Time t(msg.unix, 3); 
  Serial.print(t.timeString()); //imprime la hora. Esto está por mejorar ya que actualmente el sistema horario es el ruso.
  Serial.print(' '); //deja un espacio
  Serial.println("");
  Serial.println();
  //Serial.println(t.dateString()); // Imprime la fecha mes, dia y año
  //Serial.println(msg.toString()); // Esto imprime en puerto serie TODA la informacion sobre el usuario y mensaje, incluso la ID del usuario.

  if (msg.isFile) {                     // Si el mensaje es un archivo ...
    Serial.print("Downloading ");       // imprime Downloading
    Serial.println(msg.fileName);       // nombre.ext del archivo

    String path = '/' + msg.fileName;   // del tipo /nombrearchivo.xxx
    File f = LittleFS.open(path, "w");  // preparado para grabar
    bool status = bot.downloadFile(f, msg.fileUrl);
    Serial.println(status ? "OK" : "Error");
  }
 }

void loop() {
  bot.tick();   // marcar en bucle
  server.handleClient();
  
// Esto hace que se pueda escribir desde el serial de Arduino (o desde cualquier otro) hacia Telegram.
// Gracias a Xavi Rompe ahora una vez escrita la cadena de caracteres la imprime en la consola del puerto serie y la envia, 
// antes solo la enviaba sin imprimir.
// Está en mi lista TO DO, hacer que se imprima caracter a caracter y no la cadena completa.

if (Serial.available()) {
String teststr = Serial.readString();
Serial.print("xxxxxxxx BOT: "); // Aquí defino manualmente el nombre de mi BOT. Nombra manualmente el tuyo :)
Serial.println(teststr);
bot.sendMessage(teststr);
Serial.println();
 }

  // Esto define el tiempo entre mensajes, en este caso los mensajes se imprimen con un espacio de 1 seg. entre ellos
  static uint32_t tmr;
  if (millis() - tmr >= 1000) { 
    tmr = millis();
    // fecha y hora de salida
    FB_Time t = bot.getTime(3); 
    } 
}

// servidor web para verificar archivos (administrador de archivos)
void setWebface() {
  // ejecuta FS y comprueba que no haya errores
  if (!LittleFS.begin()) {
    Serial.println("FS Error");
    return;
  }

  // Servidor Web
  server.begin();
  server.on("/", []() {
    String str;
    str += F("<!DOCTYPE html>\n");
    str += F("<html><body>\n");

#ifdef ESP8266
    Dir dir = LittleFS.openDir("/");
    while (dir.next()) {
      if (dir.isFile()) {
        str += F("<a href=\"/");
        str += dir.fileName();
        str += F("\">");
        str += dir.fileName();
        str += F("</a><br>\n");
      }
    }
    
#else
    File root = LittleFS.open("/");
    File file = root.openNextFile();
    while (file) {
      if (!file.isDirectory()) {
        str += F("<a href=\"/");
        str += file.name();
        str += F("\">");
        str += file.name();
        str += F("</a><br>\n");
      }
      file = root.openNextFile();
    }
    
#endif

    str += F("</body></html>");
    server.send(200, "text/html", str);
  });

  // selección de archivo (url - nombre de archivo) - muestra el archivo en el navegador
  server.onNotFound([]() {
    File file = LittleFS.open(server.uri(), "r");
    if (!file) {
      server.send(200, "text/plain", "Error");
      return;
    }
    server.streamFile(file, "text/plain");
    file.close();
  });
}

void connectWiFi() {
  delay(2000); // Temporizo 2 seg.
Serial.begin(4800,SERIAL_7E1); // Establece el puerto serie a 4800 bps. para minitel
Serial.println();
  
  // Conecta a tu red WI-FI con tu contraseña 
  //WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {

    Serial.print("."); // Mientras se conecta a la WI-FI imprime "."
    delay(500);
   if (millis() > 15000) ESP.restart();// Esto es el tiempo que tiene el ESP8266 para conectar a la red WI-FI
    }

// Esto será mostrado si se resetea con el boton reset de la NodeMCU12E.
  
Serial.println("Connected to WI-FI"); // Si no hay errores en la conexion WI-FI imprime "Connected to WI-FI"
Serial.println();
Serial.println("           __ __  ___  _____      _ ");                           
Serial.println("          |  |  ||_  ||_   _|___ | | ___  ___  ___  ___  _____"); 
Serial.println("          |-   -| _| |_ | | | -_|| || -_|| . ||  _|| .'||     |");
Serial.println("          |__|__||_____||_| |___||_||___||_  ||_|  |__,||_|_|_|");
Serial.println("           Ready To Chat ...             |___|");
Serial.println("                                             .,;cWWc'");
Serial.println("                                         ,;:cWWWWWWo,");
Serial.println("                                    ,;:cWWWWWWWWWWol'");
Serial.println("                              ',:ccWWWWWWWWWWWWWWWo:.");
Serial.println("                         ',;cWWWWWWWWWc:,;cWWWWWWWW;.");
Serial.println("                    ',;:cWWWWWWWWWWc;'  ':WWWWWWWWc'");
Serial.println("               ',;:cWWWWWWWWWWWc;,'   ':WWWWWWWWWW:.");
Serial.println("           ',;:cWWWWWWWWWWWWc:,     ':WWWSOTANOWWW;");
Serial.println("        .;cWWWWWWWWWWWWWWc;'     ';:WWWWWWMSXWWWWc'");
Serial.println("         .',;:ccWWWWWWc:,      ,:WWWWWWWWWBBSWWWW:");
Serial.println("               .',;;,'       ,cWWWWWWWWWWWWWWWWWW;");
Serial.println("                          .'cWWWWWWWWWWWWWWWWWWWc,");
Serial.println("                           .:WWWWWWWWWWWWWWWWWWW:");
Serial.println("                             .,:WWWWWWWWWWWWWWWW;");
Serial.println("To get info                    .';:cWWWWWWWWWWWc'");
Serial.println("type: Help, Sotano, Ascii, /led    .';:cWWWWWWc:.");
Serial.println("in Telegram chat...                    .,:WWWWc,.");
Serial.println();
}
