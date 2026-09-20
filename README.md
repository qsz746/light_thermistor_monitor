# Light & Thermistor Monitor

An STM32F103-based embedded system that monitors ambient light and temperature using timer-triggered ADC sampling, DMA, moving-average filtering, PWM output, and hysteresis-based temperature warning control.

## Features

- Samples a light sensor and thermistor using ADC1 scan mode
- Uses TIM3 TRGO to trigger ADC conversions at 100 Hz
- Uses DMA in circular mode to transfer ADC data with minimal CPU involvement
- Applies an 8-sample moving average filter to both sensor channels
- Controls LED brightness using TIM2 Channel 3 PWM based on ambient light level
- Converts thermistor voltage into temperature using the NTC Beta equation
- Uses hysteresis-based temperature warning control to prevent LED flickering near the threshold
- Outputs filtered sensor readings through USART1 for real-time monitoring

## System Architecture

```text
                         TIM3
                     100 Hz TRGO
                          |
                          v
Light Sensor -------> ADC1 CH0
Thermistor ---------> ADC1 CH1
                          |
                    ADC Scan Mode
                          |
                          v
                    DMA Circular
                          |
               +----------+----------+
               |                     |
               v                     v
       Light Moving Average   Temp Moving Average
          8 samples              8 samples
               |                     |
               v                     v
        PWM Mapping           Voltage -> Temp
               |                     |
               v                     v
        TIM2 Channel 3          Hysteresis
             PWM                   Control
               |                     |
               v                     v
        Light-control LED      Warning LED

                     USART1
                       |
                       v
                Serial Diagnostics
```

## Hardware

- STM32F103C8T6
- Analog light sensor
- 10 kΩ NTC thermistor module
- LED for light-level PWM output
- LED for temperature warning
- CH340 USB-to-Serial adapter

## Pin Configuration
![STM32CubeMX Pin Configuration](pin%20mapping.png)
| Pin | Function |
|---|---|
| PA0 | Light sensor analog input |
| PA1 | Thermistor analog input |
| PA2 | TIM2 Channel 3 PWM output |
| PA5 | Temperature warning LED |
| PA9 | USART1 TX |
| PA10 | USART1 RX |
 

## Peripheral Configuration

### ADC1

ADC1 operates in scan mode with two regular channels:

- Channel 0: light sensor
- Channel 1: thermistor

Conversions are triggered externally by TIM3 instead of running continuously.

### TIM3 - ADC Sampling Timer

TIM3 generates a TRGO update event at 100 Hz.

```text
TIM3 update event
        |
        v
      ADC1
```

This provides a fixed sampling interval of approximately 10 ms.

### DMA

DMA1 Channel 1 transfers ADC conversion results into a two-element buffer:

```c
volatile uint16_t adc_buf[2];
```

DMA operates in circular mode, allowing continuous acquisition without CPU polling.

### TIM2 Channel 3 - PWM

TIM2 Channel 3 generates the PWM signal used to control LED brightness.

The filtered light sensor reading is mapped to the PWM compare register:

```text
Filtered light ADC value
        |
        v
Normalize / scale
        |
        v
TIM2 CH3 CCR
        |
        v
PWM duty cycle
```

## Signal Filtering

Both sensor channels use an 8-sample moving average filter.

At a 100 Hz sampling rate, the filter covers approximately:

```text
8 samples x 10 ms = 80 ms
```

This reduces short-term ADC noise while maintaining responsive sensor readings.

## Temperature Measurement

The thermistor ADC reading is first converted into voltage:

```c
voltage = adc_value / 4095.0f * 3.3f;
```

Temperature is then calculated using the NTC Beta equation.

The current implementation assumes:

- 10 kΩ NTC thermistor at 25°C
- 10 kΩ fixed resistor
- Beta value: 3950 K

## Temperature Warning with Hysteresis

The warning LED uses separate ON and OFF thresholds:

```c
#define TEMP_LED_ON_THRESHOLD   29.0f
#define TEMP_LED_OFF_THRESHOLD  28.0f
```

Behavior:

```text
Temperature >= 29°C       -> LED ON
Temperature <= 28°C       -> LED OFF
28°C < Temperature < 29°C -> Keep previous state
```

The 1°C hysteresis band prevents rapid output switching when sensor readings fluctuate around the threshold.

Temperature control is evaluated at approximately 10 Hz.

## UART Output

USART1 provides serial diagnostics at approximately 2 Hz.

Example output:

```text
Light: 2.437 V, Temp: 24.83 C
Light: 2.421 V, Temp: 24.91 C
Light: 2.405 V, Temp: 25.02 C
```

The slower UART update rate keeps the output readable while ADC sampling continues independently at 100 Hz.

## Software Architecture

The project separates high-frequency data acquisition from slower application tasks.

```text
100 Hz
ADC + DMA
   |
   v
Moving-average filtering
   |
   +---------------------+
   |                     |
   v                     v
10 Hz                  2 Hz
Temperature control    UART output
```

The ADC conversion-complete callback performs lightweight sensor processing, while temperature conversion, warning control, and serial output are handled outside the interrupt context.

## Tools

- STM32CubeMX
- Keil MDK-ARM
- STM32 HAL
- Tera Term
- Git / GitHub

## Key Concepts Demonstrated

- Timer-triggered ADC sampling
- Multi-channel ADC scan mode
- DMA circular buffering
- Interrupt/callback-based data processing
- Moving-average filtering
- PWM generation
- GPIO control
- Thermistor temperature conversion
- Hysteresis-based threshold control
- UART diagnostics
- Separation of tasks with different update rates