#include <ModbusMaster.h>
#include <WiFi.h>
#include <ArduinoJson.h>

HardwareSerial mySerial(1);
ModbusMaster node;


const char* ssid = "Z-tech";
const char* wifiPassword = "ngokngok"; 
const char* serverUrl = "http://192.168.1.3/api/sensor/index.php"; 

WiFiClient client;

void setup() {
  
  Serial.begin(9600); //  Inisialisasi komunikasi serial untuk monitoring melalui Serial Monitor
  mySerial.begin(9600, SERIAL_8N1, 4, 0)


  WiFi.begin(ssid, wifiPassword);
  while (WiFi.status() != WL_CONNECTED) {
    
    delay(1000);
    
    Serial.println("Connecting to WiFi..."); //  Menampilkan status saat mencoba koneksi ke WiFi
  }
  Serial.println("Connected to WiFi"); //  Menampilkan pesan ketika berhasil terhubung ke WiFi

  node.begin(1, mySerial); // Inisialisasi komunikasi Modbus dengan slave ID 1 melalui UART1
}

void loop() {
  uint8_t result;


  result = node.readHoldingRegisters(4, 120);

  if (result == node.ku8MBSuccess) {
    
    Serial.println("Read Holding Registers:"); //  Menampilkan pesan ketika pembacaan register berhasil

    StaticJsonDocument<200> jsonDoc;

    for (int i = 0; i < 120; i++) {
      
      int16_t regValue = static_cast<int16_t>(node.getResponseBuffer(i)); 

      float scaledValue = regValue / 10.0; 

      jsonDoc[String("TEMP") + String(i)] = scaledValue;

      Serial.print("Register 0x"); //  Menampilkan alamat register dalam format heksadesimal
      Serial.print(4 + i, HEX);
      Serial.print(": ");
      Serial.println(scaledValue); //  Menampilkan nilai sensor setelah diskalakan
    }

   
    String jsonBuffer;
    serializeJson(jsonDoc, jsonBuffer);
    
    sendDataToServer(jsonBuffer); 

  } else {
    Serial.print("Error reading registers: "); //  Menampilkan pesan error jika pembacaan register gagal

    Serial.println(result, HEX); //  Menampilkan kode error dalam format heksadesimal
  }

  delay(1000); 
}

void sendDataToServer(const String& jsonData) {
  if (WiFi.status() == WL_CONNECTED) {
    if (client.connect("192.168.1.3", 80)) {
      

      client.print(String("POST /api/sensor/index.php HTTP/1.1\r\n") +
                   "Host: 103.59.94.18\r\n" +
                   "Content-Type: application/json\r\n" +
                   "Content-Length: " + jsonData.length() + "\r\n" +
                   "Connection: close\r\n\r\n" +
                   jsonData);
      
  
      while (client.connected() || client.available()) {
        if (client.available()) {
          String line = client.readStringUntil('\n');
          Serial.println(line); //  Menampilkan respons dari server
        }
      }
      client.stop(); 

      Serial.print("Data sent to server: "); // Menampilkan pesan setelah data dikirim ke server

      Serial.println(jsonData); // DMenampilkan data JSON yang dikirim
    } else {
      Serial.println("Connection to server failed"); //  Menampilkan pesan jika koneksi ke server gagal
    }
  }
}
