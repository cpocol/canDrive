Original repo: Adam's repo https://github.com/adamtheone/canDrive

Achievement of this fork:

1. Reverse engineering of CAN messages on a Dacia Sandero second generation. They should be the same for many models in the Renault group

    Reverse engineered parameters are detailed in: https://github.com/cpocol/canDrive/blob/main/02_canSniffer_GUI/save/CAN_Messages_Sandero.md

2. Speeding up the UI

    When the laptop is in power saving/low performance mode, CAN packets (that are buffered along the pipe) continue to be received after unplugging from the car. More details are available in an issue of the original repo: https://github.com/adamtheone/canDrive/issues/35

3. Building a simple car simulator
	- it sends a message (id = 0x5DA) whose first byte represents the engine coolant temperature. It increases one degree every half a second
	- it sends a message (id = 0x186) representing the engine speed (aka engine RPM). It is manually controlled by a potentiometer
	- it is based on Arduino (Nano)
	- implementation in https://github.com/cpocol/canDrive/blob/main/00_SimpleCarSimulator_Arduino
		- the reason why some source files have the "my_" prefix is to avoid messing up with libraries that may already be installed (they are copies of the needed installed libraries). Also, there is no such need as installing those libraries

4. Building a simple OBD car simulator
	- implementation in https://github.com/cpocol/canDrive/blob/main/000_SimpleOBDCarSimulator_Arduino

![Snapshot](000_SimpleOBDCarSimulator_Arduino/Snapshot.gif)

5. canSniffer was dropping CAN messages. For some reason, the CAN library (https://github.com/sandeepmistry/arduino-CAN) wasn't fast enough. I tried it with both the interrupt based method and the polling based method: same behavior. Switching to a different CAN library (https://github.com/autowp/arduino-mcp2515) has fixed the problem: the message rate went up from 600 messages/second to 900 messages/second. This has fixed an issue that I have observed in the past: the steering wheel angle was coming at a non constant rate (usually it was coming tens of times per second, but sometimes it was missing for several seconds). The new sniffer is available at https://github.com/cpocol/canDrive/tree/main/01_canSniffer_Arduino/canSniffer_autowp
Sending messages to the car (from 02_canSniffer_GUI) is not currently handled. Still, this can be done in a custom manner in the loop function of canSniffer_autowp (some examples are there and commented out). Exemplified by https://www.youtube.com/shorts/TLSTojeKgg8
