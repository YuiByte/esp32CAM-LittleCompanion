#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>

// Camera pin definition for AI Thinker
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Wi-Fi credentials
const char* ssid = "";
const char* password = "";

// Telegram bot credentials
const char* botToken = "";
const char* chatID = "";

// Interval to take pictures (10 minutes in milliseconds)
unsigned long interval = 300000;  // 10 minutes (600000 ms)
unsigned long previousMillis = 0;

void setup() {
  Serial.begin(115200);

  // Camera init
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 4;
    config.fb_count = 1;
  }

  // Init camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  s->set_special_effect(s, 2); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)


  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi");
  sendOnlineNotification();
}

void sendOnlineNotification() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();  // Ignore SSL certificate validation

    if (client.connect("api.telegram.org", 443)) {
      String message = "ESP32-CAM is now online!";
      String url = "/bot" + String(botToken) + "/sendMessage?chat_id=" + String(chatID) + "&text=" + message;
      
      // Send the request to Telegram
      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: api.telegram.org\r\n" +
                   "Connection: close\r\n\r\n");

      // Wait for response and read it (optional)
      while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
          break;
        }
      }

      Serial.println("Online notification sent to Telegram");
    } else {
      Serial.println("Failed to connect to Telegram");
    }
  }
}

void loop() {
  unsigned long currentMillis = millis();

  // Check if 10 minutes have passed
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Take a picture
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }

    // Send the picture to Telegram
    if(WiFi.status() == WL_CONNECTED) {
      WiFiClientSecure client;
      client.setInsecure();  // Ignore SSL certificate validation

      if (client.connect("api.telegram.org", 443)) {
        String head = "--boundary\r\nContent-Disposition: form-data; name=\"chat_id\"\r\n\r\n" + String(chatID) + "\r\n--boundary\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"esp32cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
        String tail = "\r\n--boundary--\r\n";

        // Start the HTTP POST request
        client.print(String("POST ") + "/bot" + botToken + "/sendPhoto HTTP/1.1\r\n" +
                     "Host: api.telegram.org\r\n" +
                     "Content-Type: multipart/form-data; boundary=boundary\r\n" +
                     "Content-Length: " + String(head.length() + fb->len + tail.length()) + "\r\n" +
                     "Connection: close\r\n\r\n");

        client.print(head);  // Send the form header
        client.write(fb->buf, fb->len);  // Send the picture payload
        client.print(tail);  // Send the form tail

        // Read server response
        while (client.connected()) {
          String line = client.readStringUntil('\n');
          if (line == "\r") {
            break;
          }
        }

        Serial.println("Picture sent to Telegram");
      } else {
        Serial.println("Connection to Telegram failed");
      }
    }

    esp_camera_fb_return(fb);
  }
}