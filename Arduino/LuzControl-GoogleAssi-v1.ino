/*
  DETALLES
   https://things.ubidots.com/api/v1.6/devices/googletelematica?token=BBFF-5zs6veBr5nZX03SWYYkiTL1bERgaJ4
   {"luzsala":{{NumberField}}}
   {"luzsala":0}
   temperatura*3.3*100/1024
*/
#include "UbidotsESPMQTT.h"

// CONECTAR A NUESTRO SERVER WEB Y BRINDAR ACCESO A INTERNET
#define TOKEN "BBFF-5zs6veBr5nZX03SWYYkiTL1bERgaJ4" // Your Ubidots TOKEN
#define WIFINAME "INTACHABLE.Phone/>" //Your SSID
#define WIFIPASS "R00000AZA" // Your Wifi Pass
#define DEVICE_LABEL "googletelematica"

//VARIABLE QUE SE MOSTRARAN EN UBIDOTS y
#define VARIABLE_LABEL1  "luzsala"  // Nombre de variable en Ubidots
#define VARIABLE_LABEL2  "luzdormitorio"  // Nombre de variable en Ubidots
#define VARIABLE_LABEL3  "luzcocina"  // Nombre de variable en Ubidots
// DHT22
#include "DHT.h" //cargamos la librería DHT
#define DHTTYPE DHT22 //Se selecciona el DHT22(hay otros DHT)



const int ERROR_VALUE = 65535;  // Valor de error cualquiera

// CANTIDAD DE VARIABLE A LAS QUE EL PROGRAMA SE VA A SUSCRIBIR
const uint8_t NUMBER_OF_VARIABLES = 3; // Cantidad de variables a las que el programa se va a suscribir
char * variable_labels[NUMBER_OF_VARIABLES] = {"luzsala", "luzdormitorio", "luzcocina"}; // Nombres de las variables

// DEFINIENDO PINES SALIDAS
#define sala D0
#define dormitorio D1
#define cocina D2
#define DHTPIN D5 //Seleccionamos el pin en el que se conectará el sensor

//DEFINIENDO ESTADOS
float estadosala;
float estadodormitorio;
float estadococina;
float value; // Variable para almacenar el dato de entrada.
uint8_t variable; // Para usar en el switch case

/////////Funciones Auxiliares//////////////
Ubidots ubiClient(TOKEN);
WiFiClient client;
DHT dht(DHTPIN, DHTTYPE); //Se inicia una variable que será usada por Arduino para comunicarse con el sensor
/////////Funciones y Algoritmo /////////////
void callback(char* topic, byte* payload, unsigned int length) {
  char* variable_label = (char*)malloc(sizeof(char) * 30);
  get_variable_label_topic(topic, variable_label);
  value = btof(payload, length);
  set_state(variable_label);
  execute_cases();
  free(variable_label);

  ////////////LUCES/////////////
  if (estadosala == 1) {
    digitalWrite(sala, HIGH);
  } else {
    digitalWrite(sala, LOW);
  }

  if (estadodormitorio == 1) {
    digitalWrite(dormitorio, HIGH);
  } else {
    digitalWrite(dormitorio, LOW);
  }

  if (estadococina == 1) {
    digitalWrite(cocina, HIGH);
  } else {
    digitalWrite(cocina, LOW);
  }


}

//
void get_variable_label_topic(char * topic, char * variable_label) {
  Serial.print("Topic: ");
  Serial.println(topic);
  sprintf(variable_label, "");

  for (int i = 0; i < NUMBER_OF_VARIABLES; i++) {
    char * result_lv = strstr(topic, variable_labels[i]);
    if (result_lv != NULL) {
      uint8_t len = strlen(result_lv);
      char result[100];
      uint8_t i = 0;
      for (i = 0; i < len - 3; i++) {
        result[i] = result_lv[i];
      }
      result[i] = '\0';
      Serial.print("Labels is: ");
      Serial.println(result);
      sprintf(variable_label, "%s", result);
      break;
    }
  }

}

// cast from an array of chars to float value.
float btof(byte * payload, unsigned int length) {

  char* demo_ = (char*)malloc(sizeof(char) * 10);
  for (int i = 0; i < length; i++) {
    demo_[i] = payload[i];
  }
  return atof(demo_);
}

// State machine to use switch case
void set_state(char* variable_label) {
  variable = 0;
  for (uint8_t i = 0; i < NUMBER_OF_VARIABLES; i++) {
    if (strcmp(variable_label, variable_labels[i]) == 0) {
      break;
    }
    variable++;
  }
  if (variable >= NUMBER_OF_VARIABLES)variable = ERROR_VALUE;

}

