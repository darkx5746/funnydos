@bcc -c -ml -I..\..\include *.c
@del amd.lib
@tlib amd.lib +ADLIB+CONVR2A+LOADINST+AMDMUSIC+LOADBANK+LOADROL
@dir amd.lib