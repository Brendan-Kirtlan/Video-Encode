cd Desktop && git clone https://github.com/hoovi1234/Video-Encode-PREBUILT.git vid-encode -y

cd vid-encode && cd 'Video Encode 2' && g++ main.cpp lodepng.cpp -o video_encode $(pkg-config --cflags --libs opencv4) -lavcodec -lavformat -lavutil -lswscale
