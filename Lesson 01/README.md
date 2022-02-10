# Sound Engine

I'm currently building a MCU based synthesizer, with the goal to build a cheap and off-the-shelf parts based system. It would be nice to have a simple system to test how some waveforms would sound so I could make them actual sound effects/voices for the synth system. The system would need a simple toggle button for the generation of sound, a enconder and button to navigate the UI and configure the sound, a LCD for the visual part of the UI and a DAC to output the soundwaves to an amplifier. Hardware-wise the project is very simple, for its complexity is in software. It must mimic the sample-rate, bit depth and number of voices of the synthesizer in order for the simulations to yeld meaningful results.

![Block Diagram](https://i.imgur.com/uCnEwMV.png)

### MCU

The MCU should be able to process the many voices used in additive, subtractive, FM and wavetable synthesis in a period of time short enough to be able to output the result before the next cycle begins. The limiting factor here is the sample-rate. The higher the sample-rate, the less time the MCU has to calculate everything.

### DAC

The DAC should be able to handle the bit-depth and sample-rate of the output signal and, as a bonus, should have a built-in anti-aliasing filter to save us the trouble.
