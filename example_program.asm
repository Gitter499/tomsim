# Example program: Februray 15, 2026
# this program outputs the value 31.
# the assembly is:
LIZ $R0,31
LIZ $R3,4
# ADD $R0,$R3,$R0 # Line is commented!
SW  $R0, $R3
LW  $R3, $R3
PUT $R3
HALT
