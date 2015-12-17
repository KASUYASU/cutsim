
This is a revised aewallin/Cutsim that can be used more practically.

5-Axis (XYZ + AC table-table) simulations are supported.

Some other features
1) Collision detection.
2) Machine travel limit dectection.
3) Cutting Power Estimation.
4) STL file reading for stock definitions(imcomplete and slow). 

The code is divided into the following parts:

g2m: calls the emc2 rs274 G-code interpreter and builds and produces a list of canonLine objects
libcutsim: cutting-simulation library (octree stock-model, stock/tool volumes, isosurface algorithms)
app: application and user-interface (depends on LibQGLViewer, ubuntu package "libqglviewer-qt4-2")


<Build-instructions>
cd cutsim
mkdir build
cd build
cmake ../src
make
./bin/cutsim

 Note that you need rs274 G-code interpreter that is build under the AC configuration.
(You have a sample for 32bit Linux in the build/bin folder.)

<Usage>
After building your "cutsim" excecutable, evoke it in the same folder that has
emc2 rs274 G-code interpreter, machine.mspec and Tool_table.tbl. Cutsim will
soon ask you one tool table file and one setup file. So choose 'Tool_table.tbl'
for the tool table and 'KJ66_Diffuser_Top.csim', etc. for the setup file. After that,
Cutsim displays the stock material (somtimes those do not appear. this is the Bug. :-)
Then, choose ngc files from the File-Open menu and press the 'Play' button. Maybe,
you will get the cutting result. If a collision is detected, Cutsim will 
automatically pause its simulation. You can continue the process, by pressing 
the 'Play' again. Some short cut keys are available. To get the result faster,
press your 'A' key and make the animation disenable. 

'A' toggle the Animation Enable/Disable.
'C' toggle the Axis Visible/Imvisible.
'G' toggle the XY-Grid Visible/Imvisible.
'S' Show the statistics in your console.
'T' toggle the Tool Visible/Imvisible.

