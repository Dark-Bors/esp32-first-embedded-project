### Firmware Design Specification - Security Protocol

---

#### 4.7 Security Design & Access Control

This section outlines the firmware's security protocol, which is a layered system of physical and logical access controls to protect the device from unauthorized operations.

##### 4.7.1 Security Levels & Privileges

The system operates at one of four security levels, determined by a combination of physical GPIO inputs and a logical "Magic Key" command. Each level grants a specific set of privileges.

**Security Levels Table**

| Level | Description | Example Capabilities | GPIO Pattern (18, 19, 21) |
|---|---|---|---|
| 0 | 🚫 **No access** | CLI disabled, error-only logging. All commands refused. | `000` |
| 1 | 🛠 **Limited access** | CLI status queries, limited configuration view. | `001` |
| 2 | ⚙️ **Advanced access** | Read/set full YAML configuration, inspect detailed logs, manually trigger RTV mode. | `011` |
| 3 | 🔐 **Full access (Dev Mode)** | All capabilities unlocked: change operational state, reset NVS, use debug hooks. | `111` |

##### 4.7.2 Physical Layer Control (GPIOs)

* **GPIO Configuration:** GPIOs 18, 19, and 21 are configured as **inputs with internal pull-down resistors**. A `HIGH` state (1) is achieved by an external physical connection (e.g., jumper, switch). The default `LOW` state (0) corresponds to `Level 0`.
* **Maximum Attainable Level:** The physical GPIO pattern determines the **maximum security level** the device can operate at. This is a hard-coded gate; no software command can override it.
* **Security Level Determination:**
    * The GPIO state is read at **system boot** (`state_machine_init()`) to set the initial `g_security_level` global variable.
    * The GPIOs are **periodically re-read** at runtime. If the physical pattern changes to a *lower* security level, the `g_security_level` is immediately downgraded. An upgrade requires a full system reset. Redundancy checks (e.g., confirming the pattern three times) prevent transient glitches from causing downgrades.

##### 4.7.3 Logical Access Control (CLI Commands)

Access to sensitive CLI commands is managed by the `g_security_level` variable. Any command requesting a privilege higher than the current level will be refused with a logged warning.

**CLI & System Behavior by Security Level**

| Feature | Required Level | Notes |
|---|---|---|
| View system state | ≥ 1 | View current operational mode and basic health metrics. |
| View logs | ≥ 2 | Access to detailed debug and error logs. |
| Trigger RTV manually | ≥ 2 | Initiate `RTV Mode` for diagnostics. |
| Change operational state (e.g., to Untethered/Tethered) | ≥ 3 | Transition between primary sub-modes. |
| Reset NVS | 3 only | Destructive action. |
| Enter `HALTED Mode` from CLI | 3 only | Forcing a safe, non-operational state. |

##### 4.7.4 `Dev Mode` and "Magic Key"

* **`Dev Mode` as Security Level 3:** The `Dev Mode` state in the system's state machine (see Section 2.1) is synonymous with `Security Level 3`.
* **"Magic Key" Authentication:** A plaintext "Magic Key" command is used as a **secondary authentication** mechanism to transition from `Operational Mode` to `Dev Mode`. This transition is only possible if the physical GPIOs are set to `111` (`Level 3`).
* **Firmware Update Security:** A successful data transfer (e.g., firmware update) **will not** automatically transition the device into `Dev Mode`. Upon completion, the system returns to its prior operational state. If `Dev Mode` access is required for post-update validation, it must be explicitly entered via the `Magic Key` command.

##### 4.7.5 `RTV Mode` (Recovery/Test/Validation)

* `RTV Mode` is a temporary state for diagnostics and recovery.
* It can be entered automatically on `Timeout` (if defined), or manually via the `CLI: rtv_on` command (requires `Level ≥ 2`).
* The security level in `RTV Mode` is dynamic: it inherits the `g_security_level` from the state it was entered from.
* Exiting `RTV Mode` via `CLI: rtv_off` returns the system to `Operational Mode`.