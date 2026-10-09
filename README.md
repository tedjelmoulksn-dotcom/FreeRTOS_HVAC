# FreeRTOS HVAC Control — STM32 Embedded Prototype

A C prototype that separates air-quality and temperature control into two FreeRTOS tasks. ADC inputs emulate environmental measurements; PWM and GPIO outputs represent ventilation and heating actuators.

## Architecture

| Function | Implementation |
|---|---|
| Acquisition | STM32 HAL ADC channel selection, conversion polling and 12-bit normalisation |
| Air-quality control | CO₂ input scaled to a 0–2000 ppm range; stepped damper duty |
| Temperature control | Input scaled to 0–40 °C; stepped airflow demand |
| Actuation | Two timer compare channels for PWM and GPIO-controlled heater states |
| Scheduling | Two tasks at `tskIDLE_PRIORITY + 1`, with a 1000 ms relative delay |
| Diagnostics | `printf` routed through ITM character output |

Source: [`src/main.c`](src/main.c). Report: [`docs/`](docs/).

## Control rules

The CO₂ controller requests 0%, 25%, 50%, 75% or 100% opening across thresholds of 600, 700, 1000 and 1500 ppm. The temperature controller requests 0%, 20%, 50%, 30%, 10% or 5% across thresholds of 19, 21, 23, 25 and 30 °C.

PWM is calculated from the timer auto-reload value: `CCR = ARR * duty / 100`. The combined demand is capped at 100%. Below 14 °C, both heater outputs are asserted; above that threshold, the first is cleared and the second toggles in the supplied implementation.

These are exercise rules, not a tuned closed-loop PID controller.

## RTOS and hardware details

Task stacks are declared as 128 FreeRTOS stack units, whose byte size depends on the port. The delay is relative to task execution, so it does not establish an exact one-second sampling period.

ADC polling uses `HAL_MAX_DELAY`; conversion faults can therefore block a task indefinitely. Task creation results are not checked. Shared measurements and diagnostic output should be reviewed for synchronisation before extending the prototype.

The source contains peripheral pin comments, but no complete board project. Validate timer alternate functions, ADC mapping, clock configuration and heater drive circuitry against the actual hardware.

## Building and reproducing

```bash
git clone https://github.com/tedjelmoulksn-dotcom/FreeRTOS_HVAC.git
cd FreeRTOS_HVAC
```

Integrate the application into a compatible STM32 project containing HAL drivers, startup code, linker script, FreeRTOS port and `FreeRTOSConfig.h`. Configure ADC, timers, GPIO and the RTOS time base before enabling the tasks.

The repository does not currently provide a self-contained firmware build. The inputs are simulated through analogue scaling; no digital CO₂ sensor, wireless stack or remote monitoring service is implemented.

## Verification priorities

Check ADC full-scale conversion, each threshold boundary, PWM duty and frequency, heater behaviour, stack usage and task timing on hardware. Use finite timeouts, consistent measurement snapshots and `vTaskDelayUntil` if a fixed-period design is required. ITM output can affect execution time and should be measured.

No hardware or timing tests were rerun during this documentation update.

## Licence

No project-wide licence has been defined.
