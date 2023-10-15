# Pansharpening

## From DIMPROC perspective
Pansharpening appears to do its thing by ripping color
information from a lower-resolution multichannel (colored)
image and force-feeding it into the higher-resolution
panchromatic (grayscale) image, producing sharp edges with
possibly hard-to-notice color bleed.  

![](readme_color.png)

## From Analog Video perspective
Replace "multichannel" with "chrominance" and "panchromatic"
with "luminance" and you basically get [Chroma subsampling](https://en.wikipedia.org/wiki/Chroma_subsampling).  

![](readme_chrominance.jpg)  
[Image source](https://forum.videohelp.com/threads/246275-Is-it-Color-Chroma-Shift-or-Bleed-or-Smearing-can-I-improve-it)

## Color spaces
* The teacher asked us to process imagery in CIELAB color space.  
* TIFF also supports YCbCr color space, so I made a copy of the process script to process the image in that color space as well.
* Pansharpening/ChromaSubsampling should work well with a lot of color spaces that involve separating luminance into a separate channel alongside two (or more?) chrominance channels. These colorspaces include YIQ (NTSC), YUV (PAL), HSV and much more.


