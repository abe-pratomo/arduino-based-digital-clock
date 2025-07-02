# Arduino-based Digital Clock with Stopwatch and Timer/Alarm Modes
This is an Arduino IDE, C++ based digital clock software implementation, complete with timer/alarm and stopwatch modes, created in March 2024 to fulfill a big project (Tugas Besar) assignment for the Microprocessor System class in the Electrical Engineering program at Bandung Institute of Technology. It uses an Arduino family microcontroller (an Arduino Nano for this example).

## Contents
Source code ["source.ino"](https://github.com/abe-pratomo/arduino-based-digital-clock/blob/ca226d2bdaf0533ba9773980b33355e2a18d386e/source/source.ino) is available inside the [source](https://github.com/abe-pratomo/arduino-based-digital-clock/tree/85a2a71484c9189f11c33efd1d249a6488de6d89/source) folder, complete with comments, written in Bahasa Indonesia, to make the code easier to read and understand. A quick and simple demonstration of the design can also be found in the YouTube video below (note that it was narrated in Bahasa Indonesia as well).

### Demonstrasi Jam Digital - 13321051 - 13221087
[![Demonstrasi Jam Digital - 13321051 - 13221087](https://img.youtube.com/vi/WRGpoUvOJQ0/0.jpg)](https://www.youtube.com/watch?v=WRGpoUvOJQ0)

Reports (also note that they were written in Bahasa Indonesia as well), including reports of the [design](https://github.com/abe-pratomo/arduino-based-digital-clock/blob/2079f8cc46cc64fba15c7e0c103756fd6f0a1d97/docs/Laporan%20Proyek%20Jam%20Digital%20v1%20-%2013221051%20-%2013221087.pdf), of the [initial implementation](https://github.com/abe-pratomo/arduino-based-digital-clock/blob/2079f8cc46cc64fba15c7e0c103756fd6f0a1d97/docs/Laporan%20Proyek%20Jam%20Digital%20v2%20-%2013221051%20-%2013221087.pdf) which only had a simple clock mode, and of the [final implementation](https://github.com/abe-pratomo/arduino-based-digital-clock/blob/2079f8cc46cc64fba15c7e0c103756fd6f0a1d97/docs/Laporan%20Proyek%20Jam%20Digital%20Final%20-%2013221051%20-%2013221087.pdf) which had been completed with timer/alarm and stopwatch modes, can be found inside the [docs](https://github.com/abe-pratomo/arduino-based-digital-clock/tree/ec21a75530080ddde1a75926af215acb00b6324c/docs) folder.


## Suggestions for Future Improvement
The author suggests anyone who wishes to use this entirely open-sourced (!) project consider using the flash memory inside their microcontroller of choice (for example, the Arduino Nano) to save the current time when the device turns off. It is also suggested to use an RTC module to continue the timekeeping when the device is off.
