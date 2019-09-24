//////////////////////////WIFI//CREDENCIALES///////////////////////////////////////////////////

#include <ESP8266WiFi.h>
#define STASSID ""
#define STAPSK  ""
//////////////////////////SD//LIBRERIAS///////////////////////////////////////////////////
#include <SD.h>
#include <SPI.h>

File data_file, myFile;
#define PIN_CS D8
/////////////////////////USAMOS//LOS//PINES//SERIALES//DEL//PM2.5/////////////////////////////
// El pin TX del sensor debe conectarse al RX del NODE MCU
#include "PMS.h"
PMS pms(Serial);
PMS::DATA data;
////////////////////////DEFINIMOS//EL//PIN//DEL//SENSOR//DHT22/////////////////////////////////
#include "DHT.h" //cargamos la librería DHT
#define DHTPIN D1 //Seleccionamos el pin en el que se conectará el sensor
#define DHTTYPE DHT22 //Se selecciona el DHT22(hay otros DHT)
DHT dht(DHTPIN, DHTTYPE); //Se inicia una variable que será usada por Arduino para comunicarse con el sensor
/////////////////////////PIN//PARA//EL//SENSOR//DE//CO////////////////////////////////////////
#include "MQ7.h"
MQ7 pinMQ(A0, 5.0);
//////////////////////////CONFIGURACION//DEL//SERVIDOR////////////////////////////////////////
const char* ssid     = STASSID;
const char* password = STAPSK;
const String url =  "/aire-lima-2.5/neodato/registrarMedidas";
const char* host = "www.impulso-peru.com"; //localhost or www.impulso-peru.com
const uint16_t port = 80;  // HTTP PORT
//////////////////////DEFINIMOS//EL//CODIGO//DEL//SENSOR//RESPECTIVAMENTE//////////////////////
String codigoCO = "ALIMAMQ001";
String codigoPM = "ALIPMSA001";
String codigoDH = "ALIMDHT001";
////////////////////CREAMOS//VARIABLES//PARA//GUARDAR//LOS//DATOS/////////////////////////////
String token = "$2y$10$.Qgbl9e1XVUlztQUZp1OI./E6T04BFemiGxy62R0KzZZ9gj5m9eFG";
float medidaCO = 0;
float medidaPM = 0;
float medidaTEM = 0;
float medidaHUM = 0;

WiFiClient client;
/////////////////////////////////////////////////////////////////////////////////////////////
unsigned long previousMillis = 0;
unsigned long previousMillis_2 = 0;
unsigned long previousMillis_3 = 0;
String enviardatos(String datos) {
  String linea = "error";

  // Use WiFiClient para crear conexiones TCP
  Serial.print("conectando al servidor:  ");
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);


  //void(*resetFunction)(void) = 0;

  if (!client.connect(host, port)) {
    Serial.println("Conexion Fallida - Intente Nuevamente");
    delay(500);
    //Serial.println("PM 2.5 se esta Reiniciando");
    //resetFunction();
    return linea;
  }

  client.print(String("POST ") + url + " HTTP/1.1" + "\r\n" +
               "Host: " + host + "\r\n" +
               "Accept: */*" + "*\r\n" +
               "Content-Length: " + datos.length() + "\r\n" +
               "Content-Type: application/x-www-form-urlencoded" + "\r\n" +
               "\r\n" + datos);
  delay(10);
  Serial.print("enviando por la URL: ");
  Serial.println(url + "->" + datos);
  Serial.println("Enviando datos a Servidor (BD)...");

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println("Cliente fuera de tiempo!");
      client.stop();
      return linea;
    }
  }
  // Lee todas las lineas que recibe del servidro y las imprime por la terminal serial
  while (client.available()) {
    linea = client.readStringUntil('\r');
  }


  Serial.println(linea);
  return linea;

}
void setup() {
  /////////////////////////////////////////////////////////////////////////////////////////////
  /* myFile = SD.open("credenciales.txt");
    if (myFile) {
    Serial.println("Verificando Credenciales");
    while (myFile.available()) {
      Serial.write(myFile.read());
      STASSID  String(myFile.read());
      //STAPSK = myFile.read();
    }
    myFile.close();
  } else {
    Serial.println("error opening credenciales.txt");
  }*/
  /////////////////////////////////////////////////////////////////////////////////////////////
  Serial.begin(9600);
  Serial.print("Node MCU Chip Id : ");
  String chip_id = String(ESP.getChipId());
  Serial.println(chip_id);
  /////////////////////////////////////////////////////////////////////////////////////////////
  dht.begin(); //Se inicia el sensor de DHT22
  /////////////////////////////////////////////////////////////////////////////////////////////
  Serial.println();
  Serial.println();
  /////////////////////////////////////////////////////////////////////////////////////////////
  Serial.print("'Conectando Modulo' a la RED WiFi espere por favor (...) : ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi Conectado");
  Serial.println("Mi direccion IP: ");
  Serial.println(WiFi.localIP());
  /////////////////////////////////////////////////////////////////////////////////////////////
  if (!SD.begin(PIN_CS)) {
    Serial.println("Memoria SD - No Inicializada (...)");
  } else {
    Serial.println("Memoria SD, listo para ser conectada (...)");
  }
}

