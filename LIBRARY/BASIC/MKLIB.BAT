@bcc -c -ml -I..\..\include *.c
@del basic.lib
@tlib basic.lib + KEYINT+ MOUSE+ TIMER
@dir basic.lib