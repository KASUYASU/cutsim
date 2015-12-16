/*  
 *  Copyright 2010-2011 Anders Wallin (anders.e.e.wallin "at" gmail.com)
 *  Copyright 2015      Kazuyasu Hamada (k-hamada@gifu-u.ac.jp)
 *  
 *  This file is part of Cutsim / OpenCAMlib.
 *
 *  OpenCAMlib is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  OpenCAMlib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with OpenCAMlib.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "cutsim_def.hpp"
#include "cutsim_window.hpp"

#include "lex_analyzer.hpp"
#include <cutsim/facet.hpp>

CutsimWindow::CutsimWindow(QStringList ags) : args(ags), myLastFolder(tr("")), settings("github.aewallin.cutsim","cutsim") {
        octree_cube_size = DEFAULT_CUBE_SIZE / 2.0;
        max_depth = DEFAULT_MAX_DEPTH;
        cube_resolution = octree_cube_size * 2.0 / pow(2.0, max_depth - 1);
        specific_cutting_force = DEFAULT_SPECIFIC_CUTTING_FORCE;
        powerCoff = (cube_resolution * cube_resolution) * specific_cutting_force / (60.0 * 16 * 1e3);
        octree_center = new cutsim::GLVertex(0.0, 0.0, 0.0);
        step_size = DEFAULT_STEP_SIZE;
        variable_step_mode = false;
        myGLWidget = new cutsim::GLWidget(DEFAULT_SCENE_RADIUS);
        cutsim::GLData* gld = myGLWidget->addGLData();
        this->setCentralWidget(myGLWidget);
        myMachine = new cutsim::Machine();
        
        createDock();
        createActions();
        createMenus();
        setStatusBar( new QStatusBar(this) );
        myProgress = new QProgressBar();
        myProgress->setMaximum(100);
        myProgress->setMinimum(0);
        myStatus = new QLabel;
        myStatus->setText("Line : 0 Evaluated Time 0:0");
        statusBar()->insertPermanentWidget( 0, myStatus , 0);
        statusBar()->insertPermanentWidget( 1, myProgress , 0);
        createToolBar();        
        
        myG2m = new g2m::g2m(); // g-code interpreter

        connect( this, SIGNAL( setGcodeFile(QString) ),     myG2m, SLOT( setFile(QString)) );
        connect( this, SIGNAL( setRS274(QString) ),         myG2m, SLOT( setInterp(QString)) );
        connect( this, SIGNAL( setToolTable(QString) ),     myG2m, SLOT( setToolTable(QString)) );
        connect( this, SIGNAL( interpret() ),               myG2m, SLOT( interpret_file() ) );
        connect( myG2m, SIGNAL( debugMessage(QString) ),     this, SLOT( debugMessage(QString) ) );
        connect( myG2m, SIGNAL( gcodeLineMessage(QString) ), this, SLOT( appendGcodeLine(QString) ) );
        connect( myG2m, SIGNAL( canonLineMessage(QString) ), this, SLOT( appendCanonLine(QString) ) );
        
        myPlayer = new g2m::GPlayer();
        connect( myPlayer, SIGNAL( debugMessage(QString) ), this, SLOT( debugMessage(QString) ) );
        connect( myPlayer, SIGNAL( signalProgress(int, int, double, bool) ),   this, SLOT( slotSetProgress(int, int, double, bool) ) );
        connect(     this, SIGNAL( play() ), myPlayer, SLOT( play() ) );
        connect(     this, SIGNAL( pause() ), myPlayer, SLOT( pause() ) );
        connect(     this, SIGNAL( stop() ), myPlayer, SLOT( stop() ) );
        connect(    myG2m, SIGNAL( signalCanonLine(canonLine*) ), myPlayer, SLOT( appendCanonLine(canonLine*) ) );
#ifdef MULTI_AXIS
        connect( myPlayer, SIGNAL( signalToolPosition(double,double,double,double,double,double,int,int,double) ), this, SLOT( slotSetToolPosition(double,double,double,double,double,double,int,int,double) ) );
#else
        connect( myPlayer, SIGNAL( signalToolPosition(double,double,double,int,int,double) ), this, SLOT( slotSetToolPosition(double,double,double,int,int,double) ) );
#endif
        connect( myPlayer, SIGNAL( signalToolChange( int ) ), this, SLOT( slotToolChange(int) ) );     
        
        connect( this, SIGNAL( signalMoveDone() ), myPlayer, SLOT( slotRequestMove() ) );
        
//       connect( myCutsim, SIGNAL( signalDiffDone(int,int,int,double) ), this, SLOT( slotDiffDone(int,int,int,double) ) );
//       connect( myCutsim, SIGNAL( signalGLDone() ), this, SLOT( slotGLDone() ) );

        connect( myGLWidget, SIGNAL( statusBarMessage(QString) ), this, SLOT ( statusBarMessage(QString) ) );
        
        findInterp();
        chooseMachineSpecFile();
        if (myMachine->traverse_feed_rate != DEFAULT_TRAVERSE_FEED_RATE)
        	myPlayer->setTraverseFeedRate(myMachine->traverse_feed_rate);

        // T0 -- No tool
        cutsim::CutterVolume* s0 = new cutsim::CutterVolume();
        s0->length = myMachine->max_z_limit;
        s0->setColor(CUTTING_COLOR);
        s0->setHolderRadius(myMachine->holderradius);
        s0->setHolderLength(myMachine->holderlength);
        s0->enableHolder(true);
        myTools.push_back(s0);

        currentTool = 0;
		myGLWidget->setTool(myTools[currentTool]);

        chooseToolTable();
        chooseSetupFile();

        cube_resolution = octree_cube_size * 2.0 / pow(2.0, max_depth - 1);
        powerCoff = (cube_resolution * cube_resolution) * specific_cutting_force / (60.0 * 16 * 1e3);

        myCutsim = new cutsim::Cutsim(octree_cube_size , max_depth, octree_center, gld, myGLWidget);

        connect( myCutsim, SIGNAL( signalDiffDone(int,int,int,double) ), this, SLOT( slotDiffDone(int,int,int,double) ) );
        connect( myCutsim, SIGNAL( signalGLDone() ), this, SLOT( slotGLDone() ) );

        // hard-coded stock
        cutsim::RectVolume2* stock0 = new cutsim::RectVolume2();
        stock0->setWidth(1.0);
        stock0->setLength(1.0);
        stock0->setHight(1.0);
        stock0->setCenter(cutsim::GLVertex(0.0, 0.0, 0.0));
        stock0->calcBB();
        stock0->setColor(STOCK_COLOR);
        myCutsim->sum_volume(stock0);

        QString title = tr(" cutsim - ") + VERSION_STRING;
        setWindowTitle(title);
        showNormal();
        move(100,100); // position the main window
        resize(789,527);  // size window

		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		createStockParts();
		QApplication::restoreOverrideCursor();
        myCutsim->updateGL();
}

CutsimWindow::~CutsimWindow()
{
	delete myMachine;
	delete myPlayer;
	delete myG2m;
	delete myProgress;
	delete myCutsim;
	delete myGLWidget;
}

// >> means signal.
//slotRequestMove >> slotSetToolPosition -> slot_diff_volume_mt >> slotDiffDone -> update_gl_mt >> slotGLDone >> slotRequestMove
// called by gplayer
#ifdef MULTI_AXIS
void CutsimWindow::slotSetToolPosition(double x, double y, double z, double a, double b, double c, int line, int mstatus, double feedrate) {
{static int preline, preerror;
int error;
if ((preline != line) && (error = myMachine->checkLimit(x, y, z, a, 0.0, b)) && (preerror != error)) {
	QString message = tr("Machine limit %1").arg(error) + tr("@ line:%1 ").arg(myG2m->toGcodeLineNo(line))
			        + tr(" X:%1").arg(x)
					+ tr(" Y:%1").arg(y)
					+ tr(" Z:%1").arg(z)
#ifdef MULTI_AXIS
					+ tr(" A:%1").arg(SIGN_A*(a))
					+ tr(" C:%1").arg(SIGN_C*(b))
#endif
					;
	debugMessage(message);
	pauseProgram();
	preline = line;
	preerror = error;
}
}
    myTools[currentTool]->setAngle( cutsim::GLVertex(a,0.0,b) );
    myGLWidget->setToolPosition(x,y,z,a,0.0,b);
#else
void CutsimWindow::slotSetToolPosition(double x, double y, double z, int line, int mstatus, double feedrate) {
    myGLWidget->setToolPosition(x,y,z);
#endif
    myTools[currentTool]->setCenter( cutsim::GLVertex(x,y,z) );
    myCutsim->slot_diff_volume_mt( myTools[currentTool], line, mstatus, feedrate);
}

void CutsimWindow::slotDiffDone(int line, int mstatus, int error, double cuttingPower) { // called when the cut-thread is done and we can update GL
    qDebug() << " slotDiffDone() ";

static int preline;
int gcodeline;
gcodeline = myG2m->toGcodeLineNo(line);
if (error) {
  if (preline != gcodeline) {
	cutsim::GLVertex position = myTools[currentTool]->getCenter();
	cutsim::GLVertex angle	  = myTools[currentTool]->getAngle() * (180.0/PI);
	double offset = (myTools[currentTool]->cuttertype == cutsim::BALL) ? myTools[currentTool]->radius : 0.0;
	g2m::Pose current_origin = myPlayer->getCanonLine(line)->getStatus()->getOrigin();
	QString posStr = tr(" X:%1").arg(position.x - current_origin.loc.x)
				   + tr(" Y:%1").arg(position.y - current_origin.loc.y)
				   + tr(" Z:%1").arg(position.z - current_origin.loc.z - offset)
#ifdef MULTI_AXIS
				   + tr(" A:%1").arg(SIGN_A*(angle.x - current_origin.dir.x))
				   + tr(" C:%1").arg(SIGN_C*(angle.z - current_origin.dir.z))
#endif
	 	 	 ;
	if (error & (cutsim::PARTS_COLLISION | cutsim::HOLDER_COLLISION | cutsim::SHANK_COLLISION | cutsim::NECK_COLLISION)) {
			QString message;
			message = ((error & cutsim::PARTS_COLLISION) ? ((error & (cutsim::HOLDER_COLLISION | cutsim::SHANK_COLLISION | cutsim::NECK_COLLISION)) ? tr("PARTS-") : tr("PARTS")) : tr(""))
					 + ((error & cutsim::HOLDER_COLLISION) ? tr("HOLDER") : (error & cutsim::SHANK_COLLISION) ? tr("SHANK") : (error & cutsim::NECK_COLLISION) ? tr("NECK") : tr(""))
					 + tr(" Collision Detected @ line:%1").arg(gcodeline);
			debugMessage(message + posStr);
}
	if (error & (g2m::OFF | g2m::BRAKE | g2m::TRAVERSE)) {
		QString message;
		message = ((error & g2m::TRAVERSE) ? tr("High Speed Cuttings") : tr("Spindle Stoping"))
								+ tr("@ line:%1 ").arg(gcodeline);
		debugMessage(message + posStr);
	}
  }
	if (preline != gcodeline)
	if (error & (cutsim::PARTS_COLLISION | cutsim::HOLDER_COLLISION | cutsim::SHANK_COLLISION)) {
		pauseProgram();
		preline = gcodeline;
	}
}
requiredPower = powerCoff * cuttingPower;
if (requiredPower > myMachine->max_spindle_power)
	//debugMessage(tr("Power Over %1 w @line: %2").arg(requiredPower).arg(myG2m->toGcodeLineNo(line)));
	debugMessage(tr("Power Over %1 w @line: %2").arg(requiredPower).arg(gcodeline));
std::cout << "Req. Power:" << requiredPower << "w\n";

    myCutsim->update_gl_mt(); //updateGL();
}

void CutsimWindow::slotGLDone() { // called when GL-update done. we can now request a new move from gplayer
    // request more g-code from player here.
    emit signalMoveDone();
    qDebug() << " slotGLDone() ";
}

void CutsimWindow::slotToolChange(int t) {
    debugMessage( tr("Tool change to No.%1 ").arg(t) );
    if (t <= (int)myTools.size()) {
    	currentTool = t;
    	myMachine->z_limit_offset = myTools[currentTool]->length;
    	if (variable_step_mode)
    		myPlayer->setStepSize(myTools[currentTool]->radius * step_size * 2.0);
    	else
    		myPlayer->setStepSize(step_size);
    } else
    	debugMessage( tr("Can't find tool No.%1").arg(t));

	myGLWidget->setTool(myTools[currentTool]);
}    

///find the interpreter. uses QSettings, so user is only asked once unless the file is deleted
void CutsimWindow::findInterp() {
    QString interp;
    interp = settings.value("rs274/binary","/usr/bin/rs274").toString();
    if (!QFileInfo(interp).isExecutable()) {
        QString m = "Tried to use ";
        m += interp + " as the interpreter, but it doesn't exist or isn't executable.";
        debugMessage(m);
        interp = QFileDialog::getOpenFileName ( this, "Locate rs274 interpreter", "~", "EMC2 stand-alone interpreter (rs274)" );
        if (!QFileInfo(interp).isExecutable()) {
            debugMessage("Error: Interpreter does not exist!");
        }
        settings.setValue("rs274/binary",interp);
    }
    emit setRS274( interp );
}

///find the tool table. uses QSettings, to preselect the last table used
void CutsimWindow::chooseToolTable() {
    QString path;
    bool ttconf = settings.contains("rs274/tool-table");
    if (ttconf) {
        //passing the file name as the path means that it is preselected
        path = settings.value("rs274/tool-table").toString();
    } else {
        path = "/usr/share/doc/emc2/examples/sample-configs/sim"; //location when installed
    }
    path = QFileDialog::getOpenFileName ( this, "Locate tool table", path, "EMC2 new-style tool table (*.tbl)" );

    if (!QFileInfo(path).exists()){
        debugMessage("Error: Tool table does not exist!");
        return;
    }
    settings.setValue("rs274/tool-table",path);
    emit setToolTable(path);

    readToolTable(path);
}

void CutsimWindow::open() {
    QString     fileName;
    QString     fileType;
    QFileInfo   fileInfo;

    statusBar()->showMessage(tr("Open G-code file"));
    fileName = QFileDialog::getOpenFileName (this,
                        tr("Open G-code File"),
                        myLastFolder,
                        tr( "G-code (*.ngc *.canon);;"
                            ) );
    if (!fileName.isEmpty()) {
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    	fileInfo.setFile(fileName);
        fileType = fileInfo.suffix();
        if (fileType.toLower() == tr("G-code") || fileType.toLower() == tr("ngc") || fileType.toLower() == tr("canon"))  {
            statusBar()->showMessage( tr(" Opening g-code file %1").arg(fileName) );
        }
        myLastFolder = fileInfo.absolutePath();
        emit setGcodeFile( fileName );
        emit interpret();
		QApplication::restoreOverrideCursor();
    }
}

void CutsimWindow::createDock() {
    QDockWidget* dockWidget1 = new QDockWidget(this);
    dockWidget1->setWindowTitle("Debug");
    debugText = new TextArea(); 
    dockWidget1->setWidget(debugText);
    debugText->setReadOnly(true);
    
    QDockWidget *dockWidget2 = new QDockWidget(this);
    dockWidget2->setWindowTitle("G-Code");
    gcodeText = new TextArea(); 
    dockWidget2->setWidget(gcodeText);
    gcodeText->setReadOnly(true);
    QDockWidget *dockWidget3 = new QDockWidget(this);
    dockWidget3->setWindowTitle("CANON-lines");
    canonText = new TextArea(); 
    dockWidget3->setWidget(canonText);
    canonText->setReadOnly(true);
    
    addDockWidget(Qt::RightDockWidgetArea, dockWidget2);
    addDockWidget(Qt::RightDockWidgetArea, dockWidget3);
    addDockWidget(Qt::BottomDockWidgetArea, dockWidget1);
}

void CutsimWindow::createToolBar() {
    myToolBar = new QToolBar(this);
    addToolBar( myToolBar );
    myToolBar->addAction( newAction );
    myToolBar->addAction( openAction );
    myToolBar->addSeparator();
    myToolBar->addAction( playAction );
    myToolBar->addAction( pauseAction );
    myToolBar->addAction( stopAction );
}

void CutsimWindow::createActions() {        
    QIcon newIcon = QIcon::fromTheme("document-new");
    newAction = new QAction(newIcon,tr("&New"), this);
    newAction->setShortcut(tr("Ctrl+N"));
    connect(newAction, SIGNAL(triggered()), this, SLOT(newFile()));
    
    QIcon openIcon = QIcon::fromTheme("document-open" );
    openAction = new QAction(openIcon, tr("&Open..."), this);
    openAction->setShortcut(tr("Ctrl+O"));
    connect(openAction, SIGNAL(triggered()), this, SLOT(open()));

    exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcut(tr("Ctrl+X"));
    exitAction->setStatusTip(tr("Exit the application"));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

    aboutAction = new QAction(tr("&About"), this);
    aboutAction->setStatusTip(tr("Show the application's About box"));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
    
    QIcon playIcon = QIcon::fromTheme("media-playback-start");
    playAction = new QAction(playIcon,tr("&Play"), this);
    playAction->setShortcut(tr("Ctrl+P"));
    connect(playAction, SIGNAL(triggered()), this, SLOT( runProgram() ));
    
    QIcon pauseIcon = QIcon::fromTheme("media-playback-pause");
    pauseAction = new QAction(pauseIcon,tr("&Pause"), this);
    pauseAction->setShortcut(tr("Ctrl+L"));
    connect(pauseAction, SIGNAL(triggered()), this, SLOT( pauseProgram() ));
    
    QIcon stopIcon = QIcon::fromTheme("media-playback-stop");
    stopAction = new QAction(stopIcon,tr("&Stop"), this);
    stopAction->setShortcut(tr("Ctrl+C"));
    connect(stopAction, SIGNAL(triggered()), this, SLOT( stopProgram() ));
}

void CutsimWindow::createMenus() {
    fileMenu = menuBar()->addMenu( tr("&File") );
        fileMenu->addAction( newAction );
        fileMenu->addAction( openAction );
        fileMenu->addSeparator();
        fileMenu->addAction( exitAction );

    helpMenu = new QMenu(tr("&Help"));
        helpAction = menuBar()->addMenu(helpMenu);
        helpMenu->addAction(aboutAction);
}

int CutsimWindow::readToolTable(QString file) {
	int tool_count = 0;
	QFile toolFileHandle( file );
    QString line, message;
    int slot_No, tool_Id;
    double d[6];

	if ( toolFileHandle.open( QIODevice::ReadOnly | QIODevice::Text) ) {
		QTextStream in( &toolFileHandle );
        while ( !in.atEnd() ) {
            // read and parse the tool table line
            line = in.readLine();         // line of text excluding '\n'
            if (line.size() == 0) continue;
		// Slot No. Tool ID Length Diam. Flute len. Neck Diam. Reach len. Shank diam.
            lex_analyzer::LexAnalyzer lex(line.toStdString());
            if ((slot_No = lex.token2i(0)) == INT_MIN || slot_No <= 0) continue;
            if ((tool_Id = lex.token2i(1)) == INT_MIN || tool_Id <= cutsim::NO_TOOL) continue;
            for (int n=0; n < 6; n++)
            	d[n] = lex.token2d(n+2);
            if ((d[0] == NAN) || (d[0] < 0.0) || (d[1] == NAN) || (d[1] < 0.0)) continue;
            if (myTools.size() < (unsigned)slot_No+1 && slot_No < DEFAULT_MAX_TOOL_SLOT) {
            	int n = (unsigned)slot_No+1 - myTools.size();
            	for (; n > 0; n--)	myTools.push_back(myTools[0]);

            }
            message = tr("Slot No.%1 ").arg(slot_No);
            switch (tool_Id) {
            case cutsim::CYLINDER: {
            	cutsim::CylCutterVolume* cylCutter = new cutsim::CylCutterVolume();
            	cylCutter->cuttertype = cutsim::CYLINDER;
            	message += tr("Tool:CYLN ");
            	cylCutter->setLength(d[0]);
            	message += tr("Len. %1 ").arg(d[0]);
            	cylCutter->setRadius(d[1]*0.5);
            	message += tr("Diam. %1 ").arg(d[1]);
            	if (d[2] != NAN && d[2] <= d[0]) {
            		cylCutter->setFluteLength(d[2]);
               message += tr("Flute len. %1 ").arg(d[2]);
            	}
            	if (d[3] != NAN && d[3] > 0.0 && d[3] <= d[1]) {
            		cylCutter->setNeckRadius(d[3] * 0.5);
               message += tr("Neck Diam. %1 ").arg(d[3]);
            	}
            	if (d[4] != NAN && d[4] > 0.0  && d[4] <= d[0]) {
            		cylCutter->setReachLength(d[4]);
               message += tr("Reach len. %1 ").arg(d[4]);
            	}
            	if (d[5] != NAN && d[5] > 0.0) {
            		cylCutter->setShankRadius(d[5]*0.5);
               message += tr("Shank Diam. %1 ").arg(d[5]);
            	}
            	cylCutter->setColor(CUTTING_COLOR);
            	cylCutter->setHolderRadius(myMachine->holderradius);
            	cylCutter->setHolderLength(myMachine->holderlength);
            	cylCutter->enableHolder(true);
            	myTools[slot_No] = cylCutter;
            	qDebug() << "Slot No:" << slot_No << " tool ID:" << tool_Id << " len:" << d[0] << " diam:" << d[1]
            	           << " flen:" << d[2] << " ndiam:" << d[3] << " rlen:" << d[4] << " sdiam:" << d[5] << "\n";
            	break;
            	}
            case cutsim::BALL: {
            	cutsim::BallCutterVolume* ballCutter = new cutsim::BallCutterVolume();
            	ballCutter->cuttertype = cutsim::BALL;
            	message += tr("Tool:BALL ");
            	ballCutter->setLength(d[0]);
            	message += tr("Len. %1 ").arg(d[0]);
            	ballCutter->setRadius(d[1]*0.5);
            	message += tr("Diam. %1 ").arg(d[1]);
            	if (d[2] != NAN && d[2] <= d[0]) {
            		ballCutter->setFluteLength(d[2]);
               message += tr("Flute len. %1 ").arg(d[2]);
            	}
            	if (d[3] != NAN && d[3] > 0.0 && d[3] <= d[1]) {
            		ballCutter->setNeckRadius(d[3] * 0.5);
               message += tr("Neck Diam. %1 ").arg(d[3]);
            	}
            	if (d[4] != NAN && d[4] > 0.0  && d[4] <= d[0]) {
            		ballCutter->setReachLength(d[4]);
               message += tr("Reach len. %1 ").arg(d[4]);
            	}
            	if (d[5] != NAN && d[5] > 0.0) {
            		ballCutter->setShankRadius(d[5]*0.5);
               message += tr("Shank Diam. %1 ").arg(d[5]);
            	}
            	ballCutter->setColor(CUTTING_COLOR);
            	ballCutter->setHolderRadius(myMachine->holderradius);
            	ballCutter->setHolderLength(myMachine->holderlength);
            	ballCutter->enableHolder(true);
            	myTools[slot_No] = ballCutter;
            	qDebug() << "Slot No:" << slot_No << " tool ID:" << tool_Id << " len:" << d[0] << " diam:" << d[1]
            	           << " flen:" << d[2] << " ndiam:" << d[3] << " rlen:" << d[4] << " sdiam:" << d[5] << "\n";
            	break;
            	}
            default:	message += tr("Tool:? ");

            }
            debugMessage(message);
            tool_count++;
        }
	} else {
		debugMessage("Error: Can't open:" + file);
		return tool_count;
	}

    toolFileHandle.close();

    qDebug() << "Slot Size:" << myTools.size() << "\n";

	return tool_count;
}

///find the setup file. uses QSettings, to preselect the last file used
void CutsimWindow::chooseSetupFile() {
    QString path;
    bool ttconf = settings.contains("cutsim/setups");
    if (ttconf) {
        //passing the file name as the path means that it is preselected
        path = settings.value("cutsim/setups").toString();
    } else {
        path = "/usr/share/doc/emc2/examples/sample-configs/sim"; //location when installed
    }
    path = QFileDialog::getOpenFileName ( this, "Locate setup file", path, "Cutsim Setup File (*.csim)" );

    if (!QFileInfo(path).exists()){
        debugMessage("Error: Setup file does not exist!");
        return;
    }
    settings.setValue("cutsim/setups",path);

    int error_count;
    debugMessage( tr("Read Setup file: %1").arg(path) );
    error_count = readSetupFile(path);
    if (error_count)
			debugMessage( tr("%1 Error for reading Setup file").arg(error_count) );
    else
    	debugMessage( tr("Successflly read Setup file") );
}

/// read setup file and set
int CutsimWindow::readSetupFile(QString file) {
	int error_count = 0;
	int line_count = 0;
	QFile setupFileHandle( file );
    QString line, message;

	if ( setupFileHandle.open( QIODevice::ReadOnly | QIODevice::Text) ) {
		QTextStream in( &setupFileHandle );
        while ( !in.atEnd() ) {
            // read and parse the setup file line
            line = in.readLine();         // line of text excluding '\n'
            line_count++;
            if (line.size() == 0) continue;
            lex_analyzer::LexAnalyzer lex(line.toStdString());
            message = line;
            if (lex.wordMatch("aho", 0)) {
            	message = "baka";
			}
            if (lex.wordMatch("OCTREE_CUBE_SIZE", 0)) {
            	double size;
            	if ((size = lex.token2d(1)) == NAN || size <= 0) {
            		message += tr("ERROR @line %1").arg(line_count); debugMessage(message);
            		error_count++;
            		continue;
            	}
            	octree_cube_size = size / 2.0;
			}
            if (lex.wordMatch("OCTREE_MAX_DEPTH", 0)) {
            	int depth;
            	if ((depth = lex.token2i(1)) == INT_MIN || depth <= 2) {
            		message += tr("ERROR @line %1").arg(line_count); debugMessage(message);
            		error_count++;
            		continue;
            	}
            	max_depth = (unsigned int)depth;
			}
            if (lex.wordMatch("OCTREE_CENTER", 0)) {
            	double x = lex.token2d(1), y = lex.token2d(2), z = lex.token2d(3);
            	if (x == NAN || y == NAN || z == NAN) {
            		message += tr("ERROR @line %1").arg(line_count); debugMessage(message);
            		error_count++;
            		continue;
            	}
              delete octree_center;
              octree_center = new cutsim::GLVertex(x, y, z);
              message += " OK";
			}
            if (lex.wordMatch("USER_ORIGIN", 0)) {
            	double x = lex.token2d(1), y = lex.token2d(2), z = lex.token2d(3);
            	if (x == NAN || y == NAN || z == NAN) {
            		message += tr("ERROR @line %1").arg(line_count); debugMessage(message);
            		error_count++;
            		continue;
            	}
            	myG2m->setOrigin( g2m::Pose( g2m::Point(x,y,z), g2m::Point(0,0,0)) );
            	double a = lex.token2d(4), b = lex.token2d(5), c = lex.token2d(6);
            	if (a == NAN || b == NAN || c == NAN) {
            		message += tr("ERROR @line %1").arg(line_count); debugMessage(message);
            		error_count++;
            		continue;
            	}
            	myG2m->setOrigin( g2m::Pose( g2m::Point(x,y,z), g2m::Point(a*PI/180.0,b*PI/180.0,c*PI/180.0)) );
            	message += " OK";
			}
            if (lex.wordMatch("INITIAL_POSITION", 0)) {
            	double x = lex.token2d(1), y = lex.token2d(2), z = lex.token2d(3);
            	if (x == NAN || y == NAN || z == NAN) {
            		message += tr("ERROR @line %1").arg(line_count); debugMessage(message);
            		error_count++;
            		continue;
            	}
            	myG2m->setInitialPos( g2m::Pose( g2m::Point(x,y,z), g2m::Point(0,0,0)) );
            	double a = lex.token2d(4), b = lex.token2d(5), c = lex.token2d(6);
            	if (a == NAN || b == NAN || c == NAN) {
            		message += tr("ERROR @line %1").arg(line_count); debugMessage(message);
            		error_count++;
            		continue;
            	}
            	myG2m->setInitialPos( g2m::Pose( g2m::Point(x,y,z), g2m::Point(a*PI/180.0,b*PI/180.0,c*PI/180.0)) );
            	message += " OK";
			}
            if (lex.wordMatch("STEP_SIZE", 0)) {
            	double step_size = lex.token2d(2);
            	if (lex.wordMatch("VARIABLE", 1) && (step_size != NAN && step_size > 0.0)) {
            		this->step_size = step_size; variable_step_mode = true;
            		message += " OK";
            	} else if (lex.wordMatch("FIXED", 1) && (step_size != NAN && step_size > 0.0)) {
            		this->step_size = step_size; variable_step_mode = false;
            		message += " OK";
            	} else {
            		message += tr("ERROR @line %1").arg(line_count); debugMessage(message);
           		error_count++;
            		continue;
            	}
			}
            if (lex.wordMatch("SCF", 0)) {
            	double specific_cutting_force;
            	if ((specific_cutting_force = lex.token2d(1)) == NAN || specific_cutting_force <= 0) {
            		message += tr("ERROR @line %1").arg(line_count); debugMessage(message);
            		error_count++;
            		continue;
            	}
            	this->specific_cutting_force = specific_cutting_force;
			}
            if (lex.wordMatch("STOCK", 0) || lex.wordMatch("PARTS", 0)) {
            	debugMessage(message);
            	error_count = readStockFile(in, lex.wordMatch("PARTS", 0), line_count);
            	continue;
			}
            debugMessage(message);
        }
	} else {
		debugMessage("Error: Can't open:" + file);
		return ++error_count;
	}

    setupFileHandle.close();

    qDebug() << "readSetupFile error:" << error_count << "\n";

	return error_count;
}

/// read stock file and set
int CutsimWindow::readStockFile(QTextStream &in, bool parts, int& lineNo) {
	int error_count = 0;
	int line_count = lineNo;
	QString line, message, path;
	int	volumetype = cutsim::NO_VOLUME;
	double width = 0.0, length = 0.0, hight = 0.0;
	double radius;
	double cx = 0.0, cy = 0.0, cz = 0.0, rx = 0.0, ry = 0.0, rz = 0.0;
	double ra = 0.0, rb = 0.0, rc = 0.0;
	int operation = SUM_OPERATION;
	bool setCorner = false;

	while ( !in.atEnd() ) {
		line = in.readLine();         // line of text excluding '\n'
		line_count++;
		if (line.size() == 0) continue;
		lex_analyzer::LexAnalyzer lex(line.toStdString());
	message = line;
		if (lex.wordMatch("RECTANGLE", 0)) volumetype = cutsim::RECTANGLE_VOLUME;
		if (lex.wordMatch("CYLINDER" , 0)) volumetype = cutsim::CYLINDER_VOLUME;
		if (lex.wordMatch("SPHERE"   , 0)) volumetype = cutsim::SPHERE_VOLUME;
		if (lex.wordMatch("STL"      , 0)) volumetype = cutsim::STL_VOLUME;
		if (lex.wordMatch("WIDTH" , 0)) {
			width  = lex.token2d(1);
			if (width == NAN || width <= 0.0)
				message = tr("WIDTH Error @line %1").arg(line_count);
			 error_count++;
		}
		if (lex.wordMatch("LENGTH", 0)) {
			length = lex.token2d(1);
			if (length == NAN || length <= 0.0) {
				message = tr("LENGTH Error @line %1").arg(line_count);
			 error_count++;
			}
		}
		if (lex.wordMatch("HIGHT" , 0)) {
			hight = lex.token2d(1);
			if (hight == NAN || hight <= 0.0) {
				message = tr("HIGTH Error @line %1").arg(line_count);
			 error_count++;
			}
		}
		if (lex.wordMatch("RADIUS" , 0)) {
			radius = lex.token2d(1);
			if (radius == NAN || radius <= 0.0) {
				message = tr("RADIUS Error @line %1").arg(line_count);
			 error_count++;
			}
		}
		if (lex.wordMatch("CORNER", 0)) {
			cx = lex.token2d(1); cy = lex.token2d(2); cz = lex.token2d(3);
			if (cx == NAN || cy == NAN || cz == NAN) {
				message = tr("CORNER Error @line %1").arg(line_count);
				error_count++;
			} else
				setCorner = true;
		}
		if (lex.wordMatch("CENTER", 0)) {
			cx = lex.token2d(1); cy = lex.token2d(2); cz = lex.token2d(3);
			if (cx == NAN || cy == NAN || cz == NAN) {
				message = tr("CENTER Error @line %1").arg(line_count);
			 error_count++;
			}
		}
		if (lex.wordMatch("RCENTER", 0)) {
			rx = lex.token2d(1); ry = lex.token2d(2); rz = lex.token2d(3);
			if (rx == NAN || ry == NAN || rz == NAN) {
				message = tr("RENTER Error @line %1").arg(line_count);
			 error_count++;
			}
		}
		if (lex.wordMatch("ROTATION", 0)) {
			ra = lex.token2d(1); rb = lex.token2d(2); rc = lex.token2d(3);
			if (ra == NAN || rb == NAN || rc == NAN) {
				message = tr("ROTATION Error @line %1").arg(line_count);
				error_count++;
			}
		}
		if (lex.wordMatch("OPERATION", 0)) {
			if (lex.wordMatch("SUM", 1)) operation = SUM_OPERATION;
			if (lex.wordMatch("DIFF", 1)) operation = DIFF_OPERATION;
			if (lex.wordMatch("INTERSECT", 1)) operation = INTERSECT_OPERATION;
		}
		if (lex.wordMatch("FILE", 0)) {
			path = lex.getToken(1).c_str();
		}
		if (lex.wordMatch("END", 0)) {
			if (lex.wordMatch("STOCK", 1) || (parts && lex.wordMatch("PARTS", 1))) {
			switch (volumetype) {
			case cutsim::NO_VOLUME:
				message += " ??";
				break;
			case cutsim::	RECTANGLE_VOLUME: {
				StockVolume *stockVolume = new StockVolume();
				cutsim::RectVolume2* stock = new cutsim::RectVolume2();
				stock->setWidth(width);
				stock->setLength(length);
				stock->setHight(hight);
				if (setCorner)
					stock->setCorner(cutsim::GLVertex(cx, cy, cz));
				else
					stock->setCenter(cutsim::GLVertex(cx, cy, cz));
				stock->setRotationCenter(cutsim::GLVertex(rx, ry, rz));
				stock->setAngle(cutsim::GLVertex(ra*(PI/ 180.0), rb*(PI/ 180.0), rc*(PI/ 180.0)));
				if (parts)
					stock->setColor(PARTS_COLOR);
				else
					stock->setColor(STOCK_COLOR);
				stock->calcBB();
				stockVolume->stock = stock;
				stockVolume->operation = operation;
				myStocks.push_back(stockVolume);
				message += " OK";
				break;
			}
			case cutsim::	CYLINDER_VOLUME: {
				StockVolume *stockVolume = new StockVolume();
				cutsim::CylinderVolume* stock = new cutsim::CylinderVolume();
				stock->setRadius(radius);
				stock->setLength(length);
				stock->setCenter(cutsim::GLVertex(cx, cy, cz));
				stock->setRotationCenter(cutsim::GLVertex(rx, ry, rz));
				stock->setAngle(cutsim::GLVertex(ra*(PI/ 180.0), rb*(PI/ 180.0), rc*(PI/ 180.0)));
				if (parts)
					stock->setColor(PARTS_COLOR);
				else
					stock->setColor(STOCK_COLOR);
				stock->calcBB();
				stockVolume->stock = stock;
				stockVolume->operation = operation;
				myStocks.push_back(stockVolume);
				message += " OK";
				break;
			}
			case cutsim::	SPHERE_VOLUME: {
				StockVolume *stockVolume = new StockVolume();
				cutsim::SphereVolume* stock = new cutsim::SphereVolume();
				stock->setRadius(radius);
				stock->setCenter(cutsim::GLVertex(cx, cy, cz));
				if (parts)
					stock->setColor(PARTS_COLOR);
				else
					stock->setColor(STOCK_COLOR);
				stockVolume->stock = stock;
				stockVolume->operation = operation;
				myStocks.push_back(stockVolume);
				message += " OK";
				break;
			}
			case cutsim::STL_VOLUME: {
				StockVolume *stockVolume = new StockVolume();
				cutsim::StlVolume* stock = new cutsim::StlVolume();
				stock->setCenter(cutsim::GLVertex(cx, cy, cz));
				stock->setRotationCenter(cutsim::GLVertex(rx, ry, rz));
				stock->setAngle(cutsim::GLVertex(ra*(PI/ 180.0), rb*(PI/ 180.0), rc*(PI/ 180.0)));
				int error;
				error = stock->readStlFile(path);
				if (error == 0) {
					if (parts)
						stock->setColor(PARTS_COLOR);
					else
						stock->setColor(STOCK_COLOR);
//					stock->calcBB();
					stockVolume->stock = stock;
					stockVolume->operation = operation;
					myStocks.push_back(stockVolume);
					message += " OK";
				} else {
					message = tr("STL File read Error @line %1").arg(line_count);
					error_count++;
				}
				break;
			}
			default: ;
			}
			debugMessage(message);
			break;
		} else {
			message = tr("END Error @line %1").arg(line_count);
		 error_count++;
		}
		}
   debugMessage(message);
    }
	lineNo = line_count;
	return error_count;
}

/// create stock & parts
void CutsimWindow::createStockParts() {
	for (int i = 0; i < (int)myStocks.size(); i++) {
//std::cout << "operation type: " << myStocks[i]->operation << " " << i << "\n";
		switch (myStocks[i]->operation) {
		case SUM_OPERATION:
//std::cout << "sum oeration\n";
			myCutsim->sum_volume(myStocks[i]->stock);
			break;
		case DIFF_OPERATION:
			myCutsim->diff_volume(myStocks[i]->stock);
			break;
		case INTERSECT_OPERATION:
			myCutsim->intersect_volume(myStocks[i]->stock);
			break;
		default: ;
		}
		delete myStocks[i]->stock;
		delete myStocks[i];
	}
	myStocks.clear();
}

///find the machine spec file. uses QSettings, to preselect the last file used
void CutsimWindow::chooseMachineSpecFile(bool forcechoose) {
    QString path;
    bool ttconf = settings.contains("cutsim/machine");
    if (ttconf) {
        //passing the file name as the path means that it is preselected
        path = settings.value("cutsim/machine").toString();
    } else {
        path = "/usr/share/doc/emc2/examples/sample-configs/sim"; //location when installed
    }
    if (forcechoose || !QFileInfo(path).exists())
    	path = QFileDialog::getOpenFileName ( this, "Locate setup file", path, "Machine Spec. File (*.mspec)" );

    if (!QFileInfo(path).exists()){
        debugMessage("Error: Machine Spec. file does not exist!");
        return;
    }
    settings.setValue("cutsim/machine", path);

    int error_count;
    debugMessage( tr("Read Machine Spec. file: %1").arg(path) );
    error_count = readMachineSpecFile(path);
    if (error_count)
			debugMessage( tr("%1 Error for reading Machine Spec. file").arg(error_count) );
    else
    	debugMessage( tr("Successflly read Machine Spec. file") );
}

/// read machine spec. file and set
int CutsimWindow::readMachineSpecFile(QString file) {
	int error_count = 0;
	int line_count = 0;
	QFile specFileHandle( file );
	QString line, message;
	double vlimit;

	if ( specFileHandle.open( QIODevice::ReadOnly | QIODevice::Text) ) {
		QTextStream in( &specFileHandle );
		while ( !in.atEnd() ) {
			line = in.readLine();         // line of text excluding '\n'
			line_count++;
			if (line.size() == 0) continue;
			lex_analyzer::LexAnalyzer lex(line.toStdString());
		message = line;
			if (lex.wordMatch("MAX_X_LIMIT", 0)) {
				vlimit = lex.token2d(1);
				if (vlimit == NAN) {
					message = tr("MAX_X_LIMIT Error @line %1").arg(line_count);
					error_count++;
				} else
					myMachine->max_x_limit = vlimit;
			}
			if (lex.wordMatch("MIN_X_LIMIT", 0)) {
				vlimit = lex.token2d(1);
				if (vlimit == NAN) {
					message = tr("MIN_X_LIMIT Error @line %1").arg(line_count);
					error_count++;
				} else
					myMachine->min_x_limit = vlimit;
			}
			if (lex.wordMatch("MAX_Y_LIMIT", 0)) {
				vlimit = lex.token2d(1);
				if (vlimit == NAN) {
					message = tr("MAX_Y_LIMIT Error @line %1").arg(line_count);
					error_count++;
				} else
					myMachine->max_y_limit = vlimit;
			}
			if (lex.wordMatch("MIN_Y_LIMIT", 0)) {
				vlimit = lex.token2d(1);
				if (vlimit == NAN) {
					message = tr("MIN_Y_LIMIT Error @line %1").arg(line_count);
					error_count++;
				} else
					myMachine->min_y_limit = vlimit;
			}
			if (lex.wordMatch("MAX_Z_LIMIT", 0)) {
				vlimit = lex.token2d(1);
				if (vlimit == NAN) {
					message = tr("MAX_Z_LIMIT Error @line %1").arg(line_count);
					error_count++;
				} else
					myMachine->max_z_limit = vlimit;
			}
			if (lex.wordMatch("MIN_Z_LIMIT", 0)) {
				vlimit = lex.token2d(1);
				if (vlimit == NAN) {
					message = tr("MIN_Z_LIMIT Error @line %1").arg(line_count);
					error_count++;
				} else
					myMachine->min_z_limit = vlimit;
			}
			if (lex.wordMatch("MAX_A_LIMIT", 0)) {
				vlimit = lex.token2d(1);
				if (vlimit == NAN) {
					message = tr("MAX_A_LIMIT Error @line %1").arg(line_count);
					error_count++;
				} else
					myMachine->max_a_limit = vlimit * PI / 180.0;
			}
			if (lex.wordMatch("MIN_A_LIMIT", 0)) {
				vlimit = lex.token2d(1);
				if (vlimit == NAN) {
					message = tr("MIN_A_LIMIT Error @line %1").arg(line_count);
					error_count++;
				} else
					myMachine->min_a_limit = vlimit * PI / 180.0;
			}
			if (lex.wordMatch("MAX_B_LIMIT", 0)) {
				vlimit = lex.token2d(1);
				if (vlimit == NAN) {
					message = tr("MAX_B_LIMIT Error @line %1").arg(line_count);
					error_count++;
				} else
					myMachine->max_b_limit = vlimit * PI / 180.0;
			}
			if (lex.wordMatch("MIN_B_LIMIT", 0)) {
				vlimit = lex.token2d(1);
				if (vlimit == NAN) {
					message = tr("MIN_B_LIMIT Error @line %1").arg(line_count);
					error_count++;
				} else
					myMachine->min_b_limit = vlimit * PI / 180.0;
			}
			if (lex.wordMatch("MAX_C_LIMIT", 0)) {
				vlimit = lex.token2d(1);
				if (vlimit == NAN) {
					message = tr("MAX_C_LIMIT Error @line %1").arg(line_count);
					error_count++;
				} else
					myMachine->max_c_limit = vlimit * PI / 180.0;
			}
			if (lex.wordMatch("MIN_C_LIMIT", 0)) {
				vlimit = lex.token2d(1);
				if (vlimit == NAN) {
					message = tr("MIN_C_LIMIT Error @line %1").arg(line_count);
					error_count++;
				} else
					myMachine->min_c_limit = vlimit * PI / 180.0;
			}
			if (lex.wordMatch("MAX_FEED_RATE", 0)) {
				vlimit = lex.token2d(1);
				if (vlimit == NAN || vlimit <= 0.0) {
					message = tr("MAX_FEED_RATE Error @line %1").arg(line_count);
					error_count++;
				} else
					myMachine->max_feed_rate = vlimit;
			}
			if (lex.wordMatch("MAX_SPINDLE_POWER", 0)) {
				vlimit = lex.token2d(1);
				if (vlimit == NAN || vlimit <= 0.0) {
					message = tr("MAX_SPINDLE_POWER Error @line %1").arg(line_count);
					error_count++;
				} else
					myMachine->max_spindle_power = vlimit;
			}
			if (lex.wordMatch("TRAVERSE_FEED_RATE", 0)) {
				vlimit = lex.token2d(1);
				if (vlimit == NAN || vlimit <= 0.0) {
					message = tr("TRAVERSE_FEED_RATE Error @line %1").arg(line_count);
					error_count++;
				} else
					myMachine->traverse_feed_rate = vlimit;
			}
			if (lex.wordMatch("HOLDER_RADIUS", 0)) {
				vlimit = lex.token2d(1);
				if (vlimit == NAN || vlimit <= 0.0) {
					message = tr("HOLDER_RADIUS Error @line %1").arg(line_count);
					error_count++;
				} else
					myMachine->holderradius = vlimit;
			}
			if (lex.wordMatch("HOLDER_LENGTH", 0)) {
				vlimit = lex.token2d(1);
				if (vlimit == NAN || vlimit <= 0.0) {
					message = tr("HOLDER_LENGTH Error @line %1").arg(line_count);
					error_count++;
				} else
					myMachine->holderlength = vlimit;
			}
			if (lex.wordMatch("SPINDLE_RADIUS", 0)) {
				vlimit = lex.token2d(1);
				if (vlimit == NAN || vlimit <= 0.0) {
					message = tr("SPINDLE_RADIUS Error @line %1").arg(line_count);
					error_count++;
				} else {
					myMachine->spindleradius = vlimit;
					myGLWidget->setSpindleRadius(myMachine->spindleradius);
				}
			}
			if (lex.wordMatch("SPINDLE_LENGTH", 0)) {
				vlimit = lex.token2d(1);
				if (vlimit == NAN || vlimit <= 0.0) {
					message = tr("SPINDLE_LENGTH Error @line %1").arg(line_count);
					error_count++;
				} else {
					myMachine->spindlelength = vlimit;
					myGLWidget->setSpindleLength(myMachine->spindlelength);
				}
			}
			if (lex.wordMatch("SCENE_RADIUS", 0)) {
				int radius = lex.token2i(1);
				if (radius == INT_MIN || radius <= 0) {
					message = tr("SCENE_RADIUS Error @line %1").arg(line_count);
					error_count++;
				} else
					myGLWidget->setSceneRadius(radius);
			}
            debugMessage(message);
		}
	} else {
		debugMessage("Error: Can't open:" + file);
		return ++error_count;
	}

	specFileHandle.close();

    qDebug() << "readMachineSpecFile error:" << error_count << "\n";

	return error_count;
}
