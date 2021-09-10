# MAX2Sphere

Takes raw GoPro .360 frames (with GoPro EAC projection) and converts them to a more widely recoginised equirectangular projection.

## Installation




## Usage

### Preperation

This script is designed to be used with frames. You will need to first convert a `.360` video to frames and then pass the two corresponding frames to the script.

You can use ffmpeg to split your `.360` video into frames (below at a rate of 1 FPS).

```
$ ffmpeg -i GS070135.360 -map 0:0 -r 1 track0/img%d.jpg -map 0:5 -r 1 track5/img%d.jpg
```
### Script

```
$ MAX2sphere [options] frame1name frame2name
```

Options:

* -w n sets the output image width, default: 4096
* -a n sets antialiasing level, default = 2
* -o s specify the output filename, default is based on track0 name
* -d enable debug mode, default: off


### Camera support

This script has currently been tested with the following cameras and modes:

* GoPro MAX
	* 360 Video (output .360)
		* 5.6K stitched (24FPS, 30FPS)
		* 3k stitched (60 FPS)
	* 360 TimeWarp (output .360)
		* (2x,5x,10x,15x,30x frame rate)

## Support

Join our Discord community and get in direct contact with the Trek View team, and the wider Trek View community.

[Join the Trek View Discord server](https://discord.gg/ZVk7h9hCfw).