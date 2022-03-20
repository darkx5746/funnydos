@bcc -ml -I..\include pcx2img.c common.c ..\lib\*.lib
@bcc -ml -I..\include keytest.c common.c ..\lib\*.lib
@bcc -ml -I..\include rol2amd.c common.c ..\lib\*.lib
@bcc bin2.c
@dir *.exe