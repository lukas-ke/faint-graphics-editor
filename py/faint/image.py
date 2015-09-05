import ifaint
from ifaint import Shape

class FakeGrid:
    anchor = 0, 0
    color = 255, 255, 255
    dashed = False
    enabled = False
    spacing = 40

class Pimage:
    """Experimental Image implementation in Python.

    If this works out, I'll rename it as Image, and remove the
    C++-implementation py-image.cpp.

    """

    def __init__(self, w, h):
        self.w = w
        self.h = h
        self.objects = []

    def Rect(self, *args):
        sh = ifaint.create_Rect(*args)
        self.objects.append(sh)
        return sh

    def Ellipse(self, *args):
        sh = ifaint.create_Ellipse(*args)
        self.objects.append(sh)
        return sh

    def Polygon(self, *args):
        sh = ifaint.create_Polygon(*args)
        self.objects.append(sh)
        return sh

    def Raster(self, *args):
        sh = ifaint.create_Raster(*args)
        self.objects.append(sh)
        return sh

    def Spline(self, *args):
        sh = ifaint.create_Spline(*args)
        self.objects.append(sh)
        return sh

    def Text(self, *args):
        sh = ifaint.create_Text(*args)
        self.objects.append(sh)
        return sh

    def get_obj_text_height(self, *args):
        return 10.0 # Fixme

    def Group(self, *args):
        sh = ifaint.create_Group(*args)
        self.objects.append(sh)
        return sh

    def Line(self, *args):
        sh = ifaint.create_Line(*args)
        self.objects.append(sh)
        return sh

    def Path(self, *args):
        sh = ifaint.create_Path(*args)
        self.objects.append(sh)
        return sh

    def get_obj_tri(self, obj):
        """For compatibility with FrameProps"""
        return obj.get_tri()

    def set_obj_tri(self, obj, tri):
        """For compatibility with FrameProps"""
        obj.set_tri(tri)

    def get_size(self):
        return self.w, self.h

    def get_pixel(self, x, y):
        return (255, 255, 255) # Fixme

    def get_objects(self):
        return self.objects

    def _one_color_bg(self):
        return True # Fixme

    def add_warning(self, w):
        """For compatibility with FrameProps"""
        pass

    def save_backup(self, path):
        ifaint.write_png(self.background, path)

    background = None

class PimageList:
    """List of frames in Python."""

    def __init__(self):
        self.frames = []

    def add_frame(self, size):
        frame = Pimage(size[0], size[1])
        self.frames.append(frame)
        return frame

    def get_size(self):
        return self.frames[0].get_size()

    def get_frames(self):
        return self.frames

    def get_frame(self, num=0):
        return self.frames[num]

    def _one_color_bg(self):
        return True # Fixme

    def get_pixel(self, *args):
        return self.frames[0].get_pixel(*args)

    def get_objects(self):
        return self.frames[0].get_objects()

    def get_calibration(self):
        return None

    grid = FakeGrid()


def one_color_bg(obj):
    try:
        return obj._one_color_bg()
    except AttributeError:
        return ifaint._one_color_bg(obj)
