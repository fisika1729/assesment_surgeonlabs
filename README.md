# Surgeons Lab - Hardware Test Engineer Assessment

Firmware for the Part A board: an ESP32-WROOM-32UE that reads a buffered
analog sensor, drives a status LED by a voltage threshold, and toggles the
threshold with a push-button.

## Files

| File | Purpose |
| ---- | ------- |
| `main.ino` | The firmware sketch. |
| `reasoning.pdf` | Part A component selection and design notes. |
| `Schematic_surgeons-lab_2026-08-19.pdf` | Board schematic. |

## Hardware

- ADC pin: GPIO36 (SENSOR_VP, ADC1_CH0).
- Sensor divider: `SIG_IN --10k-- node --18k-- GND`, buffered by the MCP6001
  voltage follower, filtered (16k + 100n) into the ADC.
- Status LED: GPIO19, active-high through 150 ohm.
- Button: GPIO0 (BOOT), active-low with internal pull-up.

## Behavior

- Samples the sensor ~10 times per second on a `millis()` timer.
- Converts the raw 10-bit reading (0-1023, 3.3 V reference) to volts.
- Reconstructs the 0-5 V sensor voltage with the divider gain of 14/9.
- Lights the LED when the voltage sits above the active threshold.
- Each clean button press flips the threshold between 1.0 V and 2.0 V.
- Prints one line per sample: `t=1234 v=1.65V state=ON thr=1.0V`.

## Build and flash

Open `main.ino` in the Arduino IDE with the `esp32` core installed, set the
board to ESP32 Dev Module, and upload. Open the serial monitor at 115200 baud.

## Assumptions

- The ADC node sits behind the 10k/18k divider, so sensor volts equal node
  volts times 14/9. Change `kDividerGain` for a different ratio.
- The LED is active-high. Invert the `digitalWrite` in `loop()` if the LED
  hangs between VCC and the pin.
- The button is active-low. Set `BTN_PRESSED` to `HIGH` for an active-high
  button.
- The prompt specifies a 10-bit ADC and 3.3 V reference. The sketch sets the
  resolution to 10 bits with `analogReadResolution`.

## Debouncing

The button is read every loop iteration. A new level only counts after the
pin holds it for 20 ms (`DEBOUNCE_MS`). The press fires once on the accepted
edge, so a single click toggles the threshold. No blocking delays, so the
button stays responsive while the ADC and serial run.
