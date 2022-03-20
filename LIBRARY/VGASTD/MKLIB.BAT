@bcc -c -ml -I..\..\include *.c
@del vgastd.lib
@tlib vgastd.lib +CIRPAINT+LINEBOX+PEFFECT+PRINTF+STDPCX+IMAGE+PALETTE+PIXEL+STDHAN+VMODE
@dir *.lib