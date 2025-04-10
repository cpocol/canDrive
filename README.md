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
		
4. Adding support for a display on the firmware from 01_canSniffer_Arduino
	- if the display is missing, the corresponding implementation just does nothing
	- the display can be disabled by setting USE_DISPLAY to 0
	- using the display might cause some CAN messages to be dropped

5. Building a simple OBD car simulator
	- implementation and showcase in https://github.com/cpocol/canDrive/blob/main/000_SimpleOBDCarSimulator_Arduino
