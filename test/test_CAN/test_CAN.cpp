#include <Arduino.h>
#include <SPI.h>
#include <mcp2515.h>
#include "BluetoothSerial.h"

#define CS_PIN        5
#define CAN_SPEED     CAN_500KBPS
#define CAN_CLOCK     MCP_8MHZ

#define OBD_REQUEST   0x7DF   // IMPORTANT : broadcast
#define RESPONSE_MIN  0x7E8
#define RESPONSE_MAX  0x7EF

#define POLL_INTERVAL 200
#define TIMEOUT       100

MCP2515 mcp2515(CS_PIN);
BluetoothSerial SerialBT;

// ─── Structure PID ─────────────────────────────────────────────
struct PID {
  uint8_t pid;
  const char* name;
  const char* unit;
};

const PID PIDS[] = {
  { 0x0C, "RPM",        "rpm" },
  { 0x0D, "Speed",      "km/h" },
  { 0x05, "Coolant",    "C" },
  { 0x0B, "MAP",        "kPa" },
  { 0x0F, "IAT",        "C" },
};

const uint8_t PID_COUNT = sizeof(PIDS) / sizeof(PIDS[0]);

// ─── Décodage PID ─────────────────────────────────────────────
float decodePID(uint8_t pid, uint8_t A, uint8_t B) {
  switch (pid) {
    case 0x0C: return ((A * 256.0f) + B) / 4.0f; // RPM
    case 0x0D: return A;                          // Speed
    case 0x05: return A - 40.0f;                  // Coolant
    case 0x0B: return A;                          // MAP
    case 0x0F: return A - 40.0f;                  // IAT
    default:   return -1;
  }
}

// ─── Envoi requête OBD ─────────────────────────────────────────
bool sendPID(uint8_t pid) {
  struct can_frame frame;

  frame.can_id  = OBD_REQUEST;
  frame.can_dlc = 8;

  frame.data[0] = 0x02; // longueur
  frame.data[1] = 0x01; // mode 01
  frame.data[2] = pid;

  for (int i = 3; i < 8; i++) frame.data[i] = 0;

  return (mcp2515.sendMessage(&frame) == MCP2515::ERROR_OK);
}

// ─── Lecture réponse ───────────────────────────────────────────
bool readPID(uint8_t pid, float &value) {
  struct can_frame frame;
  unsigned long start = millis();

  while (millis() - start < TIMEOUT) {
    if (mcp2515.readMessage(&frame) == MCP2515::ERROR_OK) {

      // Vérifie ID réponse ECU
      if (frame.can_id >= RESPONSE_MIN && frame.can_id <= RESPONSE_MAX) {

        if (frame.data[1] == 0x41 && frame.data[2] == pid) {
          uint8_t A = frame.data[3];
          uint8_t B = frame.data[4];

          value = decodePID(pid, A, B);
          return true;
        }
      }
    }
  }
  return false;
}

// ─── Setup ────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  SerialBT.begin("OBD_Mode01");

  mcp2515.reset();
  mcp2515.setBitrate(CAN_SPEED, CAN_CLOCK);

  // PAS DE FILTRE pour debug
  mcp2515.setNormalMode();

  Serial.println("CAN prêt");
}

// ─── Loop ─────────────────────────────────────────────────────
void loop() {
  String json = "{";

  for (uint8_t i = 0; i < PID_COUNT; i++) {
    float val = -1;

    if (sendPID(PIDS[i].pid)) {
      if (readPID(PIDS[i].pid, val)) {
        json += "\"";
        json += PIDS[i].name;
        json += "\":";
        json += String(val, 1);
      } else {
        json += "\"";
        json += PIDS[i].name;
        json += "\":null";
      }
    }

    if (i < PID_COUNT - 1) json += ",";
    delay(10);
  }

  json += "}";

  Serial.println(json);
  SerialBT.println(json);

  delay(POLL_INTERVAL);
}