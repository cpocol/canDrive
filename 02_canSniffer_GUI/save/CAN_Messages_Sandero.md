References:
1) Adam's git https://github.com/adamtheone/canDrive
2) https://canhacker.com/examples/renault-kaptur-can-bus/

```
\/ TEMPLATE \/
----------------------------------------------------------------------------------------------------------------------------
0
Descr: 
Source: 
Dest: 
Length (DLC) = 8 Bytes
xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
^^^ TEMPLATE ^^^


----------------------------------------------------------------------------------------------------------------------------
186
Descr: Ignition
Source: 
Dest:
Length (DLC) = 7 Bytes
RRRRRRRR RRRRRRRR xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
|Engine RPM
|Engine RPM = D0 * 32; D1 probably shows number of 1/8th of a rotation
----------------------------------------------------------------------

2)


----------------------------------------------------------------------------------------------------------------------------
18A Accelarator pedal
Descr: 
Source: 
Dest: 
Length (DLC) = 6 Bytes
xxxxxxxx xxxxxxxx TTTTTTTT TTTTTTTT xxxxxxxx xxxxxxxx
                  |Accelerator pedal
                  |D2 is from 0 to 0xC8; D3 is for even more precision


----------------------------------------------------------------------------------------------------------------------------
217
Descr: 
Source: 
Dest: 
Length (DLC) = 8 Bytes
xxxxxxxx xxxxxxxx xxxxxxxx SSSSSSSS SSSSSSSS xxxxxxxx xxxxxxxx xxxxxxxx
                           |Speed
                           |(D2 * 255 + D3) * 0.006274 Km/h

2)


----------------------------------------------------------------------------------------------------------------------------
29A
Descr: Wheels speed
Source: 
Dest: 
Length (DLC) = 8 Bytes
RRRRRRRR RRRRRRRR LLLLLLLL LLLLLLLL SSSSSSSS SSSSSSSS xxxxxxxx xxxxxxxx
|                 |                 |Some sort of speed; varies like the others
|                 |                 -------------------------------------------
|                 |Front left wheel speed
|                 | aaaaaaaaaaaa
|                 --------------
|Front right wheel speed
| aaaaaaaaaaaa
--------------


----------------------------------------------------------------------------------------------------------------------------
350
Descr:
Source: 
Dest: Cluster
Length (DLC) = 8 Bytes
xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxIIxxxx xxxxxxxx
                                                        |Cluster illumination
                                                        |01 - Off
                                                        |10 - On
                                                        ------------------                                                         

2) For cluster ON send:
ID=0x350 DLC=8 DATA=FF 00 00 00 00 00 00 00
Turns on for a very short period of time. It blinks even when sent periodically (1 ms)


----------------------------------------------------------------------------------------------------------------------------
352 Brake
Descr: 
Source: 
Dest: 
Length (DLC) = 4 Bytes
xxxxxxxx xxxxxxxx xxxxxxxx BBBBBBBB
                           |Brake
                           |0 - 255
                           --------


----------------------------------------------------------------------------------------------------------------------------
3B7
Descr: Illumination
Source: 
Dest: 
Length (DLC) = 6 Bytes
xxxxxxxx xxxxxIII Ixxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
              |Illumination
              |1110 - Off
              |0101 - On
              ---------


----------------------------------------------------------------------------------------------------------------------------
3F7 I don't receive it. Probably for automatic gearbox only
Descr: 
Source: 
Dest: 
Length (DLC) = ? Bytes
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
Length (DLC) = 8 Bytes
xxxxHHxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
    |Handbrake
    |01 - Off
    |10 - On
    --------


----------------------------------------------------------------------------------------------------------------------------
55D
Descr: Headlights, Windshield wash
Source: 
Dest: 
Length (DLC) = 8 Bytes
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
Length (DLC) = 8 Bytes
TTTTTTTT xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
|Engine Temperature
|D0 - 40 = temperature in Celsius degrees; range is -40 .. +215
---------------------------------------------------------------


----------------------------------------------------------------------------------------------------------------------------
5DE
Descr: Turn signal lights, fog lights, head lights, doors
Source: 
Dest: 
Length (DLC) = 8 Bytes
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
653
Descr: Seatbelt 
Source: 
Dest: 
Length (DLC) = 4 Bytes
xxxxxxxx xBxxxxxx xxxxxxxx xxxxxxxx
          |Driver seatbelt fastened
          |1 - no
          |0 - yes
          --------


----------------------------------------------------------------------------------------------------------------------------
743
Descr: Ask Odo data via OBD \ UDS protocol
Source: 
Dest: 
Length (DLC) = 8 Bytes
xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
                           |This one seems to matter
                           |07 - Odometer
                           ------------------
2)


----------------------------------------------------------------------------------------------------------------------------
763 (743 + 20)
Descr: Odo data response to 743
Source: 
Dest: 
Length (DLC) = 8 Bytes
xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx OOOOOOOO OOOOOOOO OOOOOOOO xxxxxxxx
                                    |Odo
                                    |(D4 * 255 + D5) * 255 + D6
                                    -------------

```


----------------------------------------------------------------------------------------------------------------------------


Done:
- Cluster
    - Cluster illumination (350)
    - Odometer (763 upon request via 743)
- Doors (5DE)
- Engine RPM (186)
- Engine temperature (5DA)
- Handbrake (4F8)
- Lights
    - Illumination (3B7)
    - Front/rear fog lights (5DE)
    - Low beam (55D, 5DE)
    - High beam (55D, 5DE)
    - Parking lights (5DE)
    - Rear lights (55D)
    - Turn signal/hazard warning lights (5DE)
- Pedals
    - Accelerator (18A)
    - Brake (352)
- Seatbelt (653)
- Speeds 
    - Front left/right wheel speed (29A)
    - Vehicle speed (217)
- Speed gear (incl. rear lights) (55D)


Other derivable info:
- No individual speed gear; the only info is Neutral|Reverse|Forward; BUT: can be determined from RPM vs Speed when in Forward for a while


To do:
- Average speed, average fuel consumption, instantaneous fuel consumtion, etc
- Fuel level: in 5DA, D0 was 5E - should decrease
- Lock/unlock the car (55D?)
- Set the clock
- Yaw rate


Seems not doable:
- Steering wheel angle


Not Doable:
- AC
- Cluster screen change
- Clutch pedal
- Defrost
- Hood open
- Horn
- Interior lights
- Outside mirrors
- Windows
- Wipers
