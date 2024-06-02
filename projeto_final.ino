#include <ESP32Servo.h>
#include <EspMQTTClient.h>
#include <ArduinoJson.h>

#define TRIG_PIN 12
#define ECHO_PIN 14
#define servo 32
#define led_vermelho 25
#define led_verde 26
#define buzzer 13

float duracao_pulso, distancia_cm;
char dist[100];

EspMQTTClient client(
  "Nome da rede Wi-fi",
  "Senha do Wi-fi",
  "mqtt.tago.io",
  "Default",
  "token",
  "identificação do esp",
  1883
);

void onConnectionEstablished() {
  Serial.println("Conectado");

  static Servo servo_motor;
  static int angulo_motor = 0;

  client.subscribe("info/servo_ativacao", [](const String &payload) {
    if (payload == "L") {
      Serial.println("LIGAR SERVO MOTOR");
      servo_motor.attach(servo);
      for (angulo_motor = 0; angulo_motor < 180; angulo_motor++) {
        servo_motor.write(angulo_motor);
        delay(20);
      }
    } else if (payload == "D") {
      Serial.println("DESLIGAR SERVO MOTOR");
      servo_motor.attach(servo);
      servo_motor.write(0);
      delay(20);
      
    }
  });

}

void enviar_dados_iot() {
  StaticJsonDocument<300> dadosD;

  dadosD["variable"] = "dado_dist";
  dadosD["value"] = distancia_cm;

  serializeJson(dadosD, dist);

  client.publish("info/dist", dist);
}

void medicao_sensor(){
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duracao_pulso = pulseIn(ECHO_PIN, HIGH);
  distancia_cm = 0.017 * duracao_pulso;
  Serial.print("Distância: ");
  Serial.print(distancia_cm);
  Serial.println(" cm");

  if (distancia_cm <= 30) {
    digitalWrite(led_vermelho, HIGH);
    digitalWrite(led_verde, LOW);
    digitalWrite(buzzer, HIGH);
  } else {
    digitalWrite(led_verde, HIGH);
    digitalWrite(led_vermelho, LOW);
    digitalWrite(buzzer, LOW);
  }
  delay(500);
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(led_verde, OUTPUT);
  pinMode(led_vermelho, OUTPUT);
  pinMode(buzzer, OUTPUT);
  digitalWrite(led_verde, LOW);
  digitalWrite(led_vermelho, LOW);

}

void loop() {
  client.loop();
  onConnectionEstablished();
  medicao_sensor();
  enviar_dados_iot();
}
