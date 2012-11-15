// University of Southampton, 2012
// EMECS Group Design Project

#ifndef __UARTPORT_H
#define __UARTPORT_H

#include <stdint.h>
#include <stdbool.h>


#define RETARGET_LEUART0
/* Override if needed with commandline parameter -DRETARGET_xxx */
#if !defined(RETARGET_USART1) && !defined(RETARGET_LEUART0)
#define RETARGET_USART1    /**< Use USART1 by default. */
#endif

#if defined(RETARGET_USART1)
  #define RETARGET_IRQ_NAME    USART1_RX_IRQHandler         /**< USART IRQ Handler */
  #define RETARGET_CLK         cmuClock_USART1              /**< HFPER Clock */
  #define RETARGET_IRQn        USART1_RX_IRQn               /**< IRQ# 11 */
  #define RETARGET_UART        USART1                       /**< Adress range: 0x4000C400 – 0x4000C7FF */
  #define RETARGET_TX          USART_Tx                     /**< Set TX to USART_Tx */
  #define RETARGET_RX          USART_Rx                     /**< Set RX to USART_Rx */
  #define RETARGET_LOCATION    USART_ROUTE_LOCATION_LOC1    /**< Location of of the USART I/O pins */
  #define RETARGET_TXPORT      gpioPortD                    /**< USART transmission port */
  #define RETARGET_TXPIN       0                            /**< USART transmission pin */
  #define RETARGET_RXPORT      gpioPortD                    /**< USART reception port */
  #define RETARGET_RXPIN       1                            /**< USART reception pin */
  #define RETARGET_USART       1                            /**< Includes em_usart.h */

#elif defined(RETARGET_LEUART0)
  #define RETARGET_IRQ_NAME    LEUART0_IRQHandler           /**< LEUART IRQ Handler */
  #define RETARGET_CLK         cmuClock_LEUART0             /**< LFB Clock */
  #define RETARGET_IRQn        LEUART0_IRQn                 /**< IRQ# 14 */
  #define RETARGET_UART        LEUART0                      /**< Adress range: 0x40084000 – 0x400843FF */
  #define RETARGET_TX          LEUART_Tx                    /**< Set TX to LEUART_Tx */
  #define RETARGET_RX          LEUART_Rx                    /**< Set RX to LEUART_Rx */
  #define RETARGET_TXPORT      gpioPortD                    /**< LEUART transmission port */
  #define RETARGET_TXPIN       4                            /**< LEUART transmission pin */
  #define RETARGET_RXPORT      gpioPortD                    /**< LEUART reception port */
  #define RETARGET_RXPIN       5                            /**< LEUART reception pin */
  #define RETARGET_LOCATION    LEUART_ROUTE_LOCATION_LOC0   /**< Location of of the LEUART I/O pins */
  #define RETARGET_LEUART      1                            /**< Includes em_leuart.h */

#else
#error "Illegal USART/LEUART selection."
#endif

typedef void(*RxHook)(int c);

class UARTPort {
public:
  enum BaudRate {
    uartPortBaudRate4800 = 4800,
    uartPortBaudRate9600 = 9600
  };
  
  enum Parity {
    uartPortParityNone,
    uartPortParityOdd,
    uartPortParityEven
  };
  
  enum DataBits {
    uartPortDataBits8,
    uartPortDataBits9,
  };
  
  enum StopBits {
    uartPortStopBits1,
    uartPortStopBits2
  };
  
public:
  static UARTPort* getInstance()
  {
    static UARTPort instance;
    return &instance;
  }
  
  bool initialize(uint8_t *rxBuffer, uint8_t rxBufferSize, BaudRate baudRate, 
                  DataBits dataBits, Parity parity, StopBits stopBits);
  
  void setRxHook(RxHook h);
  int writeChar(char c);
  int readChar();
  void flushRxBuffer();
  
  // declare ISR for UART as friend function, since it will need access to the
  // protected function handleUARTInterrupt()
  friend void RETARGET_IRQ_NAME();
  
protected: 
  volatile int m_rxReadIndex;
  volatile int m_rxWriteIndex;
  volatile int m_rxCount;
  uint8_t *m_rxBuffer;
  uint8_t m_rxBufferSize;
  bool m_initialized;
  RxHook m_hook;

  // ------ start of singleton pattern specific section ------
  UARTPort();  
  UARTPort(UARTPort const&);                // do not implement
  void operator=(UARTPort const&);        // do not implement
  // ------ end of singleton pattern specific section --------
  
  void handleUARTInterrupt();
};

#endif  // UART_H