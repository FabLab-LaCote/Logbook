/*
  Logbook for FabLab La Côte
  www.fablab-lacote.ch
  May 2015, Gregor Bruhin

*/

#include <SPI.h>
#include <Ethernet.h>
#include <PN532.h>


//define nfc SPI pins
#define SCK 7
#define MOSI 5
#define SS 4
#define MISO 6

PN532 nfc(SCK, MISO, MOSI, SS);

#define SoundPin 3
#define GreenLed A0
#define RedLed A1


byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char server[] = "www.fablab-lacote.ch";

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;


// nfc read card id
uint32_t id, prev_id;
float prev_millis;

void setup() {

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  Serial.println("Starting Up...");

  //8,9 as aditionnal input for future use
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);

  //A0 green led, A1 red led
  pinMode(GreenLed, OUTPUT); //green led
  pinMode(RedLed, OUTPUT); //red led

  pinMode(SoundPin, OUTPUT);


  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.print("Ethernet ok: ");

  // print your local IP address:
  Serial.println(Ethernet.localIP());


  Serial.println("Starting NFC...");

  //starting nfc reader
  nfc.begin();
  // configure board to read RFID tags and cards
  nfc.SAMConfig();

  for (int i = 1; i < 300; i++) {
    digitalWrite(SoundPin, HIGH);   // set the pin HIGH
    delayMicroseconds(400 - i);     // wait for a bit
    digitalWrite(SoundPin, LOW);    // set the pin LOW
    delayMicroseconds(400 - i);     // wait for a bit
  }

  Serial.println("Finished Startup...");
}

void loop()
{

  // look for MiFare type cards
  id = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A);

  if (id != 0) {

    //do not read repeatedly and wait 10 seconds
    if (prev_id != id or millis() > prev_millis + 10000) {
      Serial.print("Read card #"); Serial.println(id);
      http_request(id);
    }

    prev_id = id;
    prev_millis = millis();
  }

}

void http_request(uint32_t id) {
  Serial.println("connecting...");

  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("connected");
    // Make a HTTP request:
    //client.print("GET /iot/logbook/nfc.php?id=");
    client.print("GET /wp-admin/admin-ajax.php?action=fablab_logbook_entry&id=");
    client.print(id);
    client.print("&a=");
    client.print(digitalRead(8));
    client.print("&b=");
    client.print(digitalRead(9));
    client.println(" HTTP/1.1");
    client.println("Host: www.fablab-lacote.ch");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();
  }
  else {
    // ko you didn't get a connection to the server:
    Serial.println("connection failed");
  }

  String result = "";

  while (client.connected()) {

    // if there are incoming bytes available
    // from the server, read them and print them:
    while (client.available()) {

      char c = client.read();
      result = result + c;


      // Process data line by line and ignore HTTP headers and other useless info
      if (result.endsWith("\r\n\r\n") || result.endsWith("\n\n"))
      {
        Serial.println("CLEAN");
        Serial.println(result);
        Serial.println("END CLEAN");
        result = ""; //clean processed result line

      }
    }
  }
  
  if (result.startsWith("Hello")) {
    Serial.println(result);
    user_found();
  }
  else if (result.startsWith("Carte Inconnue")) {
    Serial.println(result);
    user_not_found();
  }
  else {
    Serial.print("not a result: >>");
    Serial.print(result);
    Serial.println("<<<");
  }








  Serial.println("disconnecting.");
  client.stop();

}

void user_found() {
  digitalWrite(GreenLed, HIGH);
  for (int i = 1; i < 100; i++) {
    digitalWrite(SoundPin, HIGH);   // set the pin HIGH
    delayMicroseconds(500);         // wait for a bit
    digitalWrite(SoundPin, LOW);    // set the pin LOW
    delayMicroseconds(500);         // wait for a bit
  }
  delay(2000);
  digitalWrite(GreenLed, LOW);
}

void user_not_found() {
  digitalWrite(RedLed, HIGH);
  for (int i = 1; i < 200; i++) {
    digitalWrite(SoundPin, HIGH);   // set the pin HIGH
    delayMicroseconds(2000);        // wait for a bit
    digitalWrite(SoundPin, LOW);    // set the pin LOW
    delayMicroseconds(2000);        // wait for a bit
  }
  delay(2000);
  digitalWrite(RedLed, LOW);
}


