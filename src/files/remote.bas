5 PRINT "Remote Control Demo Program"
10 I = JOY
20 IF I = 255 GOTO 10
25 PRINT I
30 IF I = 254 DRIVE(5)
40 IF I = 253 DRIVE(15)
50 IF I = 251 DRIVE(7)
60 IF I = 247 DRIVE(13)
70 IF I = 239 CLAW(0)
80 IF I = 223 CLAW(100)
1000 IF JOY <> 255 GOTO 1000
1005 DRIVE(0)
1010 GOTO 10
