
Cutsim is very experimental at the moment!

the code is divided into the following parts:

g2m: calls the emc2 rs274 G-code interpreter and builds and produces a list of canonLine objects
libcutsim: cutting-simulation library (octree stock-model, stock/tool volumes, isosurface algorithms)
app: application and user-interface (depends on LibQGLViewer, ubuntu package "libqglviewer-qt4-2")


Build-instructions:
cd cutsim
mkdir build
cd build
cmake ../src
make
./bin/cutsim

