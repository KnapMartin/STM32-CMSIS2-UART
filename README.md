# STM32 CMSIS-RTOS2 UART Driver

This repository provides a modular, thread-safe UART driver for STM32 microcontrollers using CMSIS-RTOS2 (e.g., FreeRTOS with CMSIS API).  
It enables robust UART communication in embedded applications with interrupt-driven transmit and receive.

---

## Features

- **Thread-safe UART transmit/receive**
- **CMSIS-RTOS2 integration** (uses mutex, semaphore, and message queue)
- **Interrupt-driven** for efficient, non-blocking operation
- **Easy integration** with STM32 HAL

---

## Getting Started

### 1. UART Driver Setup

```cpp
#include "uart.h"

// Create UART handle and RTOS objects
UART_HandleTypeDef huart2;
osSemaphoreId_t semTx;
osMessageQueueId_t queueRx;
osMutexId_t mutexTx;

// Instantiate UART driver
Uart uart(&huart2);

// Set RTOS handles
uart.setHandleSemTx(&semTx);
uart.setHandleQueueRx(&queueRx);
uart.setHandleMutexTx(&mutexTx);

// Initialize UART
if (uart.init() == IUart::Status::OK) {
    // UART is ready to use
}
```
NOTE: A queue should be of sufficient size and should hold *uint8_t* type.
---

### 2. Transmitting Data

```cpp
const char* msg = "Hello, UART!\r\n";
uart.transmit(msg, strlen(msg));
```

---

### 3. Receiving Data

```cpp
char buffer[64];
if (uart.receive(buffer) == IUart::Status::OK) {
    // buffer now contains the received line (ending with \r or \n)
}
```

---

### 4. Using in Interrupts

Call these from your HAL UART IRQ handlers:

```cpp
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    uart.handleRxInterrupt(huart);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    uart.handleTxInterrupt(huart);
}
```

---

## Example

```cpp
// Transmit a message
uart.transmit("STM32 UART Ready\r\n", 18);

// Receive a line (blocking)
char rxLine[64];
if (uart.receive(rxLine) == IUart::Status::OK) {
    // Process rxLine
}
```

---

## License

MIT License

---

## Credits

- STM32 HAL & CMSIS-RTOS2
- Designed for robust embedded UART communication

---
