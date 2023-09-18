References:\
(1) Adam's git https://github.com/adamtheone/canDrive \
(2) https://canhacker.com/examples/renault-kaptur-can-bus/

```
\/ TEMPLATE \/
===========================================================================================================================
0
Descr: 
Source: 
Dest: 
Length (DLC) = 8 Bytes
........ ........ ........ ........ ........ ........ ........ ........
^^^ TEMPLATE ^^^


===========================================================================================================================
12E
Descr: IMU
Source: 
Dest: 
Length (DLC) = 7 Bytes
LLLLLLLL llllllll llllllll ........ ........ ........ ........
         |Lateral acceleration
         |sin((d1d2 - 32768) * 0.243) G (positive when the car turns to the left <=> the lateral acceleration vector points to the right)
         --------------------------------------------------------------------------------------------------------------------------------
|Longitudinal acceleration
|sin((d0 - 4204) * 0.366) G (positive when the car decelerates <=> the longitudinal acceleration vector points forward)
--------------------------------------------------------------------------------------------------------------------------


===========================================================================================================================
186
Descr: Ignition
Source: 
Dest:
Length (DLC) = 7 Bytes
RRRRRRRR RRRRRRRR ........ ........ ........ ........ ........
|Engine RPM
|Engine RPM = d0 * 32; d1 probably shows number of 1/8ths of a rotation
----------------------------------------------------------------------

(2)


===========================================================================================================================
18A Accelarator pedal
Descr: 
Source: 
Dest: 
Length (DLC) = 6 Bytes
........ ........ TTTTTTTT TTTTTTTT ........ ........
                  |Accelerator pedal
                  |d2 is from 0 to 0xC8; d3 is for even more precision


===========================================================================================================================
217
Descr: Speed
Source: 
Dest: 
Length (DLC) = 7 Bytes
........ ........ ........ SSSSSSSS SSSSSSSS ........ ........
                           |Speed
                           |(d3 * 255 + d4) * 0.006274 Km/h

(2)


===========================================================================================================================
29A
Descr: Wheels speed
Source: 
Dest: 
Length (DLC) = 8 Bytes
RRRRRRRR RRRRRRRR LLLLLLLL LLLLLLLL SSSSSSSS SSSSSSSS 0000dddd ssssdddd
|                 |                 |                     |don't know
|                 |                 |                     |keep changing while standing still; ssss = F when standing
|                 |                 |                     ----------------------------------
|                 |                 |Some sort of speed; varies like the others; it's 47% of the other speeds
|                 |                 -------------------------------------------
|                 |Front left wheel speed
|                 |(d2 * 255 + d3) * 0.006274 Km/h
|                 --------------
|Front right wheel speed
|(d0 * 255 + d1) * 0.006274 Km/h
--------------


===========================================================================================================================
350
Descr: Car lock
Source: 
Dest: Cluster
Length (DLC) = 8 Bytes
........ ........ ........ ........ ........ ........ ..LLRRL. ....rr..
                                                        | | |      |rear Door (Left|Right|Trunk)
                                                        | | |      |01 - closed
                                                        | | |      |10 - open
                                                        | | |Car locked/unlocked
                                                        | | |1 - Locked
                                                        | | |0 - Unlocked
                                                        | | -------------
                                                        | |Front Right Door
                                                        | |01 - closed
                                                        | |10 - open
                                                        | ----------
                                                        |Flont Left Door
                                                        |01 - closed
                                                        |10 - open
                                                        ------------------

(2) For cluster ON send:
ID=0x350 DLC=8 DATA=FF 00 00 00 00 00 00 00
Turns on for a very short period of time. It blinks even when sent periodically (1 ms)


===========================================================================================================================
352 Brake
Descr: 
Source: 
Dest: 
Length (DLC) = 4 Bytes
........ ........ ........ BBBBBBBB
                           |Brake
                           |0 - 255
                           --------


===========================================================================================================================
3B7
Descr: Illumination
Source: 
Dest: 
Length (DLC) = 6 Bytes
........ .....III I....... ........ ........ ........
              |Illumination
              |1110 - Off
              |0101 - On
              ---------


===========================================================================================================================
-3F7 I don't receive it. Probably for automatic gearbox only
Descr: 
Source: 
Dest: 
Length (DLC) = 3 Bytes
........ ........ ........


(2) DLC=3; DATA=00 00 04;
Gear

First byte PRNDL
0x0F -P ; 0x12 -R; 0x20 -D; 0xFF A-8; 0xF0 A-7; 0xEF A-6; 0xE0 A-5; 0xDF A-4; 0xD0 A-3; 0xCF A-2; 0xC0 A-1


===========================================================================================================================
4F8
Descr: Handbrake
Source: 
Dest: 
Length (DLC) = 8 Bytes
....HH.. ........ ........ ........ ........ ........ ........ ........
    |Handbrake
    |01 - Off
    |10 - On
    --------


===========================================================================================================================
55D
Descr: Headlights, Windshield washer
Source: 
Dest: 
Length (DLC) = 8 Bytes
........ ...RRR.. ........ ........ ......LL .HH..... ........ ........
            |                             |   |
            |                             |   |Headlights
            |                             |   |00 - Off
            |                             |   |10 - Low beam
            |                             |   |01 - High beam
            |                             |   ---------------
            |                             |Car locked
            |                             |increments every time the car is locked 0->1->2->3->0->...
            |                             -----------------------------------------------------------
            |Speed gear; Rear lights
            |110 - Neutral
            |001 - Reverse
            |101 - Any forward gear
            ------------------------


===========================================================================================================================
5DA
Descr: Engine Temperature
Source: 
Dest: 
Length (DLC) = 8 Bytes
TTTTTTTT ........ ........ ........ ........ ........ ........ ........
|Engine Temperature
|d0 - 40 = temperature in Celsius degrees; range is -40 .. +215
---------------------------------------------------------------


===========================================================================================================================
5DE
Descr: Lights: Turn signal lights, fog lights, head lights, doors
Source: 
Dest: 
Length (DLC) = 8 Bytes
.TTFFPLH ....L.R. .r...... ........ ........ ........ ........ ........
 | |||||     | |   |
 | |||||     | |   |1 - rear Door Open (Left|Right|Trunk)
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


===========================================================================================================================
653
Descr: Seatbelt 
Source: 
Dest: 
Length (DLC) = 4 Bytes
........ .B...... ........ ........
          |Driver seatbelt fastened
          |1 - no
          |0 - yes
          --------


===========================================================================================================================
743
Descr: Ask Odo data via OBD \ UDS protocol
Source: 
Dest: 
Length (DLC) = 8 Bytes
763000008 ........ ........ ........ AAAAAAAA ........ ........ ........ ........
                                     |This one seems to matter
                                     |07 - Odometer
                                     --------------
(2)


===========================================================================================================================
763 (743 + 20)
Descr: Odo data response to 743
Source: 
Dest: 
Length (DLC) = 8 Bytes
........ ........ ........ ........ OOOOOOOO OOOOOOOO OOOOOOOO ........
                                    |Odo
                                    |(d4 * 255 + d5) * 255 + d6
                                    -------------

===========================================================================================================================
7DF
Descr: OBD parameter request (https://www.csselectronics.com/pages/obd-ii-pid-examples)
Source: 
Dest: 
Length (DLC) = 3 Bytes
7DF000008 02 01 PID 55 55 55 55 55

===========================================================================================================================
7E8
Descr: OBD parameter response
Source: 
Dest: 
Length (DLC) >= 3 (depends in the currne PID)
41 PID d1 d2 ...

```



