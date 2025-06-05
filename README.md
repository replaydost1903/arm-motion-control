## 🛠️ System Overview

The objective of this system is to control a mechanical arm connected to a DC motor based on movement data received from a motion sensor. Using a custom-designed PC interface, users can send commands to define:
- The number of rotations to the left or right,
- And the time required for one full rotation.

The system supports **two operating modes**:
- **Manual Mode**
- **Automatic Mode**  
Details of each mode will be described later.

---
![Screenshot 2025-06-05 230225](https://github.com/user-attachments/assets/df4d8526-10e1-4815-b694-7d06bbf6fa67)
![1](https://github.com/user-attachments/assets/fe6a6db0-8602-4472-aef8-aac8d04746ed)


## 💻 User Interface

- Developed using **Visual Studio** with **C#**.
- Provides control over:
  - The motor (direction, speed, mode),
  - Other hardware components: **LED**, **buzzer**, **motion sensor**, and **speaker**.
![1](https://github.com/user-attachments/assets/9a177e52-f08c-4d13-a210-227498579515)

---

## 🔌 Communication

- **Physical Layer**: RS485 (wired)
- **Protocol**: UART
- Ensures reliable communication between the PC interface and the motor control system.
![1](https://github.com/user-attachments/assets/a2d6a33e-237c-4316-a847-0b6f277e0684)

---

## 🔄 Power Loss Recovery

In the event of a power failure (e.g., system shutdown or power interruption), the system typically requires manual reinitialization via the PC interface.  
To eliminate this dependency:

- All commands sent from the interface are stored in the **internal flash memory** of the microcontroller.
- Upon power restoration, the system automatically reads the stored commands and **resumes operation from the last known state**.

---

## 🔋 Power Supply

- The system operates with a **12V DC power source**.
- A **12V 6A AC-DC converter** supplies power to the system components.  
  _(See the diagram below for the connection schematic.)_

---

