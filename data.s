# Assembler file for including application icons. A bit crude but it
# eliminates having image files external from the EXE

.section .data

.global png_timedit
png_timedit:
	.incbin "icons/timedit.png"
	