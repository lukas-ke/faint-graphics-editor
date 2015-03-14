from ifaint import add_format, Settings

#start
def load_silly(file_path, image_props):
    frame_props = image_props.add_frame(640,480)
    frame_props.Text((0,0,400,100), file_path, Settings())

def save_silly(file_path, canvas):
    f = open(file_path, 'w')

    for object in canvas.get_objects():
        f.write(str(object) + "\n")

add_format(load_silly, save_silly, "Silly format", "silly")