// Function with switch case to determine which variable changed and assigned the value accordingly to the code variable
void execute_cases() {
  switch (variable) {
    case 0:
      estadosala = value;
      Serial.print("Luz Sala: ");
      Serial.println(estadosala);
      Serial.println();
      break;
    case 1:
      estadodormitorio = value;
      Serial.print("Luz Dormitorio: ");
      Serial.println(estadodormitorio);
      Serial.println();
      break;
    case 2:
      estadococina = value;
      Serial.print("Luz Cocina: ");
      Serial.println(estadococina);
      Serial.println();
      break;
    case ERROR_VALUE:
      Serial.println("ERROR EN 'execute_cases'...");
      Serial.println();
      break;
    default:
      Serial.println("Default...");
      Serial.println();
  }

}
/****************************************

   Funcion principal

 ****************************************/

void setup() {
  // Sets the broker properly for the business account
  ubiClient.ubidotsSetBroker("industrial.api.ubidots.com");
  // Pass a true or false bool value to activate debug messages
  ubiClient.setDebug(true);

  Serial.begin(115200);
  dht.begin(); //Se inicia el sensor

  pinMode(sala, OUTPUT);
  pinMode(dormitorio, OUTPUT);
  pinMode(cocina, OUTPUT);

  ubiClient.wifiConnection(WIFINAME, WIFIPASS);
  ubiClient.begin(callback);
  if (!ubiClient.connected()) {
    ubiClient.reconnect();
  }

  char* deviceStatus = getUbidotsDevice(DEVICE_LABEL);
  if (strcmp(deviceStatus, "404") == 0) {
    //Insert your variable Labels and the value to be sent
    ubiClient.add("luzsala", 0);
    ubiClient.ubidotsPublish(DEVICE_LABEL);

    ubiClient.add("luzdormitorio", 0);
    ubiClient.ubidotsPublish(DEVICE_LABEL);

    ubiClient.add("luzcocina", 0);
    ubiClient.ubidotsPublish(DEVICE_LABEL);

    ubiClient.loop();
  }
  //Insert the Device and Variable's Labels
  ubiClient.ubidotsSubscribe(DEVICE_LABEL, VARIABLE_LABEL1);
  ubiClient.ubidotsSubscribe(DEVICE_LABEL, VARIABLE_LABEL2);
  ubiClient.ubidotsSubscribe(DEVICE_LABEL, VARIABLE_LABEL3);
  Serial.println(variable_labels[1]); //MUESTA EL VALOR UNO

}

void loop() {

  if (!ubiClient.connected()) {
    //Insert the Device and Variable's Labels
    ubiClient.reconnect();
    ubiClient.ubidotsSubscribe(DEVICE_LABEL, VARIABLE_LABEL1);
    ubiClient.ubidotsSubscribe(DEVICE_LABEL, VARIABLE_LABEL2);
    ubiClient.ubidotsSubscribe(DEVICE_LABEL, VARIABLE_LABEL3);
  }

  float t = dht.readTemperature(); //Se lee la temperatura
  float h = dht.readHumidity(); //Se lee la humedad


  ubiClient.add("temperatura", t);
  ubiClient.ubidotsPublish(DEVICE_LABEL);

  ubiClient.add("humedad", h);
  ubiClient.ubidotsPublish(DEVICE_LABEL);


  ubiClient.loop();
  delay(500);

 /*btnUnoState = digitalRead(puls_uno);
  btnDosState = digitalRead(puls_dos);

  Serial.print("ESTADO UNO: ");
  Serial.println(btnUnoState);

  Serial.print("ESTADO DOS: ");
  Serial.println(btnDosState);


  if (btnUnoState == 1) {
    digitalWrite(pinLed, HIGH);
    servoUno.write(0);
    delay(15);
  }

  if (btnDosState == 1) {
    digitalWrite(pinLed, HIGH);
    servoUno.write(180);
    delay(15);
  }


  if (btnUnoState == 0 && btnDosState == 0) {
    digitalWrite(pinLed, LOW);
    servoUno.write(90);
    delay(15);
  }
*/
}
////////////MÁS FUNCIONES/////////////
char* getUbidotsDevice(char* deviceLabel) {

  char* data = (char*)malloc(sizeof(char) * 700);
  char* response = (char*)malloc(sizeof(char) * 400);

  sprintf(data, "GET /api/v1.6/devices/%s/", deviceLabel);
  sprintf(data, "%s HTTP/1.1\r\n", data);
  sprintf(data, "%sHost: industrial.api.ubidots.com\r\nUser-Agent:googletelematica/1.0\r\n", data);
  sprintf(data, "%sX-Auth-Token: %s\r\nConnection: close\r\n\r\n", data, TOKEN);

  if (client.connect("industrial.api.ubidots.com", 80)) {
    client.println(data);
  } else {
    return "e";
  }

  free(data);
  int timeout = 0;
  while (!client.available() && timeout < 5000) {
    timeout++;
    if (timeout >= 4999) {
      return "e";
    }
    delay(1);

  }
  int i = 0;
  while (client.available()) {
    response[i++] = (char)client.read();
    if (i >= 399) {
      break;
    }
  }

  char* pch;
  char* statusCode;
  int j = 0;

  pch = strtok(response, " ");
  while (pch != NULL) {
    if (j == 1) {
      statusCode = pch;
    }
    pch = strtok(NULL, " ");
    j++;
  }
  free(response);
  return statusCode;

}
