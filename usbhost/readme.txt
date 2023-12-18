        Picosynth prototype 1

Connector 1 with red side connected to bottom left expander.
Connector 2, black (ground) connected to middle row right most (gnd)
Connector 2, pink / blue connected to picoprobe GP4 / 5

SWD - looking at SWD end
SWD - middle to picoprobe gnd.
SWD - left to picoprobe GP2
SWD - right to picoprobe GP3


HID_ITF_PROTOCOL_NONE: 00 00 86 7F 7F 1F 00 C0
default report: 00 00 87 7F 7F 1F 00 C0
default report: 00 00 85 7F 7F 1F 00 C0 
default report: 7F 7F 85 7F 7F 0F 00 C0 
default report: 7F 7F 87 7F 7F 0F 00 C0 
default report: 7F 7F 85 7F 7F 0F 00 C0 
default report: 7F 7F 86 7F 7F 0F 00 C0 
default report: 7F 7F 85 7F 7F 0F 00 C0 
default report: 7F 7F 86 7F 7F 0F 00 C0 
                xx yy zz rz ry rz 


default report: 01 00 00 00 08 00 7F 7F 7F 7F 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 03 EF 14 00 00 00 00 23 FF 77 01 81 02 00 02 00 01 A0 02 00 
default report: 01 00 00 00 08 00 7F 7F 7F 7F 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 03 EF 14 00 00 00 00 23 FF 77 01 81 02 00 02 00 01 A0 02 00 


01 00 00 00 08 00 7F 7F 7F 7F 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 03 EF 14 
00 00 00 00 23 FF 77 01 81 02 00 02 00 01 A0 02 
00

0x31 = 

keyboard
05 01 09 80 A1 01 85 02 09 81 09 82 09 83 25 01 
15 00 75 01 95 03 81 06 75 05 95 01 81 01 C0 05 
0C 09 01 A1 01 85 03 15 00 25 01 19 B5 29 B7 09 
CD 09 E2 09 E5 09 E7 0A 83 01 09 E9 09 EA 0A 52 
01 0A 53 01 0A 54 01 0A 55 01 0A 8A 01 0A 92 01 
0A 94 01 0A 21 02 1A 23 02 2A 27 02 0A 2A 02 95 
18 75 01 81 02 C0 

blaze joypad
05 01 09 04 A1 01 A1 02 85 01 75 08 95 01 15 00 
26 FF 00 81 03 75 01 95 0D 15 00 25 01 35 00 45 
01 05 09 19 01 29 0D 81 02 75 01 95 03 06 00 FF 
81 03 05 01 25 07 46 3B 01 75 04 95 01 65 14 09 
39 81 42 65 00 75 01 95 0C 06 00 FF 81 03 15 00 
26 FF 00 05 01 09 01 A1 00 75 08 95 04 15 00 15 
00 15 00 35 00 35 00 46 FF 00 09 30 09 31 09 32 
09 35 81 02 C0 05 01 75 08 95 27 09 01 81 02 75 
08 95 30 09 01 91 02 75 08 95 30 09 01 B1 02 C0 
A1 02 85 02 75 08 95 30 09 01 B1 02 C0 A1 02 85 
EE 75 08 95 30 09 01 B1 02 C0 A1 02 85 EF 75 08 
95 30 09 01 B1 02 C0 C0 

kxd joypad
05 01 09 04 A1 01 A1 02 75 08 95 05 15 00 26 FF 
00 35 00 46 FF 00 09 30 09 31 09 32 09 32 09 35 
81 02 75 04 95 01 25 07 46 3B 01 65 14 09 39 81 
42 65 00 75 01 95 0C 25 01 45 01 05 09 19 01 29 
0C 81 02 06 00 FF 75 01 95 08 25 01 45 01 09 01 
81 02 C0 A1 02 75 08 95 07 46 FF 00 26 FF 00 09 
02 91 02 C0 C0 

orb joypad
05 01 09 04 A1 01 A1 02 85 01 75 08 95 01 15 00 
26 FF 00 81 03 75 01 95 13 15 00 25 01 35 00 45 
01 05 09 19 01 29 13 81 02 75 01 95 0D 06 00 FF 
81 03 15 00 26 FF 00 05 01 09 01 A1 00 75 08 95 
04 35 00 46 FF 00 09 30 09 31 09 32 09 35 81 02 
C0 05 01 75 08 95 27 09 01 81 02 75 08 95 30 09 
01 91 02 75 08 95 30 09 01 B1 02 C0 A1 02 85 02 
75 08 95 30 09 01 B1 02 C0 A1 02 85 EE 75 08 95 
30 09 01 B1 02 C0 A1 02 85 EF 75 08 95 30 09 01 
B1 02 C0 C0 



Copyrights
----------

https://github.com/No0ne/ps2x2pico
/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 No0ne (https://github.com/No0ne)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

Flash Chips
===========
W25Q16JVUXIQ - normal one with pico
W25Q128JVSQ

