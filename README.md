# IBM Wheelwriter Bus Sender
Uses a Dallas Semiconductor [DS89C440](https://www.maximintegrated.com/en/products/microcontrollers/DS89C440.html) MCU (an Intel 8052-compatible microcontroller) to send a hex number to the Wheelwriter's serial BUS. Useful for sussing out the Wheelwriter's behavior to commands.

This project only works on earlier Wheelwriter models, the ones that internally have two circuit boards: the Function Board and the Printer Board (Wheelwriter models 3, 5 and 6).

The MCU connects to the J1P "Feature" connector on the Wheelwriter's Printer Board. See the schematic for details.
<p align="center"><img src="Wheelwriter%20Interface.jpg"/>
<p align="center">Wheelwriter Interface</p><br>
