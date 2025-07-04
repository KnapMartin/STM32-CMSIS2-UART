#ifndef UART_H
#define UART_H

#include "main.h"
#include "cmsis_os2.h"
#include <cstdint>
#include <cstring>

constexpr std::size_t UART_RX_BUFFER_SIZE{64}; // Size of the receive buffer

class Uart
{
    public:
        enum class Status
        {
            OK,
            ERROR,
            NONE, // No data to transmit or receive
        };

        Uart(UART_HandleTypeDef *huart);
        ~Uart();

        void setHandleSemTx(osSemaphoreId_t *semTx) {m_semTx = semTx;}
        void setHandleQueueRx(osMessageQueueId_t *queueRx) {m_queueRx = queueRx;}
        void setHandleMutexTx(osMutexId_t *mutexTx) {m_mutexTx = mutexTx;}
        Uart::Status init(); 
        Uart::Status transmit(const char* data, const std::size_t len);
        Uart::Status receive(char* data);
        Uart::Status handleRxInterrupt(UART_HandleTypeDef *huart);
        Uart::Status handleTxInterrupt(UART_HandleTypeDef *huart);
        bool isInitialized() const { return m_isInitialized; }

    private:
        volatile uint8_t m_rxChar;
        UART_HandleTypeDef *m_huart;
        osSemaphoreId_t *m_semTx;
        osMessageQueueId_t *m_queueRx;
        osMutexId_t *m_mutexTx;
        bool m_isInitialized;
};



#endif