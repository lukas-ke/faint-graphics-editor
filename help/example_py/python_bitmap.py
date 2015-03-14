from ifaint import *

#start
# Create a 320 by 200 bitmap
bmp = Bitmap(320, 200)

# Fill the bitmap with magenta
for x in range(320):
  for y in range(200):
    bmp.set_pixel(x,y,(255,0,255))

# Draw the bitmap at x,y=10,10 in the active canvas
get_active_image().blit((10,10), bmp);
