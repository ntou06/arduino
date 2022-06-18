

#include <WiFi.h>
#include <PubSubClient.h>
#include <SimpleDHT.h>

#define MQTT_SERVER_IP "broker.emqx.io"
#define MQTT_SERVER_PORT 1883
#define MQTT_ID "ESP32-TEST" 
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""

char _lwifi_ssid[] = "Doreen";
char _lwifi_pass[] = "19698432";
String receivedTopic="";
String receivedMsg="";
bool waitForE=true;
bool ended=true;
bool pubCtrl=false;

WiFiClient mqttClient;
PubSubClient myClient(mqttClient);
unsigned long lastMsg = 0;


#define RELAY_PIN_1 16

void connectMQTT(){
  while (!myClient.connected()){
    if (!myClient.connect(MQTT_ID,MQTT_USERNAME,MQTT_PASSWORD))
    {
      delay(5000);
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++){ 
    Serial.print((char)payload[i]);
  }
  Serial.println();

 
  payload[length] = '\0';
  
  String message = (char *)payload;
 
  if (strcmp(topic, "hank/lamp_switch") == 0){
    if(message == "On"){ 
      digitalWrite(RELAY_PIN_1, HIGH);
      Serial.println("lamp_switch true");
    }
    if(message == "Off"){
      digitalWrite(RELAY_PIN_1, LOW);
      Serial.println("lamp_switch false");      
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!myClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32-TEST";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (myClient.connect(clientId.c_str())) {
      Serial.println("connected");
      myClient.publish("hank/reconnect", "hello world");
      myClient.subscribe("hank/lamp_switch");
    } else {
      Serial.print("failed, rc=");
      Serial.print(myClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

int pinDHT = 4; 
SimpleDHT11 dht11(pinDHT);

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN_1,OUTPUT);
  
  myClient.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  myClient.setCallback(mqttCallback);
  
  WiFi.disconnect();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(_lwifi_ssid, _lwifi_pass);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }
  delay(300);
}

void loop() {
  if (!myClient.connected()) {
    reconnect(); 
  }
  myClient.loop();
  
  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    byte temperature = 0;
    byte humidity = 0;
    int err = SimpleDHTErrSuccess;
    if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
      Serial.print("Read DHT11 failed, err=");
      Serial.println(err);
      return;
    }
    
    myClient.publish(String("hank/temperature").c_str(),String((int)(temperature)+String("°C")).c_str());
    myClient.publish(String("hank/humidity").c_str(),String((int)(humidity)+String("%")).c_str());
    
    Serial.print((String("溫度=")+(int)(temperature)+String("°C, "))); 
    Serial.println((String("濕度=")+(int)(humidity)+String("%")));
  }
}
