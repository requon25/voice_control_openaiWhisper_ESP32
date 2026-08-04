#include <WiFi.h>
#include <WebSocketsClient.h>
#include <driver/i2s.h>

// Network config
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
const char* websocket_server = "192.168.0.13"; // Your device IP
const int websocket_port = 8765;

// Hardware Pins
#define I2S_WS 15
#define I2S_SD 17
#define I2S_SCK 18

#define TOUCH_PIN 4 // TTP223 Pin

#define LED_PIN 46 // Test Pin

WebSocketsClient webSocket;
bool wasTouching = false;

// ----------------Functions
void setupI2S() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 512,
    .use_apll = false
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
}

// This function receives responses sent by the laptop via WebSocket
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    String message = String((char*)payload);
    
    if (message == "LED_ON") {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("-> Command received: LED ON");
    } 
    else if (message == "LED_OFF") {
      digitalWrite(LED_PIN, LOW);
      Serial.println("-> Command received: LED OFF");
    }
  }
}

// ------------MAIN
void setup() {
  Serial.begin(115200);

  pinMode(TOUCH_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);

  // ------WiFi config
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");

  setupI2S();
  webSocket.begin(websocket_server, websocket_port, "/");
  
  // Assign event handler for received messages
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();

  bool isTouching = (digitalRead(TOUCH_PIN) == HIGH);

  if (isTouching) {
    wasTouching = true;
    
    int32_t i2s_raw[256];
    size_t bytes_read = 0;
    i2s_read(I2S_NUM_0, i2s_raw, sizeof(i2s_raw), &bytes_read, portMAX_DELAY);

    int samples = bytes_read / sizeof(int32_t);
    int16_t pcm16_buffer[256];

    for (int i = 0; i < samples; i++) {
      // Gain adjustment and conversion to 16-bit PCM
      pcm16_buffer[i] = (int16_t)(i2s_raw[i] >> 14); 
    }

    // Send binary blocks via WebSocket
    webSocket.sendBIN((uint8_t*)pcm16_buffer, samples * sizeof(int16_t));
  } 
  else if (wasTouching) {
    wasTouching = false;

    // Notify end of word/phrase
    webSocket.sendTXT("END");
    Serial.println("Sensing finished. END command sent.");
  }
}