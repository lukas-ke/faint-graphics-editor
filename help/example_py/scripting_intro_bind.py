from ifaint import *
#start
# A custom function that changes tool settings
def example_function():
    tool_line()
    set_linewidth(10.0)
    set_fg(0,0,255)

# Connecting the function to a key
bind(example_function)
##[press key]
