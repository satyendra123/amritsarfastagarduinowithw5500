#include <SPI.h>
#include <Ethernet.h>

#define relayPin 8

//ENTRY
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 241);

//EXIT
//byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEC};
//IPAddress ip(192, 168, 1, 241);

EthernetServer server(6000);
EthernetClient client;

String clientData = "";

boolean IsClientConnected = false;
boolean IsClientAlive = false;

unsigned long currentMillis, previousMillis, healthPacketMillis, reconnectMillis;
const unsigned long healthPacketInterval = 3000; // 3 seconds
const unsigned long reconnectInterval = 5000; // 5 seconds

void setup() {
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.begin(9600);

  while (!Serial) {
    ; // wait for the serial port to connect, needed for native USB port only
  }

  Serial.print("Machine Gate IP: ");
  Serial.println(Ethernet.localIP());

  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);

  IsClientConnected = false;

  currentMillis = 0;
  previousMillis = 0;
  healthPacketMillis = 0;
  reconnectMillis = 0;
}

void loop() {
  if (!IsClientConnected) {
    EthernetClient newClient = server.available();
    if (newClient) {
      client = newClient;
      IsClientConnected = true;
      client.flush();
      Serial.println("Client Connected");
      client.println("Connected to Arduino");
    }
  }

  if (IsClientConnected) {
    if (client.available() > 0) {
      char thisChar = client.read();
      if (thisChar == '|') {
        clientData = "";
      } else if (thisChar == '%') {
        Serial.println(clientData);
        if (clientData.equals("ENTRY")) {
          Serial.println("Barrier is opening");
          digitalWrite(relayPin, LOW);
          delay(500);
          digitalWrite(relayPin, HIGH);
          delay(500);
        }
      } else {
        clientData += thisChar;
      }
    }

    // Check and send the health packet every 3 seconds
    currentMillis = millis();
    if (currentMillis - previousMillis >= healthPacketInterval) {
      previousMillis = currentMillis;
      client.println("|HLT%");
    }

    // Check if the client is still connected and reconnect if necessary
    if (!client.connected()) {
      Serial.println("Client Disconnected");
      IsClientConnected = false;
    }
  }

}
