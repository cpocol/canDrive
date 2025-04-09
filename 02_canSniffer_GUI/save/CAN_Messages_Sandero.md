## References:
(1) Adam's git https://github.com/adamtheone/canDrive \
(2) https://canhacker.com/examples/renault-kaptur-can-bus/

## Map of reverse engineered bytes/bits (more details below):
```

ID(hex) Descr                                   DLC     d0       d1       d2       d3       d4       d5       d6       d7
-------------------------------------------------------------------------------------------------------------------------
   12E  IMU                                      7   LLLLLLLL llllllll llllllll ........ ........ ........ ........
   186  EngineRPM                                7   RRRRRRRR RRRRRRRR ........ ........ ........ ........ ........
   18A  Acceleration pedal                       6   ........ ........ PPPPPPPP PPPPPPPP ........ ........
   217  Speed                                    7   ........ ........ ........ SSSSSSSS SSSSSSSS ........ ........
   29A  Wheels speed                             8   RRRRRRRR RRRRRRRR LLLLLLLL LLLLLLLL SSSSSSSS SSSSSSSS 0000dddd ssssdddd
   350  Car lock, Brake status, Doors            8   ........ ........ ........ ........ ........ iBBB.... ..LLRRC. ....rr..
   352  Brake pedal                              4   ........ ........ ........ BBBBBBBB
   3B7  Illumination                             6   ........ .....III I....... ........ ........ ........
   4F8  Handbrake, Key                           8   XI.DHH.. ........ ........ ........ ........ ........ ........ ........
   55D  Speed gear, Car lock, Headlights         8   ........ ...RRR.. ........ ........ ......LL .HH..... ........ ........
   5DA  Engine Temperature                       8   TTTTTTTT ........ ........ ........ ........ ........ ........ ........
   5DE  Lights (turn signal, fog, parking        8   .TTFFPLH ....L.R. .r...... ........ ........ ........ ........ ........
        headlights), Doors
   653  Seatbelt                                 4   ........ .B...... ........ ........
   743  Odo request                              8   ........ ........ ........ AAAAAAAA ........ ........ ........ ........
   763  Odo response                             8   ........ ........ ........ ........ OOOOOOOO OOOOOOOO OOOOOOOO ........
   7DF  OBD Request                              3         02       01      PID
   7E8  OBD Response                          >= 3         41      PID       d1       d2 ...

```



