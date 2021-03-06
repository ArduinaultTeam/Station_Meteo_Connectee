/*************************************************************************************
  Nom
        StationMeteo.ino
  Role
        Station météo connectée basée sur module NodeMCU ESP8266 Wifi
        Fonction :
          - Acquérir la température et l'hygrométrie avec un DHT11
          - Acquérir la pression atmosphérique (a venir)
          - Mesurer la pluviométrie (a venir)
          - Mettre à disposition ces informations par l'intermédiare d'un serveur web
          - Transmettre ces informations sur un serveur distant (type thingspeak)

  Auteur
        Laurent Macron
        Thierry Dezormeaux

  Version / Date / Commentaire / Auteur
        0.01 / 03-01-2017 / Version initial
*************************************************************************************/

/***********  Ajout de bibliothèque  ************************************************/
#include <dht.h> //Librairie pour le capteur DHT: http://arduino.cc/playground/Main/DHTLib
#include "Network_Setting.h" //Network settings: SSID & password in separate file
#include <ESP8266WiFi.h>

/***********  Déclaration des CONSTANTES  *******************************************/
#define DHT11_PIN 5 //The data I/O pin connected to the DHT11 sensor : GPIO5 = D1 of NodeMCU ESP8266 Board

/***********  Déclaration des variables globales  ***********************************/
dht DHT;  //Creation de l'objet DHT

WiFiServer server(80); // Create an instance of the server, specify the port to listen on as an argument

// variable pour stocker valeur lue par sensor
int humidity_DHT;
int temperature_DHT;

bool SSID_defined = false;  //define if network is configured

/***********  setup, to run once  ***************************************************/
void setup() {
  //On ouvre un connexion série pour le terminal
  Serial.begin(115200);
  Serial.println("Programme Station Meteo initialisée\n");

  // Set WiFi to station mode and disconnect from an Access Point if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  if(SSID_defined){ //Does network configured ?
    // Connexion wifi
    connectWifi();
    // Start the server
    server.begin();
    Serial.println("Server started");
    Serial.println();
  
    Serial.print("DHTLib LIBRARY VERSION: ");
    Serial.println(DHT_LIB_VERSION);
    Serial.println();
    Serial.println("Type,\tstatus,\tHumidity (%),\tTemperature (C)");
  
    pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  }
  else{
    //Configure Network
    Serial.println("Network not configured");
    
    Serial.println("scan start");

    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0)
      Serial.println("no networks found");
    else
    {
      Serial.print(n);
      Serial.println(" networks found");
      for (int i = 0; i < n; ++i)
      {
        // Print SSID and RSSI for each network found
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(WiFi.SSID(i));
        Serial.print(" (");
        Serial.print(WiFi.RSSI(i));
        Serial.print(")");
        Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
        delay(10);
      }
    }
    Serial.println("");
  }
}

/***********  main, run repeatedly **************************************************/
void loop() {

  //READ DATA
  Serial.print("DHT11, \t");
  int chk = DHT.read11(DHT11_PIN);
  switch (chk)
  {
    case DHTLIB_OK:
      Serial.print("OK,\t");
      break;
    case DHTLIB_ERROR_CHECKSUM:
      Serial.print("Checksum error,\t");
      break;
    case DHTLIB_ERROR_TIMEOUT:
      Serial.print("Time out error,\t");
      break;
    default:
      Serial.print("Unknown error,\t");
      break;
  }
  // DISPLAY DATA
  humidity_DHT = DHT.humidity;
  temperature_DHT = DHT.temperature;
  Serial.print(humidity_DHT, 1);
  Serial.print(",\t");
  Serial.println(temperature_DHT, 1);

  delay(2000);

  // Check if a client has connected
  WiFiClient client = server.available();
  if (client) {
    // Wait until the client sends some data
    Serial.println("new client");
    while (!client.available()) {
      delay(1);
    }
    // Read the first line of the request
    String req = client.readStringUntil('\r');
    Serial.println(req);
    client.flush();

    // Match the request
    if (req.indexOf("/on") != -1) {
      digitalWrite(LED_BUILTIN, LOW);
    }
    else if (req.indexOf("/off") != -1) {
      digitalWrite(LED_BUILTIN, HIGH);
    }

    client.flush();

    // Prepare the response
    String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    s += "<head>";
    s += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
    s += "<script src=\"https://code.jquery.com/jquery-2.1.3.min.js\"></script>";
    s += "<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.4/css/bootstrap.min.css\">";
    s += "</head>";
    s += "<div class=\"container\">";
    s += "<h1>Lamp Control</h1>";
    s += "<p><br/>Capteur temperature DHT 11:<br/> ";
    s += humidity_DHT;
    s += " %  ";
    s += temperature_DHT;
    s += " C";
    s += "</p><br/>";
    s += "<div class=\"row\">";
    s += "<div class=\"row\">";
    s += "<div class=\"row\">";
    s += "<div class=\"col-xs-6\"><input class=\"btn btn-block btn-lg btn-primary\" type=\"button\" value=\"On\" onclick=\"on()\"></div>";
    s += "<div class=\"col-xs-6\"><input class=\"btn btn-block btn-lg btn-danger\" type=\"button\" value=\"Off\" onclick=\"off()\"></div>";
    s += "</div>";
    s += "<div class=\"row\">";
    s += "</div></div>";
    s += "<script>function Refresh() {$.get(\"/refresh\");setTimeout(reloadpage, 500);}</script>";
    s += "<script>function on() {$.get(\"/on\");}</script>";
    s += "<script>function off() {$.get(\"/off\");}</script>";

    // Send the response to the client
    client.print(s);
    delay(1);
    Serial.println("Client disconnected");

    // The client will actually be disconnected
    // when the function returns and 'client' object is detroyed
  }
}

/************************************************************************************/
/***********  connectWifi ***********************************************************/
/************************************************************************************/
void connectWifi() {

  // We start by connecting to a WiFi network

  Serial.print("Connexion au WiFi ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);   // On se connecte

  int i = 0;
  while (WiFi.status() != WL_CONNECTED && i < 40) { // On attend max 20 s
    delay(500);
    Serial.print(".");
    i++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");  // on affiche les paramÃ¨tres
    Serial.println("WiFi connecté");
    Serial.print("Adresse IP du module EPC: ");
    Serial.println(WiFi.localIP());
    Serial.print("Adresse IP de la box : ");
    Serial.println(WiFi.gatewayIP());
  } else {
    Serial.println("");
    Serial.println("WiFi Not connected");
    Serial.println("");
  }
}
