#include "uart.h"

Uart::Uart(UART_HandleTypeDef *huart)
    : m_rxChar{}
    , m_huart(huart)
    , m_semTx(nullptr)
    , m_queueRx(nullptr)
    , m_mutexTx(nullptr)
    , m_isInitialized(false)
{
}

Uart::~Uart()
{
}

IUart::Status Uart::init()
{
    if (m_huart == nullptr)
    {
        return IUart::Status::ERROR; // UART handle is not initialized
    }
    if (m_semTx == nullptr || m_queueRx == nullptr || m_mutexTx == nullptr)
    {
        return IUart::Status::ERROR; // Semaphore, queue, or mutex not initialized
    }

    if (HAL_UART_Receive_IT(m_huart, const_cast<uint8_t *>(&m_rxChar), 1) != HAL_OK)
    {
        return IUart::Status::ERROR; // Failed to start UART reception
    }

    m_isInitialized = true; // Mark UART as initialized

    return IUart::Status::OK; // Initialization successful
}

IUart::Status Uart::transmit(const char *data, const std::size_t len)
{
    if (data == nullptr || len == 0)
    {
        return IUart::Status::NONE; // No data to transmit
    }

    if (osMutexAcquire(*m_mutexTx, osWaitForever) != osOK)
    {
        return IUart::Status::ERROR; // Failed to acquire mutex
    }

    if (HAL_UART_Transmit_IT(m_huart, reinterpret_cast<uint8_t *>(const_cast<char *>(data)), len) != HAL_OK)
    {
        osMutexRelease(*m_mutexTx);
        return IUart::Status::ERROR; // Transmission failed
    }

    if (osSemaphoreAcquire(*m_semTx, osWaitForever) != osOK)
    {
        osMutexRelease(*m_mutexTx);
        return IUart::Status::ERROR; // Timeout or failed to acquire semaphore
    }

    if (osMutexRelease(*m_mutexTx) != osOK)
    {
        return IUart::Status::ERROR; // Failed to release mutex
    }

    return IUart::Status::OK; // Transmission successful
}

IUart::Status Uart::receive(char *data)
{
    if (data == nullptr)
    {
        return IUart::Status::NONE; // No data to receive
    }

    char rxChar;
    std::size_t rxIdx{0};
    char recBuffer[UART_RX_BUFFER_SIZE] = {0};

    while (1)
    {
        if (osMessageQueueGet(*m_queueRx, &rxChar, nullptr, osWaitForever) != osOK)
        {
            return IUart::Status::ERROR; // Timeout or failed to receive data
        }

        if (rxChar == '\n' || rxChar == '\r') // End of line
        {
            recBuffer[rxIdx] = '\0'; // Null-terminate the string
            memcpy(data, recBuffer, rxIdx + 1); // Copy the received data to the output buffer
            if (osMessageQueueReset(*m_queueRx) != osOK)
            {
                return IUart::Status::ERROR; // Failed to reset the message queue
            }
            break; // Exit the loop on end of line character
        }
        recBuffer[rxIdx++] = rxChar;
    }

    return IUart::Status::OK; // Reception successful
}

IUart::Status Uart::handleRxInterrupt(UART_HandleTypeDef *huart)
{
    if (huart->Instance == m_huart->Instance)
    {
        // Check if the received character is valid
        if (m_rxChar != 0)
        {
            // Send the received character to the message queue
            if (osMessageQueuePut(*m_queueRx, (const void *)&m_rxChar, 0, 0) != osOK)
            {
                return IUart::Status::ERROR;
            }
        }

        // Re-enable UART reception interrupt
        if (HAL_UART_Receive_IT(m_huart, const_cast<uint8_t *>(&m_rxChar), 1) != HAL_OK)
        {
            return IUart::Status::ERROR;
        }
    }

    return IUart::Status::OK;
}

IUart::Status Uart::handleTxInterrupt(UART_HandleTypeDef *huart)
{
    if (huart->Instance == m_huart->Instance)
    {
        // Transmission complete, release the semaphore
        if (osSemaphoreRelease(*m_semTx) != osOK)
        {
            return IUart::Status::ERROR;
        }
    }

    return IUart::Status::OK;
}
