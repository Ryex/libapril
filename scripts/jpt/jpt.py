import os
import struct
import sys

UINT_PACK_FORMAT = "<I" # little-endian unsigned int

def _read_file(filename):
	f = open(filename, "rb")
	data = f.read()
	f.close()
	return data

def _write_file(filename, data):
	f = open(filename, "wb")
	f.write(data)
	f.close()

class Jpt:
	
	Version = 1
	
	@staticmethod
	def merge(jpt, jpeg, png):
		if not os.path.exists(jpeg):
			return "ERROR! File '%s' does not exist!" % jpeg
		if not os.path.exists(png):
			return "ERROR! File '%s' does not exist!" % png
		jpeg_data = _read_file(jpeg)
		png_data = _read_file(png)
		f = open(jpt, "wb")
		# header
		f.write("JPT")
		f.write(chr(Jpt.Version))
		# JPEG
		f.write(struct.pack(UINT_PACK_FORMAT, len(jpeg_data)))
		f.write(jpeg_data)
		# PNG
		f.write(struct.pack(UINT_PACK_FORMAT, len(png_data)))
		f.write(png_data)
		# done
		f.close()
		return "File '%s' has been successfully merged." % jpt
	
	@staticmethod
	def split(jpt, jpeg, png):
		if not os.path.exists(jpt):
			return "ERROR! File '%s' does not exist!" % jpt
		f = open(jpt, "rb")
		# header
		header = f.read(3)
		if header != "JPT":
			return "ERROR! File '%s' is not a valid JPT file! Expected 'JPT', got '%s'." % (jpt, header)
		version = ord(f.read(1))
		if version > Jpt.Version:
			return "ERROR! File '%s' version is not supported! Got version '%d'." % (jpt, version)
		# write JPEG
		jpeg_size = struct.unpack(UINT_PACK_FORMAT, f.read(4))[0]
		jpeg_data = f.read(jpeg_size)
		_write_file(jpeg, jpeg_data)
		# write PNG
		png_size = struct.unpack(UINT_PACK_FORMAT, f.read(4))[0]
		png_data = f.read(png_size)
		_write_file(png, png_data)
		# done
		f.close()
		return "File '%s' has been successfully split." % jpt