Done:
- Car locked (350, 55D), unlocked (350)
- Cluster
    - Odometer (763 upon request via 743)
- Doors (350, 5DE)
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
- Yaw rate ()
- Lateral acceleration (12E), in progress: see CAN_Messages_Sandero_Details.txt
 

Seems not doable, yet:
- Steering wheel angle 0xC6
    - the message doesn't come at a good peace
    - 0xC6 d0 goes from 6A (wheel turned rightmost) to 96 (wheel turned leftmost); d1 might be LSB
         d2d3 looks like steering wheel angle variation


Not Doable:
- AC
- Average speed, average fuel consumption, instantaneous fuel consumtion, etc
- Cluster screen change
- Clutch pedal
- Defrost
- Fuel level
- Hood open
- Horn
- Interior lights
- Outside mirrors
- Set the clock
- Windows
- Wipers


```
Map of reverse engineered bytes/bits:

ID   Descr             DLC     d0       d1       d2       d3       d4       d5       d6       d7
---------------------------------------------------------------------------------------------------
12E  IMU                7   LLLLLLLL llllllll llllllll ........ ........ ........ ........
186  Ignition           7   RRRRRRRR RRRRRRRR ........ ........ ........ ........ ........
18A  Acc pedal          6   ........ ........ TTTTTTTT TTTTTTTT ........ ........
217  Speed              7   ........ ........ ........ SSSSSSSS SSSSSSSS ........ ........
29A  Wheels speed       8   RRRRRRRR RRRRRRRR LLLLLLLL LLLLLLLL SSSSSSSS SSSSSSSS 0000dddd ssssdddd
350  Car lock           8   ........ ........ ........ ........ ........ ........ ..LLRRL. ....rr..
352  Brake              4   ........ ........ ........ BBBBBBBB
3B7  Illumination       6   ........ .....III I....... ........ ........ ........
4F8  Handbrake          8   ....HH.. ........ ........ ........ ........ ........ ........ ........
55D  Headlights, Washer 8   ........ ...RRR.. ........ ........ ......LL .HH..... ........ ........
5DA  Engine Temp        8   TTTTTTTT ........ ........ ........ ........ ........ ........ ........
5DE  Lights             8   .TTFFPLH ....L.R. .r...... ........ ........ ........ ........ ........
653  Seatbelt           4   ........ .B...... ........ ........
743  Ask OBD            8   ........ ........ ........ AAAAAAAA ........ ........ ........ ........
763  Odo                8   ........ ........ ........ ........ OOOOOOOO OOOOOOOO OOOOOOOO ........
7DF  OBD Request        3         02       01      PID
7E8  OBD Response    >= 3         41      PID       d1       d2 ...

```
