This project is an attempt to put a BASIC interperter on the ESP32.

The BASIC programming language is an old one - it became commonplace in the 70's and 80's as computers started to become affordable enough for regular people to get them.
It is a simple language, designed to fit in the small ammount of RAM availible at the time, and was designed to be easy enough to use so that you didn't need to be a scientist or engineer to program your brand new computer. 
BASIC would be availible either as port of your computers ROM, so it would start up as soon as you turned on the computer, or as part of your computer's operating system, which made it very accessible. 

I'm too young to have experienced BASIC in it's heyday, but nevertheless my first programming language also. I learned on an old early 80's PC clone, and it worked well because BASIC was included on a disk. Compare that to what it's like to program something on a computer nowadays: you have to download and install something, and maybe configure some extra stuff. There's editors, source files, libraries, compilers and more to consider, depending on the language, which can be somewhat overwhelming to someone who has never programmed before. 

Now that it's my turn to be asked about getting started in programming, I wanted to create something that you could just turn on and be pretty much ready to go and easy to learn, and BASIC seemed like a good fit.

The ESP32 microcontroller runs TinyBasicPlus, a port of TinyBASIC in C for the AVR arduinos. It's blocking, so I gave it one of the ESP32 cores to run on. The other core is used to serve a webpage, which uses WebSockets to provide a terminal to the BASIC prompt running on the ESP32.
So, the ESP32 can be installed into a robot or whatever, and you just need to turn it on and connect to the webpage in your web browser.
The SAVE and LOAD commands can be used to store your basic programs right on the ESP32's SPIFFS filesystem.



 
