# OneForAll
All in one software for the Retro PSU and other handheld projects.

This OSD (overhead screen display) is designed to be fast and informative, it uses low level gpu system calls as well as signals and interrupts to keep the resources consuption low. The software is intendeed for mobile application as it features:

    battery meter (w\ charge indicator)
    wifi meter w\ On/Off
    Bluetooth On/Off
    Analog Stick (w\ Disable/Enable Button Combo)
    Built In Controls (no need for RetroGame from Adafruit, edit the keys.cfg)
    Control Volume with Button Combo
    

<b>Build instructions:</b>
Open up the terminal using SSH and run each command below:<br>

git clone --recursive https://github.com/Helder1981/OneForAll.git<br>
sudo chmod 777 /home/pi/OneForAll/install.sh<br>
sudo /home/pi/OneForAll/./install.sh<br>
<br><br><br>
Now edit the keys.cfg to match your button wiring, I have included a GPIO Pinout jpeg that has the default wiring I used<br>
![GPIO](/Raspberry-Pi-GPIO-Pinout.jpg)<br><br><br><br>

Super big thanks to <b>Matthew English-Gallantry</b> aka <b>Bluup</b> from the awesome <b>Pocket Pi FE</b> project, he has worked really hard to get this working for everyone to use with the Retro PSU and their own projects.
<br><br><br><br>
Basic Install <br>
![BasicInstall](/RetroPSU%20Basic%20Install.jpg)<br>
Analog Stick Pinout to the Retro PSU board:<br>
![AnalogPinout](/Analog%20Stick%20Pinout.jpg)<br>
![demo2](/On%20Screen%20Display.jpg)<br>
