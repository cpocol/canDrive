\/ TEMPLATE \/
----------------------------------------------------------------------------------------------------------------------------
0
Descr: 
Source: 
Dest: 
xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
^^^ TEMPLATE ^^^


References:
1) Adam's git https://github.com/adamtheone/canDrive
2) https://canhacker.com/examples/renault-kaptur-can-bus/


----------------------------------------------------------------------------------------------------------------------------
186
Descr: Ignition
Source: 
Dest:
xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
Too many bytes/bits change when turning the key for Ignition/EngineStart/EngineStop; seems impossible to reverse engineer here.
NoIgnition:  (7 bytes) 00,00,31,73,93,00,21
EngineStart: (7 bytes) 18,2A,32,33,3C,00,21 probably D0 is fine; 0 - Engine not running, 1 - Engine running
EngineStop:  (7 bytes) 07,36,2E,F3,AE,00,21

2) DLC=7 DATA=00 00 2C B3 77 00 25 - IGN ON data from the car

1st byte    Tacho
0x1E..0x20  1000 RPM
0x40        ~2000 RPM
=> RPM = D0 * 32


----------------------------------------------------------------------------------------------------------------------------
217
Descr: 
Source: 
Dest: 
xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx

2) DLC=8; DATA=FF FF A0 00 00 00 00 38

4th, 5th bytes are Speed

0x05 0x00 08 km\h; 0x05 0xFF 10km\h

0x09 0x00 14km\h; 0x09 0xFF 16km\h

0x10 0x00 26km\h; 0x10 0xFF 27km\h

0x55 0x00 136km\h; 0x55 0xFF 138km\h

=> 1st byte resolution is 1.6 Km/h; 2nd byte resolution is 1.6 / 255 = 0.006274 Km/h
=> speed = (A * 255 + B) * 0.006274


----------------------------------------------------------------------------------------------------------------------------
350
Descr:
Source: 
Dest: Cluster
xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxIIxxxx xxxxxxxx
                                                        |Cluster illumination
                                                        |01 - Off
                                                        |10 - On
                                                        ------------------                                                         

2)
For cluster ON send:
ID=0x350 DLC=8 DATA=FF 00 00 00 00 00 00 00

----------------------------------------------------------------------------------------------------------------------------
3B7
Descr: Illumination
Source: 
Dest: 
xxxxxxxx xxxxxIII Ixxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
              |Illumination
              |1110 - Off
              |0101 - On
              ---------


----------------------------------------------------------------------------------------------------------------------------
3F7 I don't receive it?
Descr: 
Source: 
Dest: 
xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx


2) DLC=3; DATA=00 00 04;
Gear

First byte PRNDL
0x0F -P ; 0x12 -R; 0x20 -D; 0xFF A-8; 0xF0 A-7; 0xEF A-6; 0xE0 A-5; 0xDF A-4; 0xD0 A-3; 0xCF A-2; 0xC0 A-1

----------------------------------------------------------------------------------------------------------------------------
4F8
Descr: Handbrake
Source: 
Dest: 
xxxxHHxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
    |Handbrake
    |01 - Off
    |10 - On
    --------

Off,4F8 (Handbrake),                                00,00,08, 84,10,30,FF,FF,03,FF,00
On,4F8 (Handbrake),                                 00,00,08, 88,10,30,FF,FF,03,FF,00


----------------------------------------------------------------------------------------------------------------------------
55D
Descr: Headlights, Wash
Source: 
Dest: 
xxxxxxxx xxxRRRxx xxxxxxxx xxxxxxxx xxxxxxxx xHHxxxxx xxxxxxxx xxxxxxxx
            |                                 |
            |                                 |Headlights
            |                                 |00 - Off
            |                                 |10 - Low beam
            |                                 |01 - High beam
            |                                 ---------------
            |Speed gear; Rear lights
            |110 - Neutral
            |001 - Reverse
            |101 - Any forward gear
            ------------------------

Wash,55D (Headlights_Wash),                         00,00,08, 00,DA,60,00,82,80,00,00
Off,55D (Headlights_Wash),                          00,00,08, 00,FD,60,00,81,80,00,00
Low beam,55D (Headlights_Wash),                     00,00,08, 00,FD,60,00,81,C0,00,00
High beam,55D (Headlights_Wash),                    00,00,08, 00,FD,60,00,81,A0,00,00


----------------------------------------------------------------------------------------------------------------------------
5DA
Descr: Engine Temperature
Source: 
Dest: 
TTTTTTTT xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
|Engine Temperature
|D0 - 40 = temperature in Celsius degrees; range is -40 .. +215
---------------------------------------------------------------

----------------------------------------------------------------------------------------------------------------------------
5DE
Descr: Turn signal lights, fog lights, head lights, doors
Source: 
Dest: 
xTTFFPLH xxxxLxRx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
 | |||||     | |   |
 | |||||     | |   |1 - Rear Door Open (Left|Right|Trunk)
 | |||||     | |   ---------------------------------------
 | |||||     | |1 - Right Front Door Open
 | |||||     | ---------------------------
 | |||||     |1 - Left Front Door Open
 | |||||     --------------------------
 | |||||1 - High beam on
 | ||||------------------
 | ||||1 - Low beam on
 | |||-----------------
 | |||1 - Parking lights on
 | ||-----------------------
 | ||1 - Front fog lights on
 | |-------------------------
 | |1 - Rear fog lights on
 | ------------------------
 |Turn Signal
 |01 Left
 |10 Right
 |11 Both - Hazard warning
 ----------------------

----------------------------------------------------------------------------------------------------------------------------
743
Descr: Ask Odo data via OBD \ UDS protocol
Source: 
Dest: 
xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
0x743 8 03 22 02 07 00 00 00 00
2)

----------------------------------------------------------------------------------------------------------------------------
763
Descr: Odo data response to 743
Source: 
Dest: 
xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx OOOOOOOO OOOOOOOO OOOOOOOO xxxxxxxx
                                    |Odo
                                    |just convert these 3 bytes into DEC
                                    -------------

,763,00,00,08, 06,62,02,07,02,3C,AA,A3
02,3C,AA => 146602 Km


----------------------------------------------------------------------------------------------------------------------------

Other derivable info:
- No individual speed gear; the only info is Neutral|Reverse|Forward; BUT: can be determined from RPM vs Speed when in Forward for a while



TODO:
- Engine RPM - see 186
- Speed - see 217
- Individual Wheels speed
- Steering wheel angle
- Yaw rate
- Fuel level
- Seat belts
- Average speed, average fuel consumption, instantaneous fuel consumtion, etc
- set the clock
- Throttle (1F6?)
- Pedals (352?)


Not Doable:
- Cluster screen change
- Horn
- Wipers
- Windows
- Outside mirrors
- interior lights
- Defrost
- Hood open
- AC



