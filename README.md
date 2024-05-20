Original repo: Adam's repo https://github.com/adamtheone/canDrive

- The main achievement of this fork is reverse engineering of CAN messages on a Dacia Sandero second generation. They should be the same for many models in the Renault group.

    Reverse engineered parameters are datailed in: https://github.com/cpocol/canDrive/blob/main/02_canSniffer_GUI/save/CAN_Messages_Sandero.md

- A secondary achievement is speeding up the UI.

    When the laptop is in power saving/low performance mode, CAN packets (that are buffered along the pipe) continue to be received after unplugging from the car. More details are avaiable in an issue of the original repo: https://github.com/adamtheone/canDrive/issues/35
