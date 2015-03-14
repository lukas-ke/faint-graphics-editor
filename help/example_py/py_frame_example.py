from ifaint import *

#start
# Retrieving a frame from an image
image = get_active_image()
frame = image.get_frame(3)
frame.line(0,0,100,100)
