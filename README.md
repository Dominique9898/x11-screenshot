# x11-screenshot

dependencies:

libX11,libpng

`sudo apt install libx11-dev libpng-dev`


Start:

g++ shot.cpp -o makeshot -lX11 -lpng -std=c++11

./makshot
