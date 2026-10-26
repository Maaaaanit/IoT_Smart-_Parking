# SmartPark — VS Code starter

This recreates the supplied SmartPark admin GUI and provides an ESP32 PlatformIO project. The frontend currently uses demo data/local interactions; it is not yet connected to a real backend. The backend folder includes a minimal API starter. Keep your existing production backend and merge the two manual routes from `hardware-guide.html` into it rather than replacing it with this starter.

## Requirements
- VS Code
- Node.js LTS
- VS Code extensions: **PlatformIO IDE**, **ESLint** (optional)
- ESP32 DevKit V1, 4x4 keypad, 20x4 I2C LCD, SG90 servo, buzzer, two LEDs + 220Ω resistors

## 1. Run GUI
Open `frontend` in terminal:
```bash
npm install
npm run dev
```
Open the localhost URL printed by Vite (normally http://localhost:5173).

## 2. ESP32 firmware in VS Code (no Arduino IDE)
Install PlatformIO IDE extension. Open the `firmware` folder as the VS Code project. PlatformIO will install the Espressif32 platform and libraries listed in `platformio.ini`. Set Wi-Fi and backend IP in `src/main.cpp`, connect ESP32 with a data USB cable, choose the detected serial port, then click PlatformIO: Upload. Open PlatformIO Serial Monitor at 115200 baud.

The firmware keeps the pin assignments from `hardware-guide.html`: keypad rows 13,12,14,27; columns 26,25,33,32; LCD SDA 21/SCL 22; servo 18; buzzer 19; green LED 4; red LED 5.

## 3. Wiring notes
- Keypad pins 1–8 must match R1,R2,R3,R4,C1,C2,C3,C4. Verify the keypad's actual connector order before powering.
- LCD: SDA→21, SCL→22, GND→GND, VCC according to your module. Address defaults to `0x27`; some are `0x3F`.
- Servo signal→18, servo VCC→external regulated 5V, servo GND→GND. **Join external supply GND to ESP32 GND.** Do not power a loaded servo from an ESP32 GPIO.
- LED GPIO→220Ω resistor→LED long leg; short leg→GND.
- ESP32 GPIOs are 3.3V logic. Never feed 5V into a GPIO.

## 4. API integration
The ESP32 starter sends JSON to:
- `POST /api/entry/manual-entry` with `{ "plateNumber": "GJ01AB1234", "vehicleType": "CAR" }`
- `POST /api/exit/manual-scan` with `{ "plateNumber": "GJ01AB1234" }`

The firmware expects a JSON response containing `success` and `data.slotId` for entry, or `success` and `data.feeBreakdown.totalAmount` / `data.payment.amount` for exit. The exact existing backend schema may differ; adapt parsing to your real API. It uses a placeholder API key header `x-device-api-key`; match this to your backend's `authenticateDevice` middleware.

## 5. Demo GUI versus real system
The dashboard is a visual prototype modeled on your screenshots. It uses in-memory sample data and browser interactions. To make it live, connect its API calls to your Express server, enable Socket.IO events, and implement authentication/authorization. Don't expose device API keys in the React frontend.
