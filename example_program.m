# Example program: Februray 15, 2026
# this program outputs the value 31.
# the assembly is:
801F	# LIZ $R0,31
8304	# LIZ $R3,4
#0060 # ADD $R0,$R3,$R0 # Line is commented!
4860	# SW  $R0, $R3
4360	# LW  $R3, $R3
7060	# PUT $R3
6800	# HALT
