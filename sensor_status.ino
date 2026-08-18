// sensor_status.ino
// ESP32 (ESP32-WROOM-32UE) analog sensor status monitor.
// Samples a buffered sensor about 10x/s, drives the status LED by a
// threshold, and lets a push-button toggle between two thresholds.

// Hardware from the Part A schematic:
//   Sensor:  SIG_IN --10k-- node --18k-- GND, node buffered by the
//            MCP6001 voltage follower, then filtered (16k + 100n) to ADC.
//   ADC pin: GPIO36 (SENSOR_VP / ADC1_CH0).
//   LED:     GPIO19, active-high through 150 ohm.
//   Button:  GPIO0 (BOOT), active-low with internal pull-up.
// Change the defines below to match a different board.

#define SENSOR_PIN  36
#define LED_PIN     19
#define BTN_PIN      0

#define ADC_BITS    10     // 10-bit ADC: 0..1023
#define ADC_STEPS   1023.0F
#define VREF        3.3F   // reference voltage

#define LOOP_MS     100    // ~10 samples per second
#define DEBOUNCE_MS 20
#define BTN_PRESSED LOW

#define THR_LO      1.0F
#define THR_HI      2.0F

// Part A divider: node = sensor * 18k/(10k+18k), so sensor = node * 14/9.
const float kDividerGain = (10.0F + 18.0F) / 18.0F;

float threshold = THR_LO;
bool  ledOn = false;
int   stable = HIGH;       // last raw button level
bool  debounced = HIGH;    // steady, accepted button level
unsigned long lastEdge = 0;
unsigned long lastSample = 0;

float readSensorVolts() {
  int raw = analogRead(SENSOR_PIN);           // 0..1023
  float nodeVolts = raw * VREF / ADC_STEPS;   // volts at the ADC node
  return nodeVolts * kDividerGain;            // volts at the sensor, 0-5V
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(ADC_BITS);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  unsigned long now = millis();

  // Non-blocking debounce: the pin must hold one level for DEBOUNCE_MS
  // before it counts. Each clean press flips the active threshold.
  int reading = digitalRead(BTN_PIN);
  if (reading != stable) {
    stable = reading;
    lastEdge = now;
  }
  if (reading != debounced && (now - lastEdge) >= DEBOUNCE_MS) {
    debounced = reading;
    if (debounced == BTN_PRESSED) {
      threshold = (threshold == THR_LO) ? THR_HI : THR_LO;
    }
  }

  // Sample on a fixed timer. The loop never blocks, so the button stays
  // responsive while the ADC and serial work.
  if (now - lastSample >= LOOP_MS) {
    lastSample = now;
    float v = readSensorVolts();
    ledOn = v > threshold;
    digitalWrite(LED_PIN, ledOn ? HIGH : LOW);

    Serial.print("t=");
    Serial.print(now);
    Serial.print(" v=");
    Serial.print(v, 2);
    Serial.print("V state=");
    Serial.print(ledOn ? "ON" : "OFF");
    Serial.print(" thr=");
    Serial.print(threshold, 1);
    Serial.println("V");
  }
}
