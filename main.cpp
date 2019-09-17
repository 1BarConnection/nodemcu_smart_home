#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>

#define ONE_WIRE_BUS 12

int blue_led = 13;
int button = 15;
int window_sensor = 16;
int motion_sensor = 14;
int yellow_led = 2;
int relay = 5;
int photores = A0;
int red_led = 0;
int green_led = 4;
int button_state = 0;
int button_old_state = 0;
int blue_led_state = 0;
int photores_value = 0;


OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

const char* mqtt_server = "10.0.2.15"; //set your homeassistant server IP here
WiFiClient espClient;
PubSubClient client(espClient);

float celsius;
char cels[8];

void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  Serial.print("Message: ");

  String message;
  for (int i = 0; i < length; i++) {
    message = message + (char)payload[i];  //Conver *byte to String
  }
   Serial.print(message);
  if(message == "0") {digitalWrite(relay,LOW);}
  if(message == "1") {digitalWrite(relay,HIGH);}

  Serial.println();
}

void setup() {
  // put your setup code here, to run once:

  pinMode(blue_led,OUTPUT);
  pinMode(button,INPUT);
  pinMode(window_sensor,INPUT_PULLDOWN_16);
  pinMode(motion_sensor,INPUT);
  pinMode(yellow_led,OUTPUT);
  pinMode(relay,OUTPUT);
  pinMode(photores,INPUT);
  pinMode(red_led,OUTPUT);
  pinMode(green_led,OUTPUT);

  Serial.begin(9600);
  Serial.println("Readings will apear here.");

  sensors.begin();

  WiFi.begin("SSID","password"); //set ssid name and password of wifi ap
  Serial.print("Connecting");
  while(WiFi.status() != WL_CONNECTED){
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server,1883);
  client.setCallback(callback);

  if(client.connect("Temperature")){
    Serial.println("Temperature - MQTT - OK");
  }else{
    Serial.println("Temperature - MQTT - ERROR");
  }
  if(client.connect("Window")){
    Serial.println("Window - MQTT - OK");
  }else{
    Serial.println("Window - MQTT - ERROR");
  }
  if(client.connect("Motion")){
    Serial.println("Motion - MQTT - OK");
  }else{
    Serial.println("Motion - MQTT - ERROR");
  }
  if(client.connect("Light")){
    Serial.println("Light - MQTT - OK");
    client.subscribe("inLight");
  }else{
    Serial.println("Light - MQTT - ERROR");
  }
  if(client.connect("Photores")){
    Serial.println("Photores - MQTT - OK");
  }else{
    Serial.println("Photores - MQTT - ERROR");
  }
}

void loop() {
  // put your main code here, to run repeatedly:

  button_state = digitalRead(button);

  if(button_state != button_old_state){
    button_old_state = button_state;

    if(button_state == 1){
      blue_led_state = !blue_led_state;
    }
  }

  if(blue_led_state == 1){
    digitalWrite(blue_led,HIGH);
    sensors.requestTemperatures();
    Serial.print("Temperature is: ");
    Serial.print(sensors.getTempCByIndex(0));
    celsius=sensors.getTempCByIndex(0);
    Serial.println(" °C");
    if(!client.connect("Temperature")){
      Serial.println("Temperature - Connecting...");
      client.connect("Temperature");
    }
    dtostrf(celsius,6,2,cels);
    client.publish("outTemperature",cels);

    if(!client.connect("Window")){
      Serial.println("Window - Connecting...");
      client.connect("Window");
    }
    if(digitalRead(window_sensor) == HIGH){
      client.publish("outWindow","CLOSED");
      Serial.println("Windows is closed.");
    }else{
      client.publish("outWindow","OPENED");
      Serial.println("Windows is opened.");
    }

    if(!client.connect("Motion")){
      Serial.println("Motion - Connecting...");
      client.connect("Motion");
    }
    if(digitalRead(motion_sensor) == HIGH){
      client.publish("outMotion","DETECT");
      Serial.println("Motion sensor has detected something.");
      digitalWrite(yellow_led,HIGH);
    }else{
      client.publish("outMotion","CLEAR");
      Serial.println("Motion sensor hasn't detected anything.");
      digitalWrite(yellow_led,LOW);
    }

    photores_value = analogRead(photores);
    Serial.print("Photoresistor value: ");
    Serial.println(photores_value);
    if(photores_value <= 250){
      Serial.println("It's quite dark here.");
      client.publish("outPhotores","It's quite dark here.");
      digitalWrite(red_led,HIGH);
      digitalWrite(green_led,LOW);
    }else{
      Serial.println("Nice light level.");
      client.publish("outPhotores","Nice light level.");
      digitalWrite(green_led,HIGH);
      digitalWrite(red_led,LOW);
    }

    delay(1000);
  }

  if(blue_led_state == 0){
    digitalWrite(blue_led,LOW);
    delay(250);
  }

  client.loop();


}
