import os
import struct
import sys

VERSION = "2.0"

from jpt import Jpt

def process():
	print "-------------------------------------------------------------------------------"
	print "| April JPT Tool " + VERSION
	print "| JPT format version: " + str(Jpt.Version)
	print "-------------------------------------------------------------------------------"
	if len(sys.argv) < 2:
		info()
	elif sys.argv[1].lower() in ("help", "-h", "/h", "-?", "/?"):
		help()
	elif sys.argv[1].lower() == "merge":
		if len(sys.argv) != 5:
			info()
			return
		merge(sys.argv[2:len(sys.argv)])
	elif sys.argv[1].lower() == "split":
		if len(sys.argv) != 5:
			info()
			return
		split(sys.argv[2:len(sys.argv)])
	elif sys.argv[1].lower() == "prepare":
		if checkPIL():
			if len(sys.argv) != 5 and len(sys.argv) != 6:
				info()
				return
			prepare(sys.argv[2:len(sys.argv)])
	elif sys.argv[1].lower() == "convert":
		if checkPIL():
			if len(sys.argv) != 4 and len(sys.argv) != 5:
				info()
				return
			convert(sys.argv[2:len(sys.argv)])
	else:
		info()

def merge(args):
	print Jpt.merge(args[0], args[1], args[2])
	
def split(args):
	print Jpt.split(args[0], args[1], args[2])

def prepare(args):
	if checkPIL():
		quality = 95
		if len(args) == 4:
			quality = int(args[3])
		from pilconv import PilConv
		print PilConv.convert(args[0], args[1], args[2], quality)

def convert(args):
	if checkPIL():
		image = args.pop(1)
		jpeg = image + "__tmp__.jpg"
		png = image + "__tmp__.png"
		args.insert(1, jpeg)
		args.insert(2, png)
		prepare(args)
		merge([image, jpeg, png])
		os.remove(jpeg)
		os.remove(png)
		
def checkPIL():
	try:
		import Image
	except:
		print "ERROR! Please install PIL to use the 'convert' command."
		print "http://www.pythonware.com/products/pil"
		return False
	return True

def info():
	print ""
	print "usage: jpt-tool.py merge JPT_FILENAME JPEG_FILENAME PNG_FILNAME"
	print "       jpt-tool.py split JPT_FILENAME JPEG_FILENAME PNG_FILNAME"
	print "       jpt-tool.py prepare FILENAME JPEG_FILENAME PNG_FILNAME [JPEG_QUALITY]"
	print "       jpt-tool.py convert FILENAME JPT_FILENAME [JPEG_QUALITY]"
	print "       use 'jpt-tool.py -h' for more information"
	print ""
	if os.name != 'posix':
		os.system("pause")

def help():
	print ""
	print "usage: jpt-tool.py merge JPT_FILENAME JPEG_FILENAME PNG_FILNAME"
	print "       jpt-tool.py split JPT_FILENAME JPEG_FILENAME PNG_FILNAME"
	print "       jpt-tool.py prepare FILENAME JPEG_FILENAME PNG_FILNAME [JPEG_QUALITY]"
	print "       jpt-tool.py convert FILENAME JPT_FILENAME [JPEG_QUALITY]"
	print ""
	print "commands:"
	print "merge            - merges a JPEG and a PNG file into a JPT file"
	print "split            - splits a JPT file to a JPEG and a PNG file"
	print "prepare          - creates PNG and JPEG images from an image file (requires PIL installed)"
	print "convert          - creates a JPT image from an image file direclty using 'prepare' (requires PIL installed)"
	print ""
	print "JPT_FILENAME     - JPT filename to use in the process"
	print "JPEG_FILENAME    - JPEG filename to use in the process"
	print "PNG_FILENAME     - PNG filename to use in the process"
	print "FILENAME         - image filename (any format) to use in the process"
	print "JPEG_QUALITY     - value from 1 (worst) to 95 (best) for JPEG compression quality (default = 95); values above 95 should be avoided, 100 completely disables the quantization stage"
	print ""
	os.system("pause")

process()
