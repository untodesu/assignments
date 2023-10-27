#!/bin/sh

# Split panchromatic (grayscale) image into YCbCr channels
magick src_pan.bmp -colorspace YCbCr -channel 0 -separate int_pan_Y.tif
magick src_pan.bmp -colorspace YCbCr -channel 1 -separate int_pan_Cb.tif
magick src_pan.bmp -colorspace YCbCr -channel 2 -separate int_pan_Cr.tif

# Split multichannel (colored) image into YCbCr channels
# NOTE: the multichannel is assumed to be twice as small as panchromatic
magick src_multi.bmp -resize 200% -colorspace YCbCr -channel 0 -separate int_multi_Y.tif
magick src_multi.bmp -resize 200% -colorspace YCbCr -channel 1 -separate int_multi_Cb.tif
magick src_multi.bmp -resize 200% -colorspace YCbCr -channel 2 -separate int_multi_Cr.tif

# Combine panchromatic luminance and multichannel chrominance
magick int_pan_Y.tif int_multi_Cb.tif int_multi_Cr.tif -set colorspace YCbCr -combine int_result_ycbcr.tif

# Convert to a more user-friendly format and colorspace
magick int_result_ycbcr.tif -colorspace sRGB result_ycbcr.png

