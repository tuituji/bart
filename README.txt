BART: A Benchmark for Animated Ray Tracing
==============================================
by Jonas Lext, Ulf Assarsson, and Tomas M�ller
May 2000
----------------------------------------------
The three test scenes are located in the
subdirectories called:
kitchen
robots
museum

The source code for parsing, animation, and
textures are located in
src



By tuituji
----------------------------------------------
Build & run on Linux:
cd src
make
cp raytracer ../kitchen
cd ../kitchen
./raytracer kitchen.aff image

output in directory image


Make film:
cd image
convert *.ppm kitchen.mpg


Still many works to do !!!
