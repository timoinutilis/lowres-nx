from PIL import Image

im = Image.open("../assets/characters.png")
print im.format, im.size, im.mode
print "{",
for row in range(16):
	for column in range(16):
		print "{",
		for charY in range(8):
			y = row*8+charY
			val = 0
			for charX in range(8):
				pixel = im.getpixel((column*8+charX, y))[0] / 64
				val |= (pixel << (7-charX)*2)
			print val, ",",
		print "},"
print "}"