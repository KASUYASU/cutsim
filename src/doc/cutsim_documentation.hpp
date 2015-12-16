/*!
 \mainpage Cutsim documentation
 Cutsim is a GPL-licensed milling/turning/3D-printing material-removal/addition simulation.

 \section sec_1 Code
 The code is divided into three parts: <b>g2m</b>, <b>cutsim</b>, and <b>app</b>
 \subsection g2msec g2m 
 calls the emc2 rs274 G-code interpreter and builds and produces a list of canonLine objects
 \subsection cutsim
 cutting-simulation library: octree stock-model, stock/tool volumes, isosurface algorithms.
 \subsection app
 application and user-interface. Depends on LibQGLViewer, ubuntu package "libqglviewer-qt4-2".

<HR>

 \section sec_2 Installing and Building 
 To get cutsim:<BR>
 git clone https://github.com/aewallin/cutsim.git<BR>
 
 To build cutsim:<BR>
cd cutsim<BR>
mkdir build<BR>
cd build<BR>
cmake ../src<BR>
make<BR>
./bin/cutsim<BR>

 \subsection dep_sec Dependencies
 git<BR>
 boost<BR>
 qt<BR>
 libQGLViewer<BR>


*/
