// #include <ESP8266WiFi.h>
// #include <ArduinoJson.h>

// #define R D5
// #define G D6
// #define B D7

// WiFiServer server(80);

// void setup() {
//     Serial.begin(115200);

//     pinMode(R, OUTPUT);
//     pinMode(G, OUTPUT);
//     pinMode(B, OUTPUT);

//     digitalWrite(R, LOW);
//     digitalWrite(G, LOW);
//     digitalWrite(B, LOW);

//     Serial.flush();
//     Serial.println();
//     Serial.print("Conectando");
//     WiFi.begin("Familiadoremi", "Deus12345");

//     while (WiFi.status() != WL_CONNECTED) {
//         delay(100);
//         Serial.print(".");
//     }

//     Serial.println("");
//     Serial.println("Conectado!");

//     IPAddress ip(192, 168, 3, 5);
//     IPAddress gateway(192, 168, 1, 1);
//     IPAddress subnet(255, 255, 255, 0);
//     Serial.print("Configurando IP fixo para : ");
//     Serial.println(ip);
     
//     WiFi.config(ip, gateway, subnet);
//     server.begin();

//     Serial.print("Server em: ");
//     Serial.println(WiFi.localIP());
// }

// void loop() {
//     WiFiClient client = server.available();

//     if (!client) {
//         return;
//     }

//     Serial.println("Cliente conectado!");

//     String req = client.readString();

//     DynamicJsonDocument data(200);
//     deserializeJson(data, req.substring(req.indexOf("{")));

//     analogWrite(R, data["r"]);
//     analogWrite(G, data["g"]);
//     analogWrite(B, data["b"]);
// }