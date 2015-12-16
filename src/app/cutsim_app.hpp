#ifndef APPLICATION_H
#define APPLICATION_H

#include <QtCore/QObject>
#include <QtGui/QApplication>
#include <QtGui/QPixmap>
#include <QtGui/QSplashScreen>

#include <QtCore/QEvent>
#include <QtCore/QDataStream>
#include <QtGui/QCursor>
#include <QtCore/QWaitCondition>
#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtCore/QDir>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

#include <iostream>

/// the QApplication 
class CutsimApplication : public QApplication {
    Q_OBJECT
public:
    /// create QApplication
    CutsimApplication( int &argc, char **argv, int = QT_VERSION ) :
        QApplication ( argc, argv ),
        mySplash ( NULL ) {
    }
    ~CutsimApplication( ) {
        if ( mySplash ) {
            delete mySplash;
            mySplash = NULL;
        }
    }
    /// show a splash-screen (not used currently)
    void splashScreen ( const QPixmap &pixmap = QPixmap() ) {
        
        QPixmap* p = ( QPixmap* ) &pixmap;
        if ( p->isNull() ) {
            p = new QPixmap ( "images/qoccsplash.png" );
        }
        mySplash = new QSplashScreen ( *p, Qt::WindowStaysOnTopHint );
        if ( mySplash ) {
            mySplash->show();
            splashMessage ( tr ( "Initializing Application..." ), Qt::AlignRight | Qt::AlignTop );
        }
    }
    /// show a splash-message
    void splashMessage ( const QString &message, 
                         int alignment = Qt::AlignLeft,
                         const QColor &color = Qt::black ) {
        if ( mySplash )
            mySplash->showMessage ( message, alignment, color );
        
    }
    /// finish showing splash screen
    void splashFinish ( QWidget* w, long millisecs ) {
        if ( mySplash ) {
            msleep ( millisecs );
            mySplash->finish( w );
            delete mySplash;
            mySplash = NULL;
        }
    }
    /// sleep (?)
    void msleep ( unsigned long millisecs ) {
        QMutex mutex;
        QWaitCondition waitCondition;
        mutex.lock();
        waitCondition.wait ( &mutex, millisecs );
        mutex.unlock();
    }
    
private:
    QSplashScreen* mySplash;
//bool notify(QObject *receiver_, QEvent *event_);
bool notify(QObject *receiver_, QEvent *event_)
{
try
{
return QApplication::notify(receiver_, event_);
}
catch (std::exception &ex)
{
std::cout << "std::exception was caught" << std::endl;
}

return false;
}

};

#endif // APPLICATION_H
