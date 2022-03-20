@bcc -c -ml -I..\..\include *.c
@del vgapm.lib
@tlib vgapm.lib +BOXLIB+ PMHAN+PMODE+PMPCX+PMPRINTF+PMIMAGE+PMPAGE+PMPIXEL+VGASPR+PMCIRPAI+PMLINEBO
@dir vgapm.lib