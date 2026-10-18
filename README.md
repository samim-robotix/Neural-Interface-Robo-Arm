# BCI Robotic Arm Visual Stimulator & Neural Interface System

![Project Banner](assets/banner.png)

[![Platform](https://img.shields.io/badge/Platform-ESP32-E7352C?logo=espressif&logoColor=white)](https://www.espressif.com/)
[![Robotic Arm](https://img.shields.io/badge/Robotic%20Arm-Kinova%20JACO%206--DOF-00529B?logo=robotics&logoColor=white)](https://www.kinovarobotics.com/)
[![Control Bridge](https://img.shields.io/badge/Host%20Bridge-C%23%20%2F%20.NET%208.0-512BD4?logo=dotnet&logoColor=white)](https://dotnet.microsoft.com/)
[![Framework](https://img.shields.io/badge/Framework-Arduino%20%2F%20PlatformIO-00979D?logo=arduino&logoColor=white)](https://platformio.org/)
[![Hardware](https://img.shields.io/badge/Actuator-WS2812B%20NeoPixels-FF6F00)](https://www.adafruit.com/category/168)
[![Protocol](https://img.shields.io/badge/Protocol-UART%20115200%20%7C%20WiFi%20SoftAP%20%7C%20USB%202.0-008080)](https://en.wikipedia.org/wiki/Universal_asynchronous_receiver-transmitter)
[![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

---

## 📌 Project Overview

The **BCI Robotic Arm Visual Stimulator & Neural Interface System** is a closed-loop cyber-physical architecture connecting human electroencephalography (EEG) brain signals to a physical **Kinova JACO (Gen2) 6-DOF Assistive Robotic Manipulator**.

Engineered on the **Espressif ESP32** micro-architecture and **Kinova Gen2 Robotics API**, this system delivers strictly timed, microsecond-accurate optical stimuli to evoke **Steady-State Visual Evoked Potentials (SSVEP)** and **P300 Event-Related Potentials**. Decoded neural target intents are ingested by a host **C# / .NET 8** actuation bridge, which streams real-time Cartesian velocity vectors and joint trajectories to the Kinova robotic arm to execute spatial reaching, grasping, and placement tasks.

### 🌟 Why This Matters & Its Real-World Impact
- **Assistive Independence**: Empowers individuals with severe neuromuscular disorders (e.g., ALS, spinal cord injuries, quadriplegia) with a hands-free, motor-free pathway to interact with their physical environment.
- **Sub-Millisecond Timing Stability**: Eliminates the 60 Hz display refresh rate jitter common to monitor-based visual stimuli by driving physical high-speed WS2812B LEDs directly via FreeRTOS microsecond timers.
- **Closed-Loop Actuation & Diagnostics**: Combines high-frequency optical stimulation with bidirectional Kinova telemetry—monitoring live 6-axis joint torque ($N$), current ($A$), and power bus diagnostics ($V, W$) during spatial actuation.

---

## 🏗️ System Architecture & Dataflow Pipeline

```
+-----------------------------------------------------------------------------------------+
|                                1. VISUAL STIMULATION LAYER                              |
|                                                                                         |
|   +-----------------------+     GPIO 4 (800 kHz)     +------------------------------+   |
|   |  ESP32 Microcontroller| -----------------------> | WS2812B NeoPixel Stimulators |   |
|   |  (LX6 Dual-Core 240M) |                          |  - 6-Station Ring Array (72) |   |
|   +-----------+-----------+                          |  - 3x3 Matrix Grid (9)       |   |
+---------------|-----------------------------------------------------|-------------------+
                |                                                     | Optical Gaze
                | UART (115200) / Wi-Fi SoftAP                        | Stimulus
                v                                                     v
+-------------------------------+                     +-------------------------------+
|      HOST PC & TELEMETRY      |                     |       HUMAN USER & EEG        |
|  - Mode & Frequency Control   |                     |  - Visual Evoked Potentials   |
|  - Real-Time Parameter Sync   |                     |    (SSVEP / P300 Response)    |
|  - EEPROM Non-Volatile Memory |                     |  - Multi-Channel EEG Cap      |
+---------------+---------------+                     +---------------+---------------+
                |                                                     |
                | Parameter Calibration                               | Microvolt Neural
                |                                                     | Signals
                v                                                     v
+-----------------------------------------------------------------------------------------+
|                                 2. NEURAL PROCESSING ENGINE                             |
|                                                                                         |
|       Raw EEG Telemetry  ===>  Bandpass Filtering (4-40 Hz) / Notch (50/60 Hz)          |
|                                         ||                                              |
|                                         v                                               |
|           Spatial Filtering & Feature Extraction (CCA / FBCCA / Wavelet Transform)      |
|                                         ||                                              |
|                                         v                                               |
|        Classification Engine (Deep 1D-CNN / Transformer / LSTM / Linear Discriminant)   |
+-----------------------------------------------------------------------------------------+
                                          |
                                          | Decoded Spatial Intent (Target 0 to 5)
                                          v
+-----------------------------------------------------------------------------------------+
|                         3. KINOVA JACO ROBOTIC ACTUATION LAYER                          |
|                                                                                         |
|   +------------------------------------+      USB 2.0 (Kinova API)                      |
|   | Host C# / .NET 8 Actuation Bridge  | --------------------------------+              |
|   | - 50 Hz Cartesian Velocity Engine  |                                 |              |
|   | - 6-Axis Dynamic Telemetry Logger  |                                 v              |
|   +------------------------------------+              +-----------------------------+   |
|                                                       | Kinova JACO 6-DOF Manipulator|   |
|   +------------------------------------+              | - Integrated DSP Controller |   |
|   | Real-Time Feedback Stream          | <----------- | - 6-Axis Current/Torque (N) |   |
|   | - Supply V, Amps, Power (W), Force |              | - Assistive Arm + Gripper   |   |
|   +------------------------------------+              +-----------------------------+   |
+-----------------------------------------------------------------------------------------+
```

---

## ⚡ Core Technical Features

### 💡 Embedded Optical Stimulator (ESP32 Firmware)
- **Microsecond-Accurate Pulse Engine**: Non-blocking FreeRTOS state machine via `millis()` ensuring precise duty cycles across frequencies ranging from **2.0 Hz to 20.0 Hz** (and up to **60 Hz** in Wi-Fi mode).
- **Dual Firmware Operating Paradigms**:
  1. **Spatial Multi-Station Firmware (`bci_led -3`)**: Controls **72 WS2812B LEDs** distributed across **6 distinct spatial target places** (12 LEDs per place). Supports sequential place flashing (Mode 1) and grouped column/row flashing (Mode 2) with automated rest cycles.
  2. **Wireless Web-Controlled Matrix Firmware (`final code.txt`)**: Hosts a **Wi-Fi SoftAP** (`BCI_Stimulus_AP`) and serves a dark-themed responsive dashboard via `ESPAsyncWebServer` for dynamic tuning of frequency, duty cycle, intervals, and RGB colors.
- **Robust UART Command Interface**: Supports full real-time bidirectional parameter orchestration over Serial at 115200 Baud (`FETCH`, `STARTM1`, `STARTM2`, `STOP`, `M1SEQ`, `M1`, `M2`).
- **Non-Volatile Parameter Persistence**: Saves calibrated frequencies, durations, and RGB color profiles directly to ESP32 **EEPROM** (512 bytes) validated with a magic signature (`0xAABBCCDD`), surviving unexpected power cycles.
- **Visual Boot Diagnostics**: Built-in `showFlowingRadiant()` optical startup sequence verifies electrical continuity and LED channel health across all connected pixels before arming the stimulus.

### 🦾 Kinova JACO 6-DOF Robotic Actuation & Telemetry (`Jaco/`)
- **Real-Time 6-Axis Dynamics Streaming**: High-speed C# client (`jaco_test`) reading live per-joint current draw ($A$) and joint torque/force ($N$) across all 6 axes (`Joint #1` to `Joint #6`).
- **Closed-Loop Cartesian Velocity Control**: 50 Hz (20 ms) update loop commanding smooth 3D linear translations ($X, Y, Z$) and wrist rotations via `CJoystickValue`.
- **System Health & Bus Diagnostics**: Live monitoring of DSP firmware revisions, bus supply voltage ($V$), power consumption ($W$), and safety interlocks via `Kinova.API.Jaco.dll`.
- **Target Station Navigation**: Automated mapping of BCI target selections to stored spatial waypoints for pick-and-place manipulation routines.

---

## 🔌 Bill of Materials (BOM) & Hardware Connections

### Bill of Materials
| Component | Specification / Model | Quantity | Purpose |
| :--- | :--- | :---: | :--- |
| **Robotic Arm** | Kinova JACO Gen2 (6-DOF + Gripper) | 1 | Physical manipulation and target reaching |
| **Arm Power & Interface** | Kinova 24V DC Regulated Supply + USB 2.0 Cable | 1 | Powering arm actuators and host communication |
| **Microcontroller** | ESP32-WROOM-32 / NodeMCU ESP-32S | 1 | Real-time stimulus timing, Serial parsing, Wi-Fi AP |
| **Addressable LEDs** | WS2812B NeoPixel (Ring / Strip / Matrix) | 9 to 72 | Optical flicker targets for SSVEP/P300 elicitation |
| **LED Power Supply** | 5V DC @ 2A – 3A Regulated | 1 | Independent power for high-draw NeoPixel arrays |
| **Protection Circuitry** | 1000 µF capacitor + 330Ω resistor | 1 Set | Decoupling & data line ringing protection |
| **Host Interface** | Micro-USB to USB-A Cable | 1 | 115200 Baud serial communication & firmware flashing |

### Pinout Mapping (ESP32)
| ESP32 Pin | Connected Component | Function / Signal | Notes |
| :--- | :--- | :--- | :--- |
| **GPIO 4** | NeoPixel Strip / Matrix DIN | Data In (800 kHz signal) | Main PWM/RMT data transmission line |
| **GND** | WS2812B GND & Supply GND | Common Ground | Essential to tie all system grounds together |
| **5V / VIN** | External 5V Power Supply (+5V) | System Power | Feed 5V externally for arrays > 12 LEDs |
| **GPIO 1 (TX0)** | Host PC / USB-UART | Serial Transmit | 115200 Baud telemetry & status feedback |
| **GPIO 3 (RX0)** | Host PC / USB-UART | Serial Receive | 115200 Baud command ingestion |

---

## 💻 Repository Directory Structure

```plaintext
BCI Robo arm/
├── assets/
│   ├── banner.png                  # High-tech repository banner
│   ├── photos/                     # Physical hardware and setup photos (.gitkeep)
│   └── videos/                     # Demo video clips and animations (.gitkeep)
├── bci_led -3/                     # Primary PlatformIO Project (72 LEDs, 6 Stations, Serial + EEPROM)
│   └── bci_led -3/
│       ├── platformio.ini          # PlatformIO environment config for esp32dev
│       └── src/
│           └── main.cpp            # 72-LED SSVEP stimulus state machine with EEPROM & UART parser
├── bci_led -2/                     # Incremental staging build for LED array
├── bci_led/                        # Baseline PlatformIO firmware build
├── Jaco/                           # Kinova JACO Gen2 Robotics Control Suite
│   ├── test_jaco/                  # Actuation bridge & telemetry source code
│   │   ├── jaco_test/              # .NET 8 C# console telemetry & diagnostics client
│   │   │   ├── Program.cs          # 6-axis joint torque/current & bus voltage monitor
│   │   │   └── jaco_test.csproj    # .NET 8 project referencing Kinova API DLLs
│   │   ├── CSharp/                 # WPF joystick emulation & Cartesian velocity controller
│   │   └── CPlusPlus/              # Native C++/CLI API reference implementation
│   ├── PS 0000 0009_1.5.1/         # Kinova Gen2 SDK v1.5.1 installers (Windows & Ubuntu)
│   └── jaco/                       # Release archive (Jacosoft 3.2, DSP firmware, API docs)
├── New folder/                     # Curated academic research literature on BCI & Motor Imagery
│   ├── BrainComputer_InterfaceA_Brain-in-the-Loop_Communication_System.pdf
│   ├── MotorImageryClassificationUsingConvolutionalNeuralNetworks.pdf
│   └── ... (12 specialized peer-reviewed papers)
├── final code.txt                  # Standalone ESP32 Wi-Fi AP + Async Web Server stimulus controller (C++)
├── .gitignore                      # Configured to exclude build artifacts, large binaries, and secrets
└── README.md                       # Complete technical documentation
```

---

## 🛠️ Installation & Setup Guide

### 1. Embedded Firmware Deployment (PlatformIO)

The recommended setup runs via **VS Code** with the **PlatformIO IDE** extension.

1. Connect the ESP32 board to your computer via micro-USB.
2. Open the directory `bci_led -3/bci_led -3/` in PlatformIO or VS Code.
3. Verify your `platformio.ini`:
   ```ini
   [env:esp32dev]
   platform = espressif32
   board = esp32dev
   framework = arduino
   lib_deps = adafruit/Adafruit NeoPixel@^1.15.2
   ```
4. Build and flash the firmware:
   ```bash
   pio run --target upload
   ```
5. Open the serial monitor at **115200 baud**:
   ```bash
   pio device monitor -b 115200
   ```

### 2. Standalone Web Controller Setup (`final code.txt`)

If using the Wi-Fi AP 3x3 matrix controller:
1. Open the Arduino IDE.
2. Ensure you have installed the required libraries:
   - `Adafruit_NeoPixel`
   - `ESPAsyncWebServer`
   - `AsyncTCP`
3. Load the code from `final code.txt`, select board **ESP32 Dev Module**, and click **Upload**.
4. Connect to the Wi-Fi Access Point:
   - **SSID**: `BCI_Stimulus_AP`
   - **Password**: `bci-password`
5. Open your browser and navigate to `http://192.168.4.1` to access the interactive real-time stimulus dashboard.

### 3. Kinova JACO Robotic Arm Setup (`Jaco/`)

#### A. Driver & SDK Installation
- **Windows**: Navigate to `Jaco/PS 0000 0009_1.5.1/PS 0000 0009_1.5.1/Windows/` and run `vc_redist_x86.exe` followed by `Kinova-Jaco-Usb-Driver-(usblib)-Installer.exe` and `SDK_GEN2_1_5_1_x86.exe`.
- **Ubuntu Linux**: Navigate to `Jaco/PS 0000 0009_1.5.1/PS 0000 0009_1.5.1/Ubuntu/16_04/64 bits/` and execute:
  ```bash
  sudo ./installSDK64.sh
  ```

#### B. Launching the Telemetry & Control Bridge (.NET 8)
1. Power on the Kinova JACO arm with its 24V DC supply and connect via USB 2.0.
2. Navigate to the client directory and build the project:
   ```bash
   cd "Jaco/test_jaco/jaco_test"
   dotnet build
   dotnet run
   ```
3. The client will initialize `CJacoArm`, establish a hardware handshake, and stream live diagnostics, per-joint currents ($A$), and joint forces ($N$).

---

## 📡 Serial Telemetry Command Reference (`bci_led -3`)

When connected over UART at **115200 Baud**, the firmware parses newline-terminated (`\n` or `\r`) commands:

| Command | Syntax / Parameters | Description |
| :--- | :--- | :--- |
| `FETCH` | `FETCH` | Returns current Mode 1 & 2 frequencies, durations, brightness, colors, and sequence |
| `STARTM1` | `STARTM1` | Starts Mode 1 sequential flickering across the 6 targets |
| `STARTM2` | `STARTM2` | Initiates Mode 2 with a 90-second countdown delay |
| `STOP` | `STOP` | Immediately clears all LEDs and halts the active stimulus engine |
| `M1SEQ` | `M1SEQ,p0,p1,p2,p3,p4,p5` | Sets the sequential order of the 6 targets (e.g., `M1SEQ,0,1,2,3,4,5`) |
| `M1` | `M1,f0,f1,f2,f3,f4,f5,flickDur,pauseDur,bright,color` | Configures Mode 1 (e.g., `M1,5.0,7.5,10.0,12.0,14.0,15.0,3,2,150,RED`) |
| `M2` | `M2,f0,f1,f2,f3,f4,flickDur,pauseDur,bright,color` | Configures Mode 2 parameters (columns & rows) |

---

## 🎥 Media & Hardware Demonstration

<div align="center">
  <video src="https://github.com/user-attachments/assets/cc6c5d3d-c2b4-4c92-b100-55a25d3a45ce" width="100%" controls="controls">
    Your browser does not support the video tag.
  </video>
  <p><em>Demonstration: BCI Visual Stimulation & Closed-Loop Kinova JACO Robotic Arm Target Acquisition</em></p>
</div>

---

## 🔬 Scientific Foundations & Literature Reference

This project builds upon published research in motor imagery, wavelet-based feature extraction, and convolutional neural architectures for EEG classification. The repository includes relevant reference literature in [`New folder/`](New%20folder/):
- **Motor Imagery & Deep Learning**: CNN-Transformer-LSTM multi-channel EEG signal classification.
- **Wavelet Scattering Transforms**: Robust time-frequency representations of non-stationary EEG microvolt responses.
- **Brain-in-the-Loop Robotics**: Closed-loop sensory feedback and target selection paradigms for assistive robotic manipulators.

---

## 📜 License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.