```
\/ TEMPLATE for new CAN message IDs \/
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
|        |Lateral acceleration
|        |sin((d1d2 - 32768) * 0.243) G (positive when the car turns to the left <=> the lateral acceleration vector points to the right)
|        --------------------------------------------------------------------------------------------------------------------------------
|Longitudinal acceleration
|sin((d0 - 4204) * 0.366) G (positive when the car decelerates <=> the longitudinal acceleration vector points forward)
--------------------------------------------------------------------------------------------------------------------------


===========================================================================================================================
186
Descr: EngineRPM
Source: 
Dest:
Length (DLC) = 7 Bytes
RRRRRRRR RRRRRRRR ........ ........ ........ ........ ........
|Engine RPM
|Engine RPM = d0 * 32; d1 probably shows number of 1/8ths of a rotation
----------------------------------------------------------------------

(2)


===========================================================================================================================
18A Acceleration pedal
Descr: 
Source: 
Dest: 
Length (DLC) = 6 Bytes
........ ........ PPPPPPPP PPPPPPPP ........ ........
                  |Accelerator pedal
                  |d2 is from 0 to 200 (0xC8); d3 is for even more precision
				  ----------------------------------------------------------


===========================================================================================================================
217
Descr: Speed
Source: 
Dest: 
Length (DLC) = 7 Bytes
........ ........ ........ SSSSSSSS SSSSSSSS ........ ........
                           |Speed
                           |(d3 * 255 + d4) * 0.006274 Km/h
						   --------------------------------

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
|                 |                 |                     -----------------------------------------------------------
|                 |                 |Some sort of speed; varies like the others; it's 47% of the other speeds
|                 |                 -------------------------------------------------------------------------
|                 |Front left wheel speed
|                 |(d2 * 255 + d3) * 0.006274 Km/h
|                 --------------------------------
|Front right wheel speed
|(d0 * 255 + d1) * 0.006274 Km/h
--------------------------------


===========================================================================================================================
350
Descr: Car lock, Brake status, Doors
Source: 
Dest: Cluster
Length (DLC) = 8 Bytes
........ ........ ........ ........ ........ iBBB.... ..LLRRC. ....rr..
                                             ||         | | |      |rear Door (Left|Right|Trunk)
                                             ||         | | |      |01 - closed
                                             ||         | | |      |10 - open
                                             ||         | | |Car locked/unlocked
                                             ||         | | |1 - Locked
                                             ||         | | |0 - Unlocked
                                             ||         | | -------------
                                             ||         | |Front Right Door
                                             ||         | |01 - closed
                                             ||         | |10 - open
                                             ||         | ----------
                                             ||         |Flont Left Door
                                             ||         |01 - closed
                                             ||         |10 - open
                                             ||         ------------------
                                             ||Brake status, when key in ignition position                      D3 in 0x352 is tightly related
                                             ||001 brake pedal is slightly pressed or not pressed at all        D3 in 0x352 is 0
                                             ||010 brake pedal is pressed a little more                         D3 in 0x352 is 1
                                             ||100 brake pedal is pressed even more; the brake lights turn on   D3 in 0x352 is at least 1
                                             |-------------------------------------------------------------------------------------------
                                             |Ignition?: turns into 1 like 2-4 seconds after the key gets into ignition position 
                                             |           turns into 0 immediately after the key gets back from ignition position
                                             ----------------------------------------------------------------------------------

(2) For cluster ON send:
ID=0x350 DLC=8 DATA=FF 00 00 00 00 00 00 00
Turns on for a very short period of time. It blinks even when sent periodically (1 ms)


===========================================================================================================================
352 Brake pedal
Descr: 
Source: 
Dest: 
Length (DLC) = 4 Bytes
........ ........ ........ BBBBBBBB
                           |Brake pressure
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
Descr: Handbrake, Key
Source: 
Dest: 
Length (DLC) = 8 Bytes
XI.DHH.. ........ ........ ........ ........ ........ ........ ........
|| | |Handbrake
|| | |01 - Off
|| | |10 - On
|| | --------
|| |Immediate/delayed value. Note that: Ignition and Off/Accessories can't be simultaneously on
|| |When ignition on: 0 - immediate
|| |                  1 - delayed 1"
|| |When Off/Acc. on: 1 - immediate
|| |                  0 - delayed 12"
|| ----------------------------------
||Ignition
|---------
|Off/Accessories
----------------


===========================================================================================================================
55D
Descr: Speed gear, Car lock, Headlights
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
Descr: Lights (turn signal, fog, parking, headlights), Doors
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
 -------------------------


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
Descr: Odo request via OBD \ UDS protocol
Source: 
Dest: 
Length (DLC) = 8 Bytes
763000008 03 22 02 07 00 00 00 00
                   |This one seems to matter
                   |07 - Odometer
                   --------------

(2)


===========================================================================================================================
763 (743 + 20)
Descr: Odo response to 743
Source: 
Dest: 
Length (DLC) = 8 Bytes
........ ........ ........ ........ OOOOOOOO OOOOOOOO OOOOOOOO ........
                                    |Odo
                                    |(d4 * 255 + d5) * 255 + d6
                                    ---------------------------

===========================================================================================================================
7DF
Descr: OBD request
Source: TESTER
Dest: CAR

Documentation available at https://en.wikipedia.org/wiki/OBD-II_PIDs
but here are some real examples.

(example of) real request (aka query):
ID  DLC d0 d1 d2 d3 d4 d5 d6 d7
-------------------------------
7DF 8   02 01 0C
        |  |  |PID code:
        |  |  |00 = PIDs supported [$01 - $20]
        |  |  |04 = Calculated engine load
        |  |  |05 = Engine coolant temperature
        |  |  |0C = Engine speed <-----
        |  |  |0D = Vehicle speed
        |  |  |20 = PIDs supported [$21 - $40]
        |  |  --------------------------------
        |  |Service/Mode; 1 = show current data
        |  ------------------------------------
        |Number of additional data bytes
        --------------------------------

===========================================================================================================================
7E8
Descr: OBD response
Source: CAR
Dest: TESTER

(example of) real response for PIDs supported [$01 - $20]:
ID  DLC d0 d1 d2 d3 d4 d5 d6 d7
7E8 8   06 41 00 BE 3E B8 11
0xBE3EB811 = 10111110 00111110 10111000 00010001
             | |||||    |||||  | |||       |   |PIDs supported [$21 - $40]
             | |||||    |||||  | |||       |   --------------------------------------------
             | |||||    |||||  | |||       |OBD standards this vehicle conforms to
             | |||||    |||||  | |||       --------------------------------------------
             | |||||    |||||  | |||Oxygen Sensor 2 A: Voltage B: Short term fuel trim
             | |||||    |||||  | ||---------------------------------------------------
             | |||||    |||||  | ||Oxygen Sensor 1 A: Voltage B: Short term fuel trim
             | |||||    |||||  | |---------------------------------------------------
             | |||||    |||||  | |Oxygen sensors present (in 2 banks)
             | |||||    |||||  | ------------------------------------
             | |||||    |||||  |Throttle position
             | |||||    |||||  ------------------
             | |||||    |||||Intake air temperature
             | |||||    ||||-----------------------
             | |||||    ||||Timing advance
             | |||||    |||---------------
             | |||||    |||Vehicle speed
             | |||||    ||--------------
             | |||||    ||Engine speed
             | |||||    |-------------
             | |||||    |Intake manifold absolute pressure
             | |||||    ----------------------------------
             | |||||Long term fuel trim (LTFT)—Bank 1
             | ||||----------------------------------
             | ||||Short term fuel trim (STFT)—Bank 1
             | |||-----------------------------------
             | |||Engine coolant temperature	
             | ||---------------------------
             | ||Calculated engine load
             | |-----------------------
             | |Fuel system status	
             | -------------------
             |Monitor status since DTCs cleared
             ----------------------------------

(example of) real response for PIDs supported [$21 - $40]:
ID  DLC d0 d1 d2 d3 d4 d5 d6 d7
-------------------------------
7E8 8   06 41 20 A0 01 80 01
0xA0018001 = 10100000 00000001 10000000 00000001
             | |             | |               |PIDs supported [$41 - $60]
             | |             | |               ---------------------------
             | |             | |Distance traveled since codes cleared
             | |             | --------------------------------------
             | |             |Warm-ups since codes cleared
             | |             -----------------------------
             | |Fuel Rail Gauge Pressure (diesel, or gasoline direct injection)
             | ----------------------------------------------------------------
             |Distance traveled with malfunction indicator lamp (MIL) on
			 -----------------------------------------------------------

(example of) real response for PIDs supported [$41 - $60]:
ID  DLC d0 d1 d2 d3 d4 d5 d6 d7
-------------------------------
7E8 8   06 41 40 80 00 00 00
0x80000000 = 10000000 00000000 00000000 00000000
             |Monitor status this drive cycle
			 --------------------------------

===========================================================================================================================

(example of) real OBD communication (TESTER = Torque app running on smart phone):
Timestamp       Direction           ID  DLC d0 d1 d2 d3 d4 d5 d6 d7
-------------------------------------------------------------------
14:37:01.294 -> From TESTER to CAR: 7DF 8   02 01 00 00 00 00 00 00 Request: PIDs supported [$01 - $20]
14:37:01.294 -> From CAR to TESTER: 7E8 8   06 41 00 BE 3E B8 11 AA Response: PIDs supported [$01 - $20]
										    
14:37:01.786 -> From TESTER to CAR: 7DF 8   02 01 00 00 00 00 00 00 Request: PIDs supported [$01 - $20]
14:37:01.823 -> From CAR to TESTER: 7E8 8   06 41 00 BE 3E B8 11 AA Response: PIDs supported [$01 - $20]
										    
14:37:02.245 -> From TESTER to CAR: 7DF 8   02 01 00 00 00 00 00 00 Request: PIDs supported [$01 - $20]
14:37:02.245 -> From CAR to TESTER: 7E8 8   06 41 00 BE 3E B8 11 AA Response: PIDs supported [$01 - $20]
										    
14:37:02.652 -> From TESTER to CAR: 7DF 8   02 01 00 00 00 00 00 00 Request: PIDs supported [$01 - $20]
14:37:02.698 -> From CAR to TESTER: 7E8 8   06 41 00 BE 3E B8 11 AA Response: PIDs supported [$01 - $20]
										    
14:37:03.133 -> From TESTER to CAR: 7DF 8   02 01 00 00 00 00 00 00 Request: PIDs supported [$01 - $20]
14:37:03.133 -> From CAR to TESTER: 7E8 8   06 41 00 BE 3E B8 11 AA Response: PIDs supported [$01 - $20]
										    
14:37:03.212 -> From TESTER to CAR: 7DF 8   02 01 20 00 00 00 00 00 Request: PIDs supported [$21 - $40]
14:37:03.260 -> From CAR to TESTER: 7E8 8   06 41 20 A0 01 80 01 AA Response: PIDs supported [$21 - $40]
										    
14:37:03.352 -> From TESTER to CAR: 7DF 8   02 01 40 00 00 00 00 00 Request: PIDs supported [$41 - $60]
14:37:03.352 -> From CAR to TESTER: 7E8 8   06 41 40 80 00 00 00 AA Response: PIDs supported [$41 - $60]
										    
14:37:03.446 -> From TESTER to CAR: 7DF 8   02 01 05 00 00 00 00 00 Request: Engine coolant temperature
14:37:03.446 -> From CAR to TESTER: 7E8 8   03 41 05 49 AA AA AA AA Response: Engine coolant temperature: 0x49 - 40 = 33 degrees
										    
14:37:03.480 -> From TESTER to CAR: 7DF 8   02 01 0B 00 00 00 00 00 Request: Intake manifold absolute pressure
14:37:03.525 -> From CAR to TESTER: 7E8 8   03 41 0B 61 AA AA AA AA Response: Intake manifold absolute pressure: 97 kPa
										    
14:37:03.572 -> From TESTER to CAR: 7DF 8   02 01 10 00 00 00 00 00 Request: Mass air flow sensor (MAF) air flow rate
                                                                    It's not supported! Why request it?
										    
14:37:04.024 -> From TESTER to CAR: 7DF 8   02 01 0C 00 00 00 00 00 Request: Engine speed
14:37:04.024 -> From CAR to TESTER: 7E8 8   04 41 0C 00 00 AA AA AA Response: Engine speed: 0 rpm
										    
14:37:04.117 -> From TESTER to CAR: 7DF 8   02 01 04 00 00 00 00 00 Request: Calculated engine load
14:37:04.117 -> From CAR to TESTER: 7E8 8   03 41 04 00 AA AA AA AA Response: Calculated engine load: 0%
										    
14:37:04.195 -> From TESTER to CAR: 7DF 8   02 01 33 00 00 00 00 00 Request: Absolute Barometric Pressure
                                                                    It's not supported! Why request it?

```



