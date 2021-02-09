# EspressoControl
An arduino-based espresso temperature controller based on a modified PD algorithm.

<img src="./interface.jpg" alt="Espresso machine with temperature controller interface" width="352" height="553">

The temperature controller uses a PLT temperature sensor to measure the temperature of the espresso machine boiler, and a relay board to switch the heating element on and off.
Measurements from the temperature sensor are fed into a modified propotional-derivative algorithm. The resulting control variable is then used to control heating of the boiler via low-frequency pulse width modulation of the heating element.

The system also features a small screen to display the current boiler temperature, and two buttons. Currently the left button is used to initiate a brew cycle while allowing the control algorithm to compensate for the disturbance this creates. Future functionality may include handling the steam function in the same interface and allowing the user to adjust the setpoint of the temperature controller. As shown in the graph below, this system greatly reduces overshoot in the turn on transient and has the machine settled and ready to brew over 5 minutes sooner.

![The temperature control algorithm avoids overshoot and allows the boiler to settle to an appropriate brew temperature much more quickly](./startup_transient.png)
