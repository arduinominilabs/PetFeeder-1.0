/*************************************************************
 by Arduinominilabs 2019
 Smart Automatic Petfeeder 
 Alimentador automático de mascotas
 Control mediante Blynk
 Reloj NTP
 *************************************************************/

 //Librerias
 //link: https://github.com/esp8266/Arduino

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

//#include <Blynk.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Servo.h>
//#include <TimeLib.h>
//#include <TimeAlarms.h>

WiFiUDP ntpUDP;
// By default 'pool.ntp.org' is used with 60 seconds update interval and
// no offset
//NTPClient timeClient(ntpUDP);
// You can specify the time server pool and the offset, (in seconds)
// additionaly you can specify the update interval (in milliseconds).
// NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000)

//timezone sets
//For UTC -5.00 : -5 * 60 * 60 : -18000
//For UTC +1.00 : 1 * 60 * 60 : 3600
//For UTC +0.00 : 0 * 60 * 60 : 0
//for ar UTC -3.00: -3 * 60 * 60 : -10800

NTPClient timeClient(ntpUDP, "pool.ntp.org", -10800, 60000);


Servo myservo;  // create servo object to control a servo

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "WiFi-SSID-MI-RED";
char pass[] = "MI PASS";

const int ServoPin=4; //pin 24 es el D2 en Wemos d1 mini
const int ledPin =  D3; //pin 0 es el D3 en wemos d1 mini

int servo1=0;
int cantDiaria=0;
int time1=0; //time imput comida1
int time2=0; //time imput comida2
int time3=0; //time imput comida3
int time4=0; //time imput comida4
int cantidad=0; //tamaño porción
int horaActual=0;
int OnOff=0; //variable que controla si el dispositivo esta en funcionamiento o parado

unsigned long MillisInicio =0;
unsigned long MillisActual = millis();     // Almacena el tiempo actual
boolean stado=false;

WidgetLED led1(V4); //register to virtual pin 3

BLYNK_CONNECTED(){
  Blynk.syncVirtual(V2); //sincronizo los dos pin virtuales que en este caso corresponden a dos botones en blynk.
  Blynk.syncVirtual(V5); //simcronizo estado de funcionamiento on/off
  Blynk.syncVirtual(V6); //simcronizo imput dosificación diaria
  Blynk.syncVirtual(V8); //simcronizo imput time1
  Blynk.syncVirtual(V9); //simcronizo imput time2
  Blynk.syncVirtual(V10); //simcronizo imput time3
  Blynk.syncVirtual(V11); //simcronizo imput time4
  Blynk.syncVirtual(V12); //simcronizo imput tamaño porción
}

BLYNK_WRITE(V7){ //lee el estado del servidor y sincroniza al ultimo estado ..es decir "V7 escribe sobre el nodemcu". TRANSMITE AL NODECMU
  servo1=param.asInt();
  //prize ó premio
  if (OnOff==1)
  {
     if (servo1==1)
    {
    myservo.write(60); //gira
    }else{
    myservo.write(90);  //no gira
    }
  }
}

BLYNK_WRITE(V5){ //lee el estado del servidor y sincroniza al ultimo estado ..es decir "V5 escribe sobre el nodemcu". TRANSMITE AL NODECMU
  //estado del sismtema encendido o apagado
  OnOff=param.asInt();
 }

BLYNK_WRITE(V6){ //lee el estado del servidor y sincroniza al ultimo estado ..es decir "V6 escribe sobre el nodemcu". TRANSMITE AL NODECMU
  //esta variable almacena las porciones dosificadas
  cantDiaria=param.asInt();
 }

BLYNK_WRITE(V8){ //lee el estado del servidor y sincroniza al ultimo estado ..es decir "V8 escribe sobre el nodemcu". TRANSMITE AL NODECMU
  //seteo de comida1
  time1=param.asInt();
 }

 BLYNK_WRITE(V9){ //lee el estado del servidor y sincroniza al ultimo estado ..es decir "V9 escribe sobre el nodemcu". TRANSMITE AL NODECMU
  //seteo de comida2
  time2=param.asInt();
 }

 BLYNK_WRITE(V10){ //lee el estado del servidor y sincroniza al ultimo estado ..es decir "V10 escribe sobre el nodemcu". TRANSMITE AL NODECMU
  //seteo de comida3
  time3=param.asInt();
 }

 BLYNK_WRITE(V11){ //lee el estado del servidor y sincroniza al ultimo estado ..es decir "V11 escribe sobre el nodemcu". TRANSMITE AL NODECMU
  //seteo de comida4
  time4=param.asInt();
 }

  BLYNK_WRITE(V12){ //lee el estado del servidor y sincroniza al ultimo estado ..es decir "V12 escribe sobre el nodemcu". TRANSMITE AL NODECMU
  //esta variable almacena el tamaño de la porción en segundos.
  cantidad=param.asInt();
 }

void setup()
{
  // Debug console
  Serial.begin(9600);
  myservo.attach(ServoPin);
  
  timeClient.begin(); //cliente udp

  pinMode(ledPin, OUTPUT); // initialize the LED pin as an output
  
  Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8442);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8442);
}

void loop()
{
  Blynk.run();
  timeClient.update();
// -----------------------configuro una función para poder comparar con la hora de blynk


  if (OnOff==1)
{
    // turn LED on:
    digitalWrite(ledPin, HIGH);
  } else {
    // turn LED off:
    digitalWrite(ledPin, LOW);
}

  //if (OnOff==1)
  //{
  horaActual=timeClient.getHours()*3600+timeClient.getMinutes()*60+timeClient.getSeconds();
  Serial.println(horaActual);
  delay(1000);
  //}
//-------------------------

//reseteo contador porciones
if ((horaActual>0)&&(horaActual<time1))
{
cantDiaria=0;
Blynk.virtualWrite(V6,0); //cuando la hora sobrepasa las 00:00 y la actual es menor a la primera hora de la comida1. El monitor muestra cero "0"
}


//comida1
if ((horaActual==time1)&&(OnOff==1))
{
comida();
}

//comida2
if ((horaActual==time2)&&(OnOff==1))
{
comida();
}

//comida3
if ((horaActual==time3)&&(OnOff==1))
{
comida();
}

//comida4
if ((horaActual==time4)&&(OnOff==1))
{
comida();
}

//---------

dispensar(); //rutina para dispensar el alimento

//widget led comprobación orden
ledWidget();


}

void comida()
{
    
    MillisInicio = millis ();

    stado=true;
    myservo.write(60); //dispensa por 10seg
    //Serial.println("empezando a dispensar...");
  
}

void dispensar()
{
    if(stado == true){
    
  MillisActual = millis ();
  
  if(MillisActual - MillisInicio > (cantidad * 1000)) 
  {
       myservo.write(90); //para dejar de dispensar
       Serial.println("terminando a dispensar...");
       Blynk.virtualWrite(V6,cantDiaria+=1);
       Serial.println(cantDiaria);
       stado=false;
  }
  }
}

void ledWidget()
{
 if(OnOff==1)
 {
    if (myservo.read()==60)
    {
    led1.on();
    
    //Serial.println(time1);
    }else{
    led1.off();
    }
  }
 }



  