void loop() {
  if (pms.read(data)) {
    unsigned long currentMillis_2 = millis();
    if (currentMillis_2 - previousMillis_2 >= 1000) { //Definimos el tiempo de envio
      previousMillis_2 = currentMillis_2;
      medidaCO = pinMQ.getPPM(); //Lemos la salida analógica del MQ
      medidaPM = data.PM_AE_UG_2_5;
      medidaTEM = dht.readTemperature(); //Se lee la temperatura
      medidaHUM = dht.readHumidity(); //Se lee la humedad

      Serial.println("-----------------------------------------");
      Serial.print("Medida CO:");
      Serial.println(medidaCO);
      Serial.println("-----------------------------------------");
      Serial.print("PM 2.5 (ug/m3): ");
      Serial.println(medidaPM);
      Serial.println("-----------------------------------------");
      Serial.print("Humedad: ");
      Serial.println(medidaHUM);
      Serial.print("Temperatura: ");
      Serial.println(medidaTEM);
      Serial.println("-----------------------------------------");

      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Guardando datos en la memoris SD (...)");
        guardarSD();
      } else {
        Serial.println("Guardando datos en el Servidor (...)");
        guardarHost();
      }

    }
  }

}

void guardarHost() {
  ///////////////TODOS//LOS//PINES//DEBEN//ESTAR//CONECTADOS//AL//NODE//MCU////////////////////
  unsigned long currentMillis = millis();
  /////////////////////////////////////////////////////////////////////////////////////////////
  if (currentMillis - previousMillis >= 5000) { //Definimos el tiempo de envio
    previousMillis = currentMillis;
    //aire-lima-2.5-v1.6/neodato/registrarMedidas.php?codigox=ACALLMQ001&medidax=355.00&codigoy=BLIPMSA001&mediday=103.00&codigoz=CSJLDHT001&temperaturaz=18.30&humedadz=90.10
    enviardatos("codigox=" + codigoCO  +
                "&medidax=" + medidaCO +
                "&codigoy=" + codigoPM +
                "&mediday=" + medidaPM +
                "&codigoz=" + codigoDH +
                "&temperaturaz=" + medidaTEM +
                "&humedadz=" + medidaHUM +
                "&token=" + token);
  }
  /////////////////////////////////////////////////////////////////////////////////////////////
}
void guardarSD() {
  unsigned long currentMillis_3 = millis();
  data_file = SD.open("aire_libre.txt", FILE_WRITE);
  /////////////////////////////////////////////////////////////////////////////////////////////
  if (currentMillis_3 - previousMillis_3 >= 2000) { //Definimos el tiempo de envio
    previousMillis_3 = currentMillis_3;
    if (data_file) {

      data_file.println("codigox=" + codigoCO  +
                        "&medidax=" + medidaCO +
                        "&codigoy=" + codigoPM +
                        "&mediday=" + medidaPM +
                        "&codigoz=" + codigoDH +
                        "&temperaturaz=" + medidaTEM +
                        "&humedadz=" + medidaHUM +
                        "&token=" + token);
                        
      Serial.println("codigox=" + codigoCO  +
                        "&medidax=" + medidaCO +
                        "&codigoy=" + codigoPM +
                        "&mediday=" + medidaPM +
                        "&codigoz=" + codigoDH +
                        "&temperaturaz=" + medidaTEM +
                        "&humedadz=" + medidaHUM +
                        "&token=" + token);

      data_file.flush(); //saving the file
      data_file.close(); //closing the file
    }
    else {
      Serial.println("No se puede registrar en el archivo 'aire_libre.txt'");
    }
    
  }

}
