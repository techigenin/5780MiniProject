The line under properties->toolchain->Misc. must be updated to this for the project to flash properly...
-Tsamd21g18a_flash.ld  -Wl,--section-start=.text=0x2000

Run this line in a command line window to make the project run.
You'll have to update the arduino install location and project .bin location
C:\Users\techi\AppData\Local\Arduino15\packages\arduino\tools\bossac\1.7.0/bossac -i -d --port=COM10 -U true -i -e -w -v "D:\Users\techi\Desktop\Basic Project\Basic Project\Basic Project\Debug\kiddyPiano2OSC.bin" -R 

Also, you will have to figure out what port the Feather is on.
Press the feathers button twice quickly to go into progrmaming mode.  The LED will fade up and down slowly.
