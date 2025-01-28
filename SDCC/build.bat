@echo off
echo compiling...
sdcc -c main.c
sdcc -c uart12.c
sdcc -c watchdog.c

echo linking...
sdcc main.c watchdog.rel uart12.rel

echo generating HEX file...
packihx main.ihx > send.hex

echo cleaning up...
del *.asm
del *.ihx
del *.lk
del *.lst
del *.map
del *.mem
del *.rel
del *.rst
del *.sym

pause