from ifaint import *
import os

lipsum = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."

def create_image():
    IMAGE_WIDTH = 640
    IMAGE_HEIGHT = 480
    img = Canvas(IMAGE_WIDTH, IMAGE_HEIGHT)

    text = img.Text((10, 10, 100, 1000), lipsum)
    assert text.get_text_raw() == lipsum
    return img

def in_out_dir(file_name):
    return os.path.join(os.getcwd(), 'out', file_name)

def save(img):
    # Save as a png for comparison
    img.save_backup(in_out_dir('test-save-svg-text.png'))

    # Save as SVG and load again
    file_name = in_out_dir('test-save-svg-text.svg')
    img.save_backup(file_name)
    img2 = Canvas(file_name)
    assert img2.get_objects()[0].get_text_raw() == lipsum

save(create_image())
