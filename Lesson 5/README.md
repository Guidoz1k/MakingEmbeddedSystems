# 3.1 Final project's board

For the final project I'm gonna use a "alternative" board for I couldn't afford the better NUCLEO boards (for import reasons, welcome to BR). The board is a simple breakout board (DevEBox STM32H7XX_M) with only a SPI flash and no more peripherals (only connectors), and at the heart of it there is a Cortex-M7:
 - STM32H743VIT6 (LQFP100)
 - 400MHz max clock
 - 2MB flash / 1MB RAM internal memory
 - Ethernet / QSPI / RNG peripherals / Chrom-ART graphics accelerator / JPEG hardware accelerator
 - 3x fast 16-bit SAR ADCs capable of DMA and low-power operation, self-calibration and oversampling

The [application note](https://www.st.com/resource/en/application_note/an3126-audio-and-waveform-generation-using-the-dac-in-stm32-products-stmicroelectronics.pdf) read for this task was the "Audio and waveform generation using the DAC in STM32 products".

![Board](https://stm32-base.org/assets/img/boards/STM32H743VIT6_STM32H7XX_M-1.jpg)

# 3.2 SPI Flash

## TBD
