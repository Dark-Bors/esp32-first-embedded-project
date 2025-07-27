
## 📊 CLI Command vs Transport Matrix

| Command               | USB (Dev Serial) | BLE (Mobile/Field) | Wi-Fi (Remote/API) | Notes                                         |
| --------------------- | ---------------- | ------------------ | ------------------ | --------------------------------------------- |
| `fsm.state`           | ✅                | ✅                  | ✅                  | Safe read-only                                |
| `fsm.set <STATE>`     | ✅                | 🔒                 | 🚫                 | Only trusted users should change system state |
| `fsm.trigger <EVENT>` | ✅                | 🚫                 | 🚫                 | Dev/debug only                                |
| `led.on`              | ✅                | 🚫                 | 🚫                 | Static override, dev-only                     |
| `led.off`             | ✅                | 🚫                 | 🚫                 | Same                                          |
| `led.pattern <MODE>`  | ✅                | ✅                  | 🚫                 | BLE techs can test LEDs                       |
| `led.burst <f d>`     | ✅                | 🚫                 | 🚫                 | Custom burst for testing                      |
| `led.debug`           | ✅                | 🚫                 | 🚫                 | Dumps debug info via UART                     |
| `sec.level`           | ✅                | ✅                  | 🚫                 | Check security status                         |
| `sec.unlock <key>`    | ✅                | 🔒                 | 🚫                 | BLE technician unlock                         |
| `mem.map`             | ✅                | 🚫                 | 🚫                 | Show file→block mapping                       |
| `mem.read <blk>`      | ✅                | 🚫                 | 🚫                 | Low-level, USB only                           |
| `mem.write <blk>`     | ✅                | 🚫                 | 🚫                 | High risk, dev only                           |
| `mem.dump s n`        | ✅                | 🚫                 | 🚫                 | Dump sequential blocks                        |
| `rtv.start usb`       | ✅                | 🚫                 | 🚫                 | Local session                                 |
| `rtv.start ble`       | 🚫               | ✅                  | 🚫                 | BLE-only field mode                           |
| `rtv.start wifi`      | 🚫               | 🚫                 | ✅                  | Cloud-triggered session                       |
| `rtv.stop`            | ✅                | ✅                  | ✅                  | Always safe                                   |
| `rtv.status`          | ✅                | ✅                  | ✅                  | Read-only                                     |
| `cli.version`         | ✅                | ✅                  | ✅                  | Always safe                                   |
| `cli.help`            | ✅                | ✅                  | ✅                  | Public help                                   |
| `cli.bdds.start`      | ✅🐣              | 🚫                 | 🚫                 | Hidden disco mode 💃                          |
| `cli.reset`           | ✅                | 🔒                 | 🚫                 | Soft reboot — secure only                     |
| `cli.tick`            | ✅                | 🚫                 | 🚫                 | Dev heartbeat/test                            |
| `dbg.state <n>`       | ✅                | 🚫                 | 🚫                 | Developer override                            |
| `dbg.event <n>`       | ✅                | 🚫                 | 🚫                 | Developer event injection                     |

---
Each row is a command, and the transport columns show where it is allowed.
Legend:

* ✅ = Supported
* 🔒 = Could be supported, but behind a security check
* 🚫 = Not allowed or unsafe for that transport
* 🐣 = Fun or dev-only (hidden or easter egg)

---

## 📦 Transport Summary

| Transport            | Commands Allowed                                               |
| -------------------- | -------------------------------------------------------------- |
| **USB Serial (Dev)** | ✅ All commands, including debug, memory, and FSM override      |
| **BLE**              | 🔒 Limited safe commands: FSM status, LED pattern, RTV, unlock |
| **Wi-Fi**            | ✅ Read-only telemetry + RTV start/stop (auth-only)             |
