from ifaint import *
#start
def set_red_pixel():
    x, y = get_mouse_pos() # Get parameters
    set_pixel(x,y,(255,0,0))

bindc('r', set_red_pixel, mod.shift) # Set red pixel under mouse with Shift+R
