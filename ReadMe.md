# Water Filter Project
- Source code handling the logic of a water filter
- Contains other files to run the simulation at WOKWI
- The system has 3 tanks that contain water:
  - Intermediate Tank (Tank that hold the inputed water before the water passes through the filter)
  - Filtered Water Tank (Tank that hold the water that passed through the filter)
  - Output Tank (Tank that contains the filtered water that was pumped from the Filtered Water Tank)
    
## Functionality
- Push button to dispense water
- LCD Display
  - Displays Filtered Water/Output Tank Capacity
  - Displays PH levels of Intermediate/Output Tank
  - Displays Tank States
  - Displays Fault States
- Fault Detection
  - When too much water is in the Filtered Water/Output Tank
  - PH level in Output Tank is greater than 9ph or less than 4ph
  - When there is no water in the Filtered Water Tank
    
## Code Logic
### Tank States
0% == Full;100% == Empty
| Tank State | Filtered Water Tank Capacity | Output Tank Capacity| Extra Conditions |
|-|-|-|-|
| IDLE | 100% | 100% | N.A. |
| FILLING | > 0% | N.A. | N.A. |
| READY | N.A. | > 0% | N. A. |
| DISPENSE | N.A. | > 0% | if Tank State is READY and dispense button is held |
| FAULT | N.A | N.A. | when any of the faults are true |

### Pump States
| Pump State | Condition |
|-|-|
| ON | when the filtered water tank has water |
| OFF | when the filtered water tank has no water or Tank State is FAULT |

### Fault States
| Fault State | Condition |
|-|-|
| NO_FAULT | when system is normal |
| OVERFLOW | when anytank is overflowing |
| INPUT_EMPTY | when there is no water for the pump to suck |
| ACIDIC | when the output tank ph is less than 4ph |
| BASIC | when the output tank ph is greater than 9ph |
