# Adrian's-HackPad
This is a custom macro pad based on the Seeed XIAO RP2040, It features dedicated macro keys, a rotary encoder for volume control, and addressable RGB LEDs for visual feedback.

# Overall View
![Logo](Presentation-Readme/product.png)

This hackpad includes:

- 5 macro keys
- 1 rotary encoder
- 8 addressable RGB LEDs

#  Schematic

![Logo](Presentation-Readme/Schematich.png)


**Components:**

- MCU: Seeed XIAO RP2040
- Keys: Connected directly to GPIO pins
- Rotary Encoder:
- RGB LEDs

# PCB

![Logo](Presentation-Readme/PCB.png)

The PCB was designed to be single-board, compact layout

# Case & Assembly
![Logo](Presentation-Readme/3D.png)

The case is designed to:
- Secure the PCB with screws
- Allow RGB light diffusion
- Expose the USB port cleanly

# Firmware & Key Functions
**Keys:**

- <kbd>Win</kbd> + <kbd>D</kbd> → Show Desktop
- <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>Esc</kbd> → Task Manager
- <kbd>Win</kbd> + <kbd>E</kbd> → File Explorer
- <kbd>Win</kbd> + <kbd>L</kbd> → Lock Windows
- <kbd>Alt</kbd> + <kbd>F4</kbd> → Close active window

**Encoder:**
- Rotate clockwise → Volume Up
- Rotate counter-clockwise → Volume Down
- Press encoder → Mute

# Bill of Materials (BOM)
| Quantity | Component           | Description            |
|:--------:|:-------------------:|:----------------------:|
| 1        | Seeed XIAO RP2040   | Microcontroller board  |
| 5        | Mechanical switches | MX-style or compatible |
| 1        | Rotary encoder      | With push button       | 
| 8        | WS2812B LEDs        | Addressable RGB LEDs   |
| 1        | PCB                 | Custom designed        |
| 1        | Case                | 3D printed or CNC      |
| 1        | Encoder knob        | Any compatible knob    |
| 5        | Keycaps             | 1u keycaps             |
| 1        | USB cable           | USB-C (or as needed)   |