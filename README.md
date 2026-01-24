A fun project showing a proof of concept I've thought about for a while.
Since some video hosting services let you store unlimited videos, you can store infinite data if used correctly. 
This project uses lodepng, ffmpeg, and opencv2 to encode files into video format

Video explanation: https://youtu.be/_w6PCHutmb4?si=J81z_G9hhyyJ-BBY

# Manual build
To build this project on Linux, simply follow the instructions below. Windows will only work if you use something like WSL2 or MINGW64.

1. cd to the project folder (for this i'll just use /usr/vid-encode)

`cd '/usr/vid-encode/Video Encode 2'`

2. use g++ to build main.cpp

`g++ main.cpp lodepng.cpp -o video_encode $(pkg-config --cflags --libs opencv4) -lavcodec -lavformat -lavutil -lswscale`


3. use `./video_encode encode /path/to/your/file` to encode files and `./video_encode decode /path/to/encoded/video` to decode.

# Use included auto builder
1. Download the install.sh from the repo.

2. Run it. It will automatically install to ~/Desktop/vid-encode and you can run the program by going to ~/Desktop/vid-encode/Video Encode 2 and running ./video_encode.

## Requirements
You will need FFMPEG and Opencv2 to actually use the program. All requirements for building are included.
