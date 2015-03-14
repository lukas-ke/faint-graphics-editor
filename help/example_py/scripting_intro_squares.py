from ifaint import line
#start
for i in range(100):
    line( (0, i * 10), (640, i * 10) ) # Horizontal lines
    line( (i * 10, 0), (i * 10, 480) ) # Vertical lines
