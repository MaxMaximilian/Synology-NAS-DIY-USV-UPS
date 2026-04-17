# Synology-NAS-DIY-USV-UPS

## Safety Notice

This project involves batteries and power electronics.
Improper handling may cause damage, fire, or injury.
Use at your own risk.

## Security Notes

- The shutdown server uses a simple token-based authentication
- No encryption (HTTP only)
- Intended for local network use only

## Why

My initial idea was to use an existing UPS as a reliable power source for my Synology NAS.  
The main problem was the lack of efficient, long-lasting, and affordable options (at least in my opinion).

- Most commercially available UPS systems convert 230V AC to 12V DC and then back to 230V AC. At least it seems that way, judging by the heat buildup over time—even without load. This is extremely inefficient. It gets even worse when converting the 230V AC back to 12V DC for the NAS.
  - 230V → 12V → 230V → 12V
- Most commercially available UPS systems use a 12V lead-acid battery as an emergency power source.
  - These can last a while, but they need to be replaced eventually.
  - Some newer models use Li-Ion or LiPo batteries, which involve a risk of fire, especially in 24/7 operation.

## My Initial Targets

- Use a LiFePO₄ battery
  - Easy to obtain
  - Lightweight
  - Safe
- Avoid unnecessary voltage conversions
  - Saves power
- Automatic shutdown of the NAS in case of an emergency
- Get notified when something is not working correctly
- No hardware modifications to the NAS
- No software modifications to the NAS operating system
- Keep it simple
- Keep it cheap (which turned out to be harder than expected)

## First Ideas (which did not work out)

### Power System

- It might be possible to use the original power supply to power the NAS and trickle-charge a battery. In case of a power outage, a buck-boost converter could maintain a stable 12V supply.
  - After some consideration, it became clear that this solution would be unreliable at best and not suitable for 24/7 operation.

### Control System

- Most commercial UPS systems use USB or LAN to provide information such as state of charge (SOC) and shutdown signals to the NAS. I thought it should be possible to replicate this functionality with an Arduino.
  - After some research, I discovered the NUT project, which seemed perfect for my use case.

  https://networkupstools.org/  
  https://github.com/networkupstools/nut

- First, I tried the USB approach from this project:

  https://github.com/networkupstools/nut/wiki/DIY-UPS-with-and-Arduino

  After a few minutes, I had an Arduino running and reporting battery information to my Windows and Linux PCs. Unfortunately (as mentioned in the project documentation), Synology systems are too restricted to accept an Arduino as a NUT device—even after some hacking attempts with support from ChatGPT.

- The next idea was to run a NUT server using an Arduino with a network shield.
  - Unfortunately, I learned that this is also not practical, as the Arduino lacks sufficient computing power. This would require something like a Raspberry Pi, which is too expensive for my low-cost approach.

## Actual Solution

After some online research, I found a good starting point for the *power system*: the "USV 1228-12 Module".  
This is a 100W low-voltage UPS system with low power consumption and all the necessary features for my application.

The only remaining challenge was the *control system* for shutting down the NAS during a power outage.  
ChatGPT suggested using a simple Python web server running on the NAS that can trigger a shutdown command. In this setup, the Arduino only needs to call a specific URL to shut down the NAS.

# Hardware Requirements

- USV 1228-12 Module  
  https://www.google.com/search?udm=2&q=USV+1228-12

- Arduino Pro Micro (ATmega32U4)
- W5500 Ethernet module
- LiFePO₄ battery (~12–14V, e.g., 8Ah)  
  https://amzn.eu/d/0g9cFo4b
- Buck converter as power supply for the Arduino  
  (e.g., MP1584)
- ~19V power supply (>80W)  
  (e.g., old laptop power supply)
- Passive buzzer
- 2 LEDs
- Synology power plug  
  (shown in "images/synology power plug.png")
- Additional connectors (e.g., XT60)
- Resistors for a voltage divider

# Arduino

## Software

The Arduino software is developed in Visual Studio Code using the PlatformIO plugin.

## Wiring

just look it up in the Schematic `hardware\pcb`

## PCB

I have milled the PCB using a 3018CNC using FlatCam.
All used Files are in the `hardware\pcb` Folder.
**Use Them at your one Risk**

# Case

  I 3D printed a custom Case for the whole Setup.
  All used Files are in the `hardware\case` Folder.

## NAS

- The Synology NAS script is written in Python.
- Python must be installed on the NAS.
  - This can be done via the built-in App Store (Package Center).
- To ensure the script runs continuously, a scheduled task (Task Scheduler) is configured.
  - Settings can be found in the `software\python` folder.
- My Nas has 2 Network Ports. One is used for normal acces, the other one communicates with the "USV". The USV Port got a fixed IP Assigned via NAS Webinterface.
  - **Keep in mind: Don't use a Swich between the NAS and USV if its not powerd via the USV!**

# Limitations

- No battery SOC reporting
- No encrypted communication
- Requires static IP setup

# Improvement Ideas

- Add a button to acknowledge an error and silence the buzzer