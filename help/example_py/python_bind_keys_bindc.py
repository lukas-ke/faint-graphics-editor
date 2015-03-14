from ifaint import *

#start
bindc('h',invert) # Binds the invert function to the H-key
bindc('h',desaturate,mod.ctrl) # Binds desaturate to Ctrl+H
bindc('h',context_flip_horizontal,mod.ctrl|mod.shift) # Binds context_flip_horizontal to Ctrl+Shift+H
