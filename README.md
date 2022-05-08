# chevuino
Get information out of your GM OBD1 ECU.
This project enables you to connect to your GM ECU from the 1980s and early 1990s.
After connection, you have some options on how to process or view the data.
The idea is to build an architecture that others can plug into.

# Introduction
The GM ECUs from the 1980s, such as the 1227165 and 1227747 are in loads of of old cars.
Frustrating then, that getting data off them to properly look after your old car can be a real pain.
Which is a shame. There are lots of cool 1980s GM (and other cars than would run great with their old engines).

# OBD1 Basics
Your 1980s GM car has an Engine.
The engine is run by an ECU.
The ECU takes input from sensors, processes it, and sends output to the injectors etc.
The ECU also sends its data out to the ODB1 port in the cockpit as a continuosu 160 bit/sec stream.
You can tap into the stream with an Arduino without needing any electronics.

# Hardware
The Arduino I use is a nano (5$ each). 
Just connect the Arduino to your computer's USB for power and to the ALDL / Ground connections for data input.
The Arduino will convert the specialised signal from the ECU into a simple CSV on your computer's Serial interface.
Although this "Serial" is actually visible from your computer's USB subsystem.
Voila - the ALDL data is available on your PC as a CSV.

# Processing
Simply read lines of data of the USB line. 
Then do whatever you need to do to convert it into human-readable data.
For that you'll need a "definitions file".
Lots of definitions files are around on the internet and chevuino can use them.

# UI
Viewing the data can be done at the hardware level with an LCD screen, 
 at the Processing level on the command line OR
 in a separate UI that gets its data from the processing layer.
This separation means you can view you data on anything from a LCD to a web browser.

# Utilities
The definitions files available on the internet are a bit specialised.
So we need to turn them into JSON. 
This and other utilities are available to save some spadework.




