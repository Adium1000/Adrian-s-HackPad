# Adrian-s-HackPad
This project is a custom hackpad / macro pad based on the Seeed XIAO RP2040, designed for productivity and media control on Windows.
It features dedicated macro keys, a rotary encoder for volume control, and addressable RGB LEDs for visual feedback.
# 🖼️ Hackpad – Overall View
📷 Screenshot: Overall Hackpad

This hackpad includes:

5 macro keys

1 rotary encoder with push button

8 addressable RGB LEDs

Compact form factor powered via USB

#🔌 Schematic

📷 Screenshot: Schematic



Main components:

MCU: Seeed XIAO RP2040

Keys: Connected directly to GPIO pins

Rotary Encoder:

A → GPIO 3

B → GPIO 2

Button → GPIO 4

RGB LEDs (WS2812):

Data → GPIO 7

All components are powered from USB (5V), with proper decoupling and data line protection.

🧩 PCB

📷 Screenshot: PCB Layout

The PCB was designed to be:

Single-board, compact layout

Easy to hand-solder

Optimized for small enclosures

Compatible with the Seeed XIAO RP2040 footprint

# 🧱 Case & Assembly

📷 Screenshot: Case and Assembly



The case is designed to:

Secure the PCB with screws

Hold the rotary encoder firmly

Allow RGB light diffusion

Expose the USB port cleanly

Assembly order:

Solder components on PCB

Mount PCB into bottom case

Attach encoder knob

Close case with top cover

# ⌨️ Firmware & Key Functions

Keys

Win + D → Show Desktop

Ctrl + Shift + Esc → Task Manager

Win + E → File Explorer

Win + L → Lock Windows

Alt + F4 → Close active window

Encoder

Rotate clockwise → Volume Up

Rotate counter-clockwise → Volume Down

Press encoder → Mute

# 📦 Bill of Materials (BOM)
Qty	Component	Description
1	Seeed XIAO RP2040	Microcontroller board
5	Mechanical switches	MX-style or compatible
1	Rotary encoder	With push button
8	WS2812B LEDs	Addressable RGB LEDs
1	PCB	Custom designed
1	Case	3D printed or CNC
1	Encoder knob	Any compatible knob
5	Keycaps	1u keycaps
1	USB cable	USB-C (or as needed)
