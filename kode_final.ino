#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// Replace with your network credentials
const char* ssid = "ours";
const char* password = "tanyamahanin";

// Initialize Telegram BOT
#define BOTtoken "6983590652:AAGgFNx0BZLWGa2My8uk9vuu6xqxshx9ufA"  // your Bot Token (Get from Botfather)
#define CHAT_ID "1303585678"

#ifdef ESP8266
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const int sensorSaltPin1 = 32;
const int sensorSaltPin2 = 33; // Change to the GPIO pin for salt sensor on ESP32
const int pinSiramAir = 26;
const int pinSiramPupuk = 27;

void setup() {
  pinMode(sensorSaltPin1, INPUT);
  pinMode(sensorSaltPin2, INPUT);
  pinMode(pinSiramAir, OUTPUT);
  pinMode(pinSiramPupuk, OUTPUT);
  Serial.begin(115200);

#ifdef ESP8266
  configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
  client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
#endif

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
#ifdef ESP32
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
#endif

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
}

void loop() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  if (numNewMessages > 0) {
    for (int i = 0; i < numNewMessages; i++) {
      String chat_id = String(bot.messages[i].chat_id);
      if (chat_id != CHAT_ID) {
        bot.sendMessage(chat_id, "Unauthorized user", "");
        continue;
      }

      String text = bot.messages[i].text;
      if (text == "/get_salt") {
        float saltValue1 = measureSalt1();
        float saltValue2 = measureSalt2();

        int banyakNormal = 0;
        int banyakTinggi = 0;
        int banyakRendah = 0;

        String kelompokValueTDS1 = (saltValue1 >= 200 && saltValue1 <= 300) ? "Normal" : ((saltValue1 < 200) ? "Rendah" : "Tinggi");
        String kelompokValueTDS2 = (saltValue2 >= 200 && saltValue2 <= 300) ? "Normal" : ((saltValue2 < 200) ? "Rendah" : "Tinggi");

        if (kelompokValueTDS1 == "Normal"){
          banyakNormal += 1;
        } else if (kelompokValueTDS1 == "Rendah"){
          banyakRendah += 1;
        } else{
          banyakTinggi += 1;
        }

        if (kelompokValueTDS2 == "Normal"){
          banyakNormal += 1;
        } else if (kelompokValueTDS2 == "Rendah"){
          banyakRendah += 1;
        } else{
          banyakTinggi += 1;
        }

        float meanValueTDS = (saltValue1 + saltValue2) / 2;

        String kelompokMeanValueTDS = (meanValueTDS >= 200 && meanValueTDS <= 300) ? "Normal" : ((meanValueTDS < 200) ? "Rendah" : "Tinggi");

        if (kelompokMeanValueTDS == "Normal"){
          banyakNormal += 1;
        } else if (kelompokMeanValueTDS == "Rendah"){
          banyakRendah += 1;
        } else{
          banyakTinggi += 1;
        }

        float medianValueTDS = meanValueTDS;
        String kelompokMedianValueTDS = (medianValueTDS >= 200 && medianValueTDS <= 300) ? "Normal" : ((medianValueTDS < 200) ? "Rendah" : "Tinggi");

        if (kelompokMedianValueTDS == "Normal"){
          banyakNormal += 1;
        } else if (kelompokMedianValueTDS == "Rendah"){
          banyakRendah += 1;
        } else{
          banyakTinggi += 1;
        }        

        String kelompokModusValueTDS = ((banyakNormal > banyakTinggi) ? (banyakNormal > banyakRendah ? "Normal" : "Rendah") : (banyakTinggi > banyakRendah ? "Tinggi" : "Rendah"));


        String response1 = "Current salt value: " + String(saltValue1, 3) + " PPM dan " + String(saltValue2, 3) + " PPM";
        bot.sendMessage(chat_id, response1, "");

        String response2 = "Kesimpulannya: " + kelompokModusValueTDS;
        bot.sendMessage(chat_id, response2, "");
      } else if (text == "/siram_air") {
        String siramValue = siramAir();
        bot.sendMessage(chat_id, siramValue, "");
      } else if (text == "/siram_pupuk") {
        String siramValue = siramPupuk();
        bot.sendMessage(chat_id, siramValue, "");
      }
    }
  }

  delay(1000); // Adjust the delay based on your needs
}

float measureSalt1() {
   // Add your logic to measure salt value
  // Placeholder implementation, replace with actual sensor reading
  // Default salt value for demonstration

  // Read the analog in value:
  int sensorValue1 = analogRead(sensorSaltPin1);

  // Mathematical Conversion from ADC to conductivity (uSiemens)
  // Rumus berdasarkan datasheet
  float outputValueConductivity = (0.2142 * sensorValue1) + 494.93;

  // Mathematical Conversion from ADC to TDS (ppm)
  // Rumus berdasarkan datasheet
  float outputValueTDS = (0.1126 * sensorValue1) + 291.26;
  return outputValueTDS;
}

float measureSalt2() {
   // Add your logic to measure salt value
  // Placeholder implementation, replace with actual sensor reading
  // Default salt value for demonstration

  // Read the analog in value:
  int sensorValue1 = analogRead(sensorSaltPin2);

  // Mathematical Conversion from ADC to conductivity (uSiemens)
  // Rumus berdasarkan datasheet
  float outputValueConductivity = (0.2142 * sensorValue1) + 494.93;

  // Mathematical Conversion from ADC to TDS (ppm)
  // Rumus berdasarkan datasheet
  float outputValueTDS = (0.1126 * sensorValue1) + 291.26;
  return outputValueTDS;
}

String siramAir() {
  // Mengaktifkan siram air (misalnya, memberikan tegangan tinggi ke pin siram air)
  digitalWrite(pinSiramAir, HIGH);

  // Menunda selama beberapa waktu (sesuai kebutuhan aplikasi Anda)
  delay(2500);  // Contoh: Siram air selama 5 detik

  // Menonaktifkan siram air
  digitalWrite(pinSiramAir, LOW);

  return "Berhasil menyiram air";
}

String siramPupuk() {
  // Mengaktifkan siram Pupuk (misalnya, memberikan tegangan tinggi ke pin siram Pupuk)
  digitalWrite(pinSiramPupuk, HIGH);

  // Menunda selama beberapa waktu (sesuai kebutuhan aplikasi Anda)
  delay(2500);  // Contoh: Siram Pupuk selama 5 detik

  // Menonaktifkan siram Pupuk
  digitalWrite(pinSiramPupuk, LOW);

  return "Berhasil menyiram pupuk";
}
