# 🌟 OptiPulse™State: Developer Training Project

Welcome to **OptiPulse™State**, a hands-on embedded systems learning project focused on mastering **FreeRTOS**, **state machines**, **LED control**, **CLI interaction**, and **real-time camera streaming** — all on the ESP32-S3.

This project mimics a simplified embedded operating state machine, commonly used in real-world firmware systems, with **defined states**, **LED-based feedback**, **timed logic**, and **robust transition handling**.

---

## 📌 Project Purpose

To serve as a **learning platform** for embedded software and firmware design by:

- Practicing state machine implementation
- Applying FreeRTOS and multitasking
- Managing peripheral control (LED, camera, SD card)
- Handling persistent config (NVS and YAML parsing)
- Exploring real-world patterns like CLI tools and recovery logic

---

## ⚙️ Architecture Overview

### 🔁 State Machine Logic

The core of the system is built around a **finite state machine (FSM)** with the following states:

1. **DEV_MODE**  
   - Full CLI access over UART  
   - LED control commands (on, off, blink, duty cycle, frequency, etc.)  
   - Wi-Fi scanning and connection via CLI  
   - Real-time logs printed every ≤1s  
   - Loads configuration from a YAML file  
   - Magic key required to switch states  

2. **OPERATIONAL**  
   - LED behavior driven by config or runtime status  
   - Logs saved to SD card  
   - Real-Time View (RTV) supported with a timer-based session  
   - On command, splits into:
     - **TETHERED**:  
       ↳ USB-connected; allows log transfer to PC, uses slow LED blink  
     - **UNTETHERED**:  
       ↳ Wi-Fi log upload to cloud, with fallback to Tethered if failure occurs  
       ↳ LED reflects network activity  

3. **HALTED**  
   - LED blinks rapidly at 5Hz  
   - System enters deep sleep indefinitely  
   - Can only be resumed by sending a CLI command with the **magic key**  

---

## 🧠 Learning Outcomes

By completing this project, you'll develop:

| Skill / Concept                     | Description                                                                 |
|------------------------------------|-----------------------------------------------------------------------------|
| Embedded State Machines            | Design and manage FSM logic with safe transitions                          |
| FreeRTOS                           | Task creation, delay handling, and multitasking principles                 |
| GPIO Control                       | Blinking LED patterns, pulse width control, and status indication          |
| UART CLI                           | Custom command handler for real-time interaction                           |
| SD Card Integration                | Log writing, retrieval, and structured debug storage                       |
| Camera Streaming                   | Timer-based real-time view with adaptive feedback                          |
| YAML Configuration Parsing         | Load startup parameters, Wi-Fi info, LED behavior, and secure keys         |
| Persistent Storage (NVS)           | Store critical state to resume after resets or crashes                     |

---

## 📐 System Diagram

![alt text](<state machine diagram.png>)

---

## 🔦 LED Pattern Overview

![alt text](<LED pattern timing.png>)

---

## 📂 Project Structure (Simplified)

```
optipulse_state/
│
├── main/
│   ├── main.c               # Entry point (previously blink_led.c)
│   ├── cli_handler.c/h      # UART CLI parser and command dispatcher
│   ├── state_machine.c/h    # FSM transitions, state logic, resume logic
│   ├── led_controller.c/h   # LED patterns and timed control
│   ├── config_loader.c/h    # YAML parsing and loading logic
│   ├── camera_handler.c/h   # Real-time view (RTV) logic with timer
│   ├── storage_manager.c/h  # SD card or NVS storage interactions
│   └── transfer_manager.c/h # Tethered / Untethered log transfer (future)
│
├── sdkconfig                # ESP-IDF config
├── README.md                # This file
└── CMakeLists.txt           # Build instructions
```

---

## 🔧 Bill of Materials (BOM)

| Component                     | Description                        |
|------------------------------|------------------------------------|
| ESP32-S3 WROOM Board         | Main MCU with camera + SD support  |
| Onboard GPIO2 LED            | For visual feedback                |
| MicroSD Card (1GB+)          | For logging                        |
| USB-C Cable (2x)             | One for UART CLI, one for OTG if needed |
| Optional: PC or phone        | For viewing RTV or transferring logs |

---

## 🛠️ Tools & Software Used

| Tool / Lib                   | Purpose                                      |
|-----------------------------|----------------------------------------------|
| [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/) | Core framework for ESP32-S3 development |
| FreeRTOS                    | Multitasking kernel                          |
| YAML Parser (e.g., libyaml) | Reading configuration files                  |
| UART CLI (custom)           | Serial command interface                     |
| VS Code + ESP-IDF Plugin    | Development environment                      |
| Miro / Mermaid / Draw.io    | State machine and architecture diagrams      |
| Git                         | Version control                              |

---

## 🧭 Roadmap / Phases

- [x] Phase 0: LED + Logger (Completed)  
- [x] Phase 1: Add CLI + Config YAML + State Transitions  
- [ ] Phase 2: Add SD Card Logging + Persistent State Save  
- [ ] Phase 3: Add Camera + Real-Time View logic  
- [ ] Phase 4: Implement Tethered / Untethered Flow  
- [ ] Phase 5: Error Recovery + Auto Resume + OTA (future)

---

## 🔐 Security Layer Implementation Concept

### ✅ Security Levels Table

| Level | Description                        | Example Capabilities                               | GPIO Pattern (18, 19, 21) |
|-------|------------------------------------|----------------------------------------------------|---------------------------|
| 0     | 🚫 No access                       | No CLI, no DEV actions, error-only                 | `000`                     |
| 1     | 🛠 Limited access                  | CLI status, limited config queries                 | `001`                     |
| 2     | ⚙️ Advanced access                | Can read/set YAML config, inspect logs             | `110`                     |
| 3     | 🔐 Full access (dev/unlocked mode) | Change state, reset NVS, debug hooks               | `101`                     |

---

### 🧩 Design Decisions Recap

1. **Security Logic Source**
   - Physical-layer check via 3 input GPIOs at startup (and maybe periodically).
   - Logic stored in:
     ```c
     int get_security_level_from_gpio(void);
     ```

2. **Integration Point**
   - Call at system boot (`state_machine_init()`) or CLI entry.
   - Store in a global `security_level` variable.
   - Guard sensitive CLI commands:  
     ```c
     if (security_level >= 2) { /* allow access */ }
     ```

3. **Magic Key Logic (Temporary)**
   - Currently uses plaintext CLI key:  
     `MAGIC_KEY = "unlocked_dev_123"`
   - Future upgrade: AES-256 + CRC + ESP Secure Boot
   - TODO: delete tag to clean it from production

4. **CLI & System Behavior by Security Level**

| Feature                 | Required Level |
|------------------------|----------------|
| View system state      | ≥ 1            |
| View logs              | ≥ 2            |
| Trigger RTV manually   | ≥ 2            |
| Change operational state | ≥ 3          |
| Reset NVS              | 3 only         |
| Enter HALTED from CLI  | 3 only         |

5. **Fail-Safe Handling**
   - If GPIO pattern is invalid → fallback to `SEC_LEVEL_0`
   - In `SEC_LEVEL_0`, most actions are refused and warning is logged


---

## ✅ Final Notes

This project blends **professional firmware architecture** with **educational design principles**, making it perfect for anyone serious about embedded systems development and system-level design patterns.

---

> © 2025 Dark Bors — Learning by doing, one LED pulse at a time.
