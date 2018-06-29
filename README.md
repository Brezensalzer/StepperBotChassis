# StepperBotChassis

This is the chassis of a two wheeled rover with stepper motors for 
precision movement. It is part of a larger project. The whole rover is 
"layered" in three slices:

- the chassis layer: https://github.com/Brezensalzer/StepperBotChassis
- the control layer: https://github.com/Brezensalzer/StepperBotController
- the sensor layer: https://github.com/Brezensalzer/StepperBotLidar2
- the ground station code: https://github.com/Brezensalzer/StepperBotGroundStation

The Arduino code is for a Teensy LC but should be easily adaptable to 
other microcontroller boards. Openscad, FreeCAD drawings and STL files 
are provided for 3D printing.

The chassis works great on a smooth floor (parquet) but on the seams of 
floor tiles it is easily pushed off track because of the small ball 
caster.

To complete the mechanical build, the following parts are needed:

- 2x Pololu DRV8834 Low-Voltage Stepper Motor Driver Carrier
- 2x Stepper motor 11HS12-0674S
- 2x O-rings 60 x 5 mm
- 2x 18650 LiPo cell
- LiPo protection circuit
- LiPo charging module
- Pololu ball caster 3/8" plastic ball
- Pololu 3.3V step-up/step-down voltage regulator S7V8F3
- Teensy LC
- Toggle switch 3-pin
- M2.5 screws for the stepper mounting

<p align="center">
  <img src="./StepperBotChassis.jpg" width="400"/>
</p>
