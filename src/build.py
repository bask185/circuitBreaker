#!/usr/bin/env python
import os
#print('ASSEMBLING IO FILES')
#os.system("updateIO.py")
print('ADDING TIME STAMP')
os.system("addDate.py")
print('BUILDING PROJECT')
# os.system('arduino-cli compile -b attiny:avr:ATtinyX5 -e')  # change x in 4? FOR ATTINY
os.system('arduino-cli compile -b arduino:avr:nano -e')  # change x in 4?	FOR NANO
exit