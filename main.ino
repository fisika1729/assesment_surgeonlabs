
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
