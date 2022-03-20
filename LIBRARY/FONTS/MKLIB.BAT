@bgiobj /F english.fnt efont _engfont efont_TEXT
@bgiobj /F hangulg.fnt kfont _korfont kfont_TEXT
@del  fonts.lib
@tlib fonts.lib +EFONT+ KFONT
@dir  fonts.lib