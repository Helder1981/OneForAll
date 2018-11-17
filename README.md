
# OneForAll
All in one software for the Retro PSU and other handheld projects.

This OSD (on screen display) is designed to be fast and informative, it uses low level gpu system calls as well as signals and interrupts to keep the resources consuption low. The software is intendeed for mobile application as it features:

 - Battery meter.
 - Wifi connectivity indicator. Can be toggled On/Off.
 - Bluetooth connectivity indicator. Can be toggled On/Off.
 - Analog Stick. Can be toggled On/Off.
 - Built In GPIO Controls (no need for RetroGame from Adafruit, edit the keys.cfg).
 - Control Volume with Button Combo.
 - Overlay that shows all hotkey combinations.
    

### Build instructions:
Open up the terminal using SSH and run each command below:<br>

```sh
$ git clone --recursive https://github.com/Helder1981/OneForAll.git<
$ sudo chmod 755 /home/pi/OneForAll/install.sh
$ sudo /home/pi/OneForAll/./install.sh
```

Now edit the keys.cfg to match your button wiring, I have included a GPIO Pinout jpeg that has the default wiring I used<br>

![GPIO](/Raspberry-Pi-GPIO-Pinout.jpg)<br><br><br><br>

### Configuration:

One For All can be configured through keys.cfg through setting various flags:

| Flag | Description |
| ------ | ------ |
| SHUTDOWN_DETECT | GPIO Pin that when driven low will cause the system to shutdown. Set to -1 if not required. |
| [KEYS] | Configure which GPIOs relate to which buttons. |
| [JOYSTICK] DISABLED| If True, completely turns off Joystick. If False, joystick can be toggled on and off when needed using a hotkey combo. |
| [JOYSTICK] DEADZONE | This is the voltage range that is ignored by the software. When a joystick moves, it generates a voltage. This deadzone stops movement commands being sent form slight presses of the joystick. It is the joystick sensitivity. |
| [JOYSTICK] VCC | This is the voltage change from furthest left to furthest right. Change this if not using 5V as an input to the Joystick. Does not need to be changed when using the Retro PSU Board. |
| [BATTERY] ENABLED | If True, enables battery monitoring on the RetroPSU Board. Set to false if not using a battery monitor |
| [BATTERY] ENABLED | If True, enables battery monitoring on the RetroPSU Board. Set to false if not using a battery monitor |
| [BATTERY] FULL_BATT_VOLTAGE | Voltage when the battery is fully charged. In centivolts e.g. 3.2 Volts is 320. |
| [BATTERY] BATT_LOW_VOLTAGE | Voltage when the battery is considered low. Shows as red in battery meter. In centivolts e.g. 3.2 Volts is 320. |
| [BATTERY] BATT_SHUTDOWN_VOLT | Voltage when safe shutdown is initiated. Shows as red in battery meter. In centivolts e.g. 3.2 Volts is 320. |

### Basic Install:
![BasicInstall](/RetroPSU%20Basic%20Install.jpg)<br><br><br>
<b>Analog Stick Pinout to the Retro PSU board:</b><br>
![AnalogPinout](/Analog%20Stick%20Pinout.jpg)<br><br><br>
<b>Software Installed:</b><br>
![demo2](/On%20Screen%20Display.jpg)<br>


Super big thanks to <b>Matthew English-Gallantry</b> aka <b>Bluup</b> from the awesome <b>Pocket Pi FE</b> project, he has worked really hard to get this working for everyone to use with the Retro PSU and their own projects.
<br><br><br><br>
