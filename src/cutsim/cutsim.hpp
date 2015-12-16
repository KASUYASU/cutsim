/*  
 *  Copyright 2010-2011 Anders Wallin (anders.e.e.wallin "at" gmail.com)
 *  Copyright 2015      Kazuyasu Hamada (k-hamada@gifu-u.ac.jp)
 *  
 *  This file is part of OpenCAMlib.
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

#ifndef CUTSIM_H
#define CUTSIM_H

#include <QObject>
#include <QRunnable>

#include <string>
#include <iostream>
#include <cmath>
#include <vector>
#include <ctime>

#include <boost/bind.hpp>
#include <boost/timer.hpp>

#include "octree.hpp"
#include "octnode.hpp"
#include "volume.hpp"
#include "marching_cubes.hpp"
#include "cube_wireframe.hpp"
#include "gldata.hpp"
#include "glwidget.hpp"

#include <g2m/g2m.hpp>
#include <g2m/gplayer.hpp>

namespace cutsim {

/// Task to diff a volume from the tree
class DiffTask : public QObject, public QRunnable  {
    Q_OBJECT

public:
    /// create task for cutting Volume from Octree which is drawn with GLData
    DiffTask(Octree* t, GLData* g, const Volume* v, int l, int ms, double fr ) : tree(t), gld(g), vol(v), line(l), mstatus(ms), feedrate(fr) { }
    /// run the task
    void run() {
    	CuttingStatus cstatus;
        qDebug() << "DiffTask thread" << QThread::currentThread();
        std::clock_t start, stop;
        start = std::clock();
//        tree->diff( vol );
        cstatus = tree->diff_c( vol );
        stop = std::clock();
        qDebug() << "   " << ( ( stop - start ) / (double)CLOCKS_PER_SEC ) ;
        qDebug() << "DiffTask thread DONE " << QThread::currentThread();

qDebug() << "Cutting Count: " << cstatus.cutcount << "cut(s)" << ((mstatus & (g2m::POSITIVE_PLUNGE | g2m::NEGATIVE_PLUNGE)) ? "plunge" : "") << "@ line:" << line;

        int error = 0;
        if (cstatus.cutcount)
        	error = (mstatus & (g2m::OFF | g2m::BRAKE | g2m::TRAVERSE));

        emit signalDone(line, mstatus, cstatus.collision | error, cstatus.cutcount * feedrate);
    }

signals:
    /// emitted when the task is done
    void signalDone(int line, int mstatus, int error, double cuttingPower);

private:
    Octree* tree;
    GLData* gld;
    const Volume* vol;
    int line;
    int mstatus;
    double feedrate;
};

/// task for updating GLData of an Octree
class UpdateGLTask : public QObject, public QRunnable  {
    Q_OBJECT

public:
    /// Create task for updating GLData based on given Octree. Use the given IsoSurfaceAlgorithm for the update
    UpdateGLTask(Octree* t, GLData* g, IsoSurfaceAlgorithm* ia, GLWidget* wid) : tree(t), gld(g), iso_algo(ia), widget(wid) { }
    /// run the task
    void run() {
        qDebug() << "UpdateGLTask thread" << QThread::currentThread();
        std::clock_t start, stop;
        start = std::clock();
        if (widget->doAnimate()) {
        	iso_algo->updateGL();
        	gld->swap();
        }
        stop = std::clock();
        qDebug() << "   " << ( ( stop - start ) / (double)CLOCKS_PER_SEC ) ;
        //std::cout << "cutsim.cpp updateGL() : " << ( ( stop - start ) / (double)CLOCKS_PER_SEC ) <<'\n';
        emit signalDone();
    }

signals:
    /// emitted when current move done
    void signalDone();

private:
    Octree* tree;
    GLData* gld;
    IsoSurfaceAlgorithm* iso_algo;
    GLWidget* widget;
}; 

/// a Cutsim stores an Octree stock model, uses an iso-surface extraction
/// algorithm to generate surface triangles, and communicates with
/// the corresponding GLData surface which is used by GLWidget for rendering
class Cutsim : public QObject {
    Q_OBJECT

public:
    /// create a cutting simulation
    /// \param octree_size side length of the depth=0 octree cube
    /// \param octree_max_depth maximum sub-division depth of the octree
    /// \param gld the GLData used to draw this tree
    Cutsim(double octree_size, unsigned int octree_max_depth, GLVertex* octree_center, GLData* gld, GLWidget* widget);
    virtual ~Cutsim();
    /// subtract/diff given Volume
    void diff_volume( const Volume* vol );
    /// sum/union given Volume
    void sum_volume( const Volume* vol );
    /// intersect/"and" given Volume
    void intersect_volume( const Volume* vol );
    /// update the GL-data
    void updateGL();

signals:
    /// emitted when diff is done
    void signalDiffDone(int line, int mstatus, int error, double cuttingPower);
    /// emitted when GL update is done
    void signalGLDone();

public slots:
    /// call to signal that diff-task is done
    void slotDiffDone(int line, int mstatus, int error, double cuttingPower) {
        qDebug() << " Cutsim::slotDiffDone() ";
        emit signalDiffDone(line, mstatus, error, cuttingPower);
    }
    /// call to signal that gl-task is done
    void slotGLDone() {
        emit signalGLDone();
    }
    /// diff the given Volume from the stock
    void slot_diff_volume( const Volume* vol) { diff_volume(vol);}
    /// multithreaded diff (FIXME: broken)
    void slot_diff_volume_mt( const Volume* vol, int line, int mstatus, double feedrate ) {
        DiffTask* dt = new DiffTask(tree, g, vol, line, mstatus, feedrate);
        connect( dt, SIGNAL( signalDone(int,int,int,double) ), this, SLOT( slotDiffDone(int,int,int,double) ) );
        QThreadPool::globalInstance()->start(dt);
        
QThreadPool::globalInstance()->waitForDone();
    }
    /// multithreaded GL-update (broken...)
    void update_gl_mt( ) { 
        UpdateGLTask* ua = new UpdateGLTask(tree, g, iso_algo, widget);
        connect( ua, SIGNAL( signalDone() ), this, SLOT( slotGLDone() ) );
        QThreadPool::globalInstance()->start(ua);
        
QThreadPool::globalInstance()->waitForDone();
    }
    /// sum given Volume to tree
    void slot_sum_volume( const Volume* vol)  { sum_volume(vol);} 
    /// intersect three with volume
    void slot_int_volume( const Volume* vol)  { intersect_volume(vol);}

private:
    IsoSurfaceAlgorithm* iso_algo; // the isosurface-extraction algorithm to use
    Octree* tree; // this is the stock model
    GLData* g; // this is the graphics object drawn on the screen, representing the stock
    GLWidget* widget;
};

} // end namespace

#endif