## Done:
- Car locked (350, 55D), unlocked (350)
- Cluster
    - Odometer (763 upon request via 743)
- Doors (350, 5DE)
- Engine RPM (186)
- Engine temperature (5DA)
- Ignition (350)
- Handbrake (4F8)
- Lateral acceleration (12E)
- Lights
    - Illumination (3B7)
    - Front/rear fog lights (5DE)
    - Low beam (55D, 5DE)
    - High beam (55D, 5DE)
    - Parking lights (5DE)
    - Rear lights (55D)
    - Turn signal/hazard warning lights (5DE)
    - Brake lights (350)
- Pedals
    - Accelerator (18A)
    - Brake (352)
- Seatbelt (653)
- Speeds 
    - Front left/right wheel speed (29A)
    - Vehicle speed (217)
- Speed gear (incl. rear lights) (55D)


Other derivable info:
- No individual speed gear; the only info is Neutral|Reverse|Forward; BUT: can be determined from RPM vs Speed when in Forward for a while (the clutch needs some time until fully engaged - less than a second; filtering out short RPM/Speed matches should suffice here)


## To do:
- Yaw rate
 

## Seems not doable, yet:
- Steering wheel angle 0xC6
    - the message doesn't come at a good pace
    - d0 goes from 96 (wheel turned leftmost) to 6A (wheel turned rightmost); d1 might be LSB
    - d2d3 looks like steering wheel angle variation


