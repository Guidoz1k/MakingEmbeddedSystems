# Board used

 As I still couldn't manage to make my board work with PlatformIO, I've used a Teensy 4.1 with Arduino framework for abstraction.
Unfortunately I couldn't find any specification of what registers were being used.
 Utilizing the beginner friendly HAL of the Arduino Framework I made use of of an interruption to trigger a polling wait on the main function.
The main function will detect if the input is consistent and debounce it.
 Code can be viewed in the TeensyDemo/src/main.c file.
