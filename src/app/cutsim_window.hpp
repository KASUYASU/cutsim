#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
//#include <QPluginLoader>
//#include <QMutex>

#include <cutsim/cutsim.hpp>
#include <cutsim/glwidget.hpp>
#include <cutsim/machine.hpp>

#include <g2m/g2m.hpp>
#include <g2m/gplayer.hpp>

#include "version_string.hpp"
#include "text_area.hpp"

class QAction;
class QLabel;
class QMenu;

typedef enum {
			NO_OPERATION = 0,
			SUM_OPERATION = 1,
			DIFF_OPERATION = 2,
			INTERSECT_OPERATION = 3,
		} OperationType;

class StockVolume {
public:
			StockVolume() {};
			virtual ~StockVolume() {};
			cutsim::Volume*	stock;
			int operation;
} ;

/// the main application window for the cutting-simulation
/// this includes menus, toolbars, text-areas for g-code and canon-lines and debug
/// the 3D view of tool/stock.
class CutsimWindow : public QMainWindow {
    Q_OBJECT

public:
    /// create window
    CutsimWindow(QStringList ags);
    ~CutsimWindow();
    void findInterp();
    void chooseToolTable();
    void chooseSetupFile();
    void chooseMachineSpecFile(bool forcechoose = false);
    /// return helpAction
    QAction* getHelpMenu() { return helpAction; };
    /// return the n'th argument given to the constructor
    QString getArg(int n) {if(n<=args.count())return args[n];else return "n--";}
    /// return all arguments
    QStringList* getArgs() {return &args;}

public slots:
    /// set progress value (0..100)
    void slotSetProgress(int n, int line, double time, bool force) { myProgress->setValue(n);
    	if (force) { playAction->setEnabled(true);
    		myCutsim->updateGL();
    		bool pre_status;
    		if ((pre_status = myGLWidget->doAnimate()) == false)
    			myGLWidget->setAnimate(true);
    		myGLWidget->reDraw();
    		myGLWidget->setAnimate(pre_status);
    	} else
    		myGLWidget->reDraw();
{
static int preline = 0;
int gcode_line = myG2m->toGcodeLineNo(line);
if (line != preline) {
QString o = QString("Line : %1").arg(gcode_line)
+ QString("  Evaluated Time %1:%L2").arg((int)time / 60).arg((int)time % 60, 2, 10, QLatin1Char('0'));
myStatus->setText(o);
canonText->setCursor(line+1);
gcodeText->setCursor(gcode_line);
preline = line;
}
}
    }
    /// show a debug message
    void debugMessage(QString s) { debugText->appendLine(s); debugText->setCursor(debugText->getLineSize()); }
    /// show a message in the status bar
    void statusBarMessage(QString s) { statusBar()->showMessage(s); }
    /// add a Qstring line to the g-code window
    void appendGcodeLine(QString s) { gcodeText->appendLine(s); }
    /// add a Qstring line to the canon-line window
    void appendCanonLine(QString s) { canonText->appendLine(s); }
    /// position tool
#ifdef MULTI_AXIS
    void slotSetToolPosition(double x, double y, double z, double a, double b, double c, int line, int mstatus, double feedrate);
#else
    void slotSetToolPosition(double x, double y, double z, int line, int mstatus, double feedrate);
#endif
    /// change the tool
    void slotToolChange(int t);
    /// slot called by worker tasks when a diff-operation is done
    void slotDiffDone(int line, int mstatus, int error, double cuttingPower);
    /// slot called by GL-thread when GL is updated
    void slotGLDone();

signals:
    /// signal other objects (g2m) with the path to the g-code file
    void setGcodeFile(QString f);
    /// signal to g2m with the path to the rs274 binary
    void setRS274(QString s);
    /// signal to g2m the path to the tootable
    void setToolTable(QString s);
    /// signal to g2m to start interpreting
    void interpret();
    /// play signal to Gplayer
    void play();
    /// pause signal to Gplayer
    void pause();
    /// stop signal to Gplayer
    void stop();
    /// emitted when the current move is done and we can request a enw one
    void signalMoveDone();

private slots:
    void newFile() { statusBar()->showMessage(tr("Invoked File|New")); }
    void open();
    void save(){
        statusBar()->showMessage(tr("Invoked File|Save"));
    }
    void runProgram() {
        statusBar()->showMessage(tr("Running program..."));
        playAction->setDisabled(true);
        emit play();
    }
    void pauseProgram() {
        statusBar()->showMessage(tr("Pause program."));
        emit pause();
        playAction->setEnabled(true);
    }
    void stopProgram() {
        statusBar()->showMessage(tr("Stop program."));
        emit stop();
        playAction->setEnabled(true);
    }
    void about() {
        statusBar()->showMessage(tr("Invoked Help|About"));
        QMessageBox::about(this, tr("About Menu"),
            tr("<b>cutsim</b> is an open-source cutting simulation. <p>(C) 2011 Anders Wallin. <p>(C) 2015 Kazuyasu Hamada."));
    }
    void statusMessage (const QString aMessage) {
        statusBar()->showMessage(aMessage);
    }

private:
    void createDock();
    void createToolBar();
    void createActions();
    void createMenus();

    // read tool table and set
    int readToolTable(QString file);
    // read setup file and set
    int readSetupFile(QString file);
    // read stock file and set
    int readStockFile(QTextStream &in, bool parts, int& lineNo);
    // create stock & parts
    void createStockParts();
    // read machine spec. file and set
    int readMachineSpecFile(QString file);

    QMenu* fileMenu;
    QMenu* helpMenu;
    QAction* helpAction;
    QAction* newAction;
    QAction* openAction;
    QAction* exitAction;
    QAction* aboutAction;
    QAction* playAction;
    QAction* pauseAction;
    QAction* stopAction;
    
    QProgressBar* myProgress;
    QToolBar* myToolBar;
    cutsim::Cutsim* myCutsim;
    
    cutsim::GLWidget* myGLWidget;
    
    std::vector<cutsim::CutterVolume*> myTools;

    unsigned int currentTool;
    g2m::g2m* myG2m;
    g2m::GPlayer* myPlayer;
    TextArea* debugText;
    TextArea* gcodeText;
    TextArea* canonText;
    QStringList args;
    QString myLastFolder;
    QSettings settings;
    QLabel* myStatus;
    cutsim::Machine* myMachine;
    double octree_cube_size;
    unsigned int max_depth;
    cutsim::GLVertex* octree_center;
    double cube_resolution;
    double specific_cutting_force;
    double powerCoff;
    double requiredPower;

    std::vector<StockVolume*> myStocks;

	double step_size;
	bool   variable_step_mode;
};

#endif
