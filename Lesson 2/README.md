# 2.1 Harware and software block diagrams

I'm going to reuse last session's project in this task as well for I intend to design, program and develop a prototype out of it. The diagrams now are more detailed, one being focused on hardware connections and the other focusing in software dependencies.

![Block Diagram 1](https://i.imgur.com/TjlFCZl.png)

# 2.2 Mbed

I chose the DISCO-H747I for it's based on the STM32H747. The processor is not only closely related to the STM32H743, the same processor in the board I chose to work with, but it's also a dual core ARM Cortex-M7 microcontroller, which is super interesting. The board also uses a OTM8009A TFT LCD driver which I'll use in a future project. I hope one day work with this exact board. I used the template "DISCO_H747I_LCD_demo". This board contains the following parts:
- WM8994 audio codec hub
- OV9655 camera
- OTM8009A LCD driver
- MT25TL01G QSPI memory
- FT6X06 IO expander
The template only accesses the LCD driver to set it up and run a demo code to show the processor/driver capabilities. Unfortunately the template didn't use much beyond a LCD driver for software and the LCD interface for hardware so the diagram is super simplistic.

![Block Diagram 2](https://i.imgur.com/lvPPWoI.png)
