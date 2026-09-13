import pathlib
import sys

image = pathlib.Path(sys.argv[1]).read_bytes()
output = "unsigned char binary_icons_timedit_png_start[] = {\n"
for offset in range(0, len(image), 12):
	output += "\t" + ", ".join(str(value) for value in image[offset:offset+12]) + ",\n"
output += "};\n"
output += "unsigned int binary_icons_timedit_png_size = sizeof(binary_icons_timedit_png_start);\n"
pathlib.Path(sys.argv[2]).write_text(output)
