# InfectGame
Kinky interactive Borg fan fiction

Releases:
1.0 - Oct. 20, 2019

Infect v1.0
by Borg 5 of 9

Designed for GCC and compatibles
Build:  g++ *.cpp -o infect

It can be built on MSVC with these modifications: remove unistd.h in System, replace sleep(1) with sleep(1000). 
Include the appropriate headers. I may patch it to support MSVC soon.

Modification:
Right now, Infect is co-ed. Check Crew.cpp and look at gender variables to change this.
There is a starting gender and a target gender.
You can also start on the cyborg ship if you look at Infect.cpp (doCybShipSequence)...

You will also want to change the crewperson's name from You to your own name...
This is found in the constructor for Being. 


Feel free to modify and copy, just leave this readme and/or give credit to Borg 5 of 9.

