# FreeRTOS HVAC Controller

STM32 exercise separating sensor acquisition and HVAC control into two periodic FreeRTOS tasks. ADC potentiometers simulate temperature and CO₂ inputs; PWM outputs and GPIOs represent dampers and heaters.

## Repository guide

| Location | Contents |
|---|---|
| [src/](src/) | Reconstructed application source |
| [docs/](docs/) | Real-time operating-systems report |
| [assets/](assets/) | Project overview |

## Getting started

Read the report alongside `src/main.c`. To run on hardware, integrate the source into a STM32CubeIDE project with the matching ADC, GPIO, TIM1 and FreeRTOS configuration.

## Project context

Developed with Sarah Dahmoun. The source was transcribed from report screenshots; generated CubeMX files are not included. Wi-Fi/MQTT belongs to the original brief and is not implemented here.