## Not Doable:
- AC
- Average speed, average fuel consumption, instantaneous fuel consumption, etc
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

## Message frequency, without turning ignition on (just open the car)
```
ID  (Label)                       msg/s
---------------------------------------
12E (IMU)                          97.8
29A (WheelsSpeed)                  48.1
242                                46.6
354                                22.5
352 (Brake)                        22.5
5DE (Lights)                       10.0
5DF                                 9.8
3B7 (Illumination)                  9.7
4F8 (Handbrake)                     9.7
55D (SpeedGear_CarLock_Headlights)  9.6
666                                 9.3
5D7                                 9.2
350 (CarLock)                       7.7
69F                                 1
C6                                  0.9
6FB                                 0.3
                            sum   314.7
```

## Message frequency, engine turned on (not updated)
```
ID  (Label)                       msg/s
---------------------------------------
12E (IMU)                          90.6
29A (WheelsSpeed)                  44.6
242                                43.0
18A (AccelarationPedal)            41.2
1F6                                28.2
354                                21.0
352 (Brake)                        21.0
217 (Speed)                        20.6
2C6                                16.1
5DE (Lights)                        9.2
3B7 (Illumination)                  9.1
4F8 (Handbrake)                     9.1
5DF                                 9.0
666                                 8.9
55D (SpeedGear_CarLock_Headlights)  8.8
5D7                                 8.6
350 (CarLock)                       7.0
575                                 5.1
653 (Seatbelt)                      4.9
552                                 4.2
564                                 4.1
65C                                 3.9
500                                 3.8
186 (EngineRPM)                     2.2
C6                                  1.3
5DA (EngineTemperature)             1.1
648                                 1.0
511                                 0.9
69F                                 0.9
29C                                 0.8
6FB                                 0.3
                            sum   430.5
```
