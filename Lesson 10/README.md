# 10 Power consumption

### Sleep mode

 My project does not run on sleep mode. So I'll estimate the current consumption by the microcontroller and all the 4 peripherals.

### Input buttons and the debounce cirtuit

 As the circuit works with transient currents to a debouncing capacitor and to a CD4050 buffer, I'll use here only the current consumed by the buffer according to the datasheet. Ib ~= 10mA. The buttons circuit can be run in 3.3V or 5V. I chose to run it with 5V for now.

### 16x02 LCD screen

 The current for the display with set contrast and full brightness is estimate to be Is ~= 20mA. This display can be run with 3.3V but I'm running it with 5V for lack of components.

### Analog sensor

 The soft-pot strip (ribbon sensor) circuit will consume approximately Ia ~= 0.5mA. The sensor is working with 3.3V to not overload the ADC.

### DAC

 According to the datasheet, the PCM5102a consumes around Id ~= 13mA while in use. This DAC uses 3.3V as it's main voltage.

### Finally the µctrlr

 The STM32H743 is a beefy and powerful microcontroller, which is estimated do consume Im ~= 200mA while in use. The chip uses 3.3V.

## Final sum of currents

 As this project was not designed with efficiency in mind, the current sum is kinda high: It = 243.5mA.\
 This project would work for approximately 9 minutes and 50 seconds before complete depletion of the battery.