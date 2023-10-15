#!/bin/sh

# Split panchromatic (grayscale) image into LAB channels
magick src_pan.bmp -colorspace LAB -channel 0 -separate int_pan_L.tif
magick src_pan.bmp -colorspace LAB -channel 1 -separate int_pan_A.tif
magick src_pan.bmp -colorspace LAB -channel 2 -separate int_pan_B.tif

# Split multichannel (colored) image into LAB channels
# NOTE: the multichannel is assumed to be twice as small as panchromatic
magick src_multi.bmp -resize 200% -colorspace LAB -channel 0 -separate int_multi_L.tif
magick src_multi.bmp -resize 200% -colorspace LAB -channel 1 -separate int_multi_A.tif
magick src_multi.bmp -resize 200% -colorspace LAB -channel 2 -separate int_multi_B.tif

# Combine panchromatic luminance and multichannel chrominance
magick int_pan_L.tif int_multi_A.tif int_multi_B.tif -set colorspace LAB -combine int_result.tif

# Convert to a more user-friendly format and colorspace
magick int_result.tif -colorspace sRGB result.png

