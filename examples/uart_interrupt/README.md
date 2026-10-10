# STM32 Interrupt-Driven UART Exercise

Standalone UART-reception example relocated from Divers_TP. [main_uart_reception_interruption.c](main_uart_reception_interruption.c) preserves the original source.

Review the STM32 HAL handles, interrupt callbacks and peripheral configuration in the source. Integrate it into a matching STM32 project with its startup files and HAL dependencies; this single file is not a complete board firmware.

The example illustrates a peripheral mechanism relevant to the HVAC project. It is separate from the main FreeRTOS application.
