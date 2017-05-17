import sys
from PIL import Image

if len(sys.argv) >= 2:
	filename = sys.argv[1]
else:
	filename = "../assets/characters.png"
im = Image.open(filename)
print im.format, im.size, im.mode
print "{",
for row in range(16):
	for column in range(16):
		print "{",
		for bit in range(2):
			for charY in range(8):
				y = row*8+charY
				val = 0
				for charX in range(8):
					pixel = im.getpixel((column*8+charX, y))[0] / 64
					pcolor = 0
					if pixel > 0:
						pcolor = 4 - pixel
					pbit = (pcolor >> bit) & 0x01
					val |= (pbit << (7-charX))
				print str(val & 0xFF)+ ",",
		print "},"
print "}"