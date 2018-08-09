#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "config.h"

WiFiClient espClient;
PubSubClient client(espClient);

#define DEBUG false
#define VIBRATION_SENSOR_PIN D4
#define N_SAMPLES 50
#define SAMPLE_INTERVAL 100
#define ACTIVE 1
#define INACTIVE 0

int samples[N_SAMPLES];
int sample_index = 0;
int current_state = INACTIVE;
unsigned long current_state_at = 0;
unsigned long last_sample_at = 0;

void restart() {
  delay(2000);
  ESP.restart();
}

void mqtt_connect() {
  Serial.print("MQTT...");
  while (!client.connected()) {
    client.setServer(mqtt_host, mqtt_port);
    if(!client.connect(client_id, mqtt_user, mqtt_pwd)) {
      Serial.println("NOPE");
      restart();
    }
  }
  Serial.println("OK!");
}

void wifi_connect() {
  Serial.print("Wifi...");

  WiFi.hostname(client_id);
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_name, wifi_password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    restart();
  }

  Serial.println("OK! (" + WiFi.localIP().toString() + ")");
}

void setup() {
  Serial.begin(9600);

  wifi_connect();
  delay(500);
  mqtt_connect();
}

void publishInt(int value) {
  char str[1];
  String(current_state).toCharArray(str, 2);
  client.publish(vibration_state_topic, str, true);
}

void transition_to(int new_state, unsigned long transition_time) {
  int old_state = current_state;
  current_state = new_state;
  current_state_at = transition_time;
  publishInt(current_state);
  Serial.println(String(old_state) + " -> " + String(new_state) + " (at " + String(transition_time) + ")");
}

void sample_loop() {
  int sample = digitalRead(VIBRATION_SENSOR_PIN);
  last_sample_at = millis();

  samples[sample_index] = sample;

  int sum = 0;
  if(DEBUG) Serial.println("-----------");
  for(int i = 0; i < N_SAMPLES; i++) {
    sum += samples[i];
    if(DEBUG) Serial.print(samples[i]);
  }
  if(DEBUG) Serial.println();
  if(DEBUG) Serial.println("-----------");

  if(current_state == ACTIVE && sum == 0) {
    // set to inactive if there are no vibrations
    // for N_SAMPLES * SAMPLE_INTERVAL milliseconds
    transition_to(INACTIVE, last_sample_at);
  } else if(current_state == INACTIVE && sum > N_SAMPLES / 2.0) {
    // Set to active if there are "enough" vibrations.
    // Here I used "half of the samples" just as an euristic
    // to determine that a consistent vibration activity has started
    transition_to(ACTIVE, last_sample_at);
  }

  sample_index = (sample_index + 1) % N_SAMPLES;
}
void loop() {
  if(!client.connected()) {
    mqtt_connect();
  }

  // sampling this way instead of setting a delay
  // so we can potentially do something else
  // while we "wait"
  if(millis() - last_sample_at > SAMPLE_INTERVAL) {
    sample_loop();
  }
  client.loop();
}
