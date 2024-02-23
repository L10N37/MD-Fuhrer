Probably better to use 16F627. Cheaper

or port across to 16F54, cheaper again. If a PCB is made
would likely use 16F54 SSOP (smaller footprint, not thru hole)

I dont see it being much that useful these days but
R type is a good SMS game and playing a legit copy without 
doing the 'swap' trick is nice


There are mistakes on the main.c, but it compiles, and it works. It was late, I was tired.

The original reference code used sets PORTB to 0x63 in the last byte, 01100011 - and to 0x7C on the byte prior: 01111100 and 0x70 the byte prior again, 01110000
These all have bit 6 set, but bit 6 is PORTB6 and that's an oscillator pin and not used for the chips function. 


I do have a genuine megadrive master system adapter, I could hook an LA to it and power it up with a bench supply and check the capture.. but... it works... maybe by fluke :P
