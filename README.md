# University-Project
Function Generator Team Project

# Real-Time Embedded Function Generator (C/C++)

A deterministic, real-time embedded system utilizing C/C++ and FreeRTOS to achieve low-latency signal synthesis. 

## Technical Overview
* **Operating System:** FreeRTOS (Task prioritization, queue management)
* **Peripherals Configured:** DAC, UART, Hardware Timers, Quadrature Encoders
* **Language:** C/C++

## System Architecture & Optimization
* Configured hardware registers directly to bypass abstraction layers and minimize signal latency.
* Optimized task priorities and algorithmic execution loops to prevent thread starvation and minimize timing jitter.
* Developed mathematical wave synthesis logic to generate real-time precise trigonometric and digital waveforms.
