# A script that converts a PNG into an array of pixels on the default VGA Palette (requires pillow and numpy)
#@todo finsish implementing the rest of the VGAPallete
import sys
import numpy
from PIL import Image

VGAPallete = [[0,0,0],[0,0,168],[0,168,0],[0,168,168],[168,0,0],[168,0,168],[168,87,0],[168,168,168],[87,87,87],[87,87,255],[87,255,87],[87,255,255],[255,87,87],[255,87,255],[255,255,87],[255,255,255],[0,0,0],[23,23,23],[32,32,32],[47,47,47],[56,56,56],[71,71,71],[80,80,80],[96,96,96],[112,112,112],[128,128,128],[144,144,144],[160,160,160],[183,183,183],[200,200,200],[224,224,224],[255,255,255]]

def main(imgPath):
    img = Image.open(imgPath,'r')
    img = img.convert("RGB")
    width, height = img.size
    pixel_values = list(img.getdata())
    channels = 3
    pixel_values = numpy.array(pixel_values).reshape((width, height, channels))
    VGAArray = numpy.array(VGAPallete)
    result_array = []
    for x in pixel_values:
        for colors in x:
            dists = numpy.sqrt(numpy.sum((VGAArray-colors)**2,axis=1))
            index_of_smallest = numpy.where(dists==numpy.amin(dists))
            smallest_dist = VGAArray[index_of_smallest]
            result_array.append(index_of_smallest[0][0])
    
    out_string = "const u8 data[] = {"
    for index, value in enumerate(result_array):
        out_string = out_string + str(value) + ","
        if index % 50 == 0:
            out_string = out_string + '\n'

    out_string = out_string[:-1]+"};"
    out_file = open(imgPath+".c","w")
    out_file.write(out_string)


if __name__ == "__main__":
    if len(sys.argv) == 1:
        print("No image provided!")
    else:
        main(sys.argv[1])