#ifndef MYVIEWER_H
#define MYVIEWER_H

// Mesh stuff:
#include "Mesh.h"
#include "tetgenhandler.h"

// Parsing:
#include "BasicIO.h"

// opengl and basic gl utilities:
#include <gl/openglincludeQtComp.h>
#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLFunctions>
#include <QGLViewer/qglviewer.h>
#include <gl/GLUtilityMethods.h>

// Qt stuff:
#include <QColorDialog>
#include <QFileDialog>
#include <QKeyEvent>



class MyViewer : public QGLViewer , public QOpenGLFunctions_4_3_Core
{
    Q_OBJECT

    Mesh mesh;
    TetGenHandler tetmesh;

    std::vector<Triplet> pointSet;

public :

    MyViewer(QGLWidget * parent = NULL) : QGLViewer(parent) , QOpenGLFunctions_4_3_Core() {
        setWindowTitle(QString("Our cool project. PRESS 'H' FOR HELP!"));
    }


    void draw() {
        glEnable( GL_LIGHTING );
        glColor3f(0.5,0.5,0.8);
        glBegin(GL_TRIANGLES);
        for( unsigned int t = 0 ; t < mesh.triangles.size() ; ++t ) {
            point3d const & p0 = mesh.vertices[ mesh.triangles[t][0] ].p;
            point3d const & p1 = mesh.vertices[ mesh.triangles[t][1] ].p;
            point3d const & p2 = mesh.vertices[ mesh.triangles[t][2] ].p;
            point3d const & n = point3d::cross( p1-p0 , p2-p0 ).direction();
            glNormal3f(n[0],n[1],n[2]);
            glVertex3f(p0[0],p0[1],p0[2]);
            glVertex3f(p1[0],p1[1],p1[2]);
            glVertex3f(p2[0],p2[1],p2[2]);
        }
        glEnd();
    }

    void pickBackgroundColor() {
        QColor _bc = QColorDialog::getColor( this->backgroundColor(), this);
        if( _bc.isValid() ) {
            this->setBackgroundColor( _bc );
            this->updateGL();
        }
    }

    void adjustCamera( point3d const & bb , point3d const & BB ) {
        point3d const & center = ( bb + BB )/2.f;
        setSceneCenter( qglviewer::Vec( center[0] , center[1] , center[2] ) );
        setSceneRadius( 1.5f * ( BB - bb ).norm() );
        showEntireScene();
    }


    void init() {
        makeCurrent();
        initializeOpenGLFunctions();

        setMouseTracking(true);// Needed for MouseGrabber.

        setBackgroundColor(QColor(255,255,255));

        // Lights:
        GLTools::initLights();
        GLTools::setSunsetLight();
        GLTools::setDefaultMaterial();

        //
        glShadeModel(GL_SMOOTH);
        glFrontFace(GL_CCW); // CCW ou CW

        glEnable(GL_DEPTH);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glEnable(GL_CLIP_PLANE0);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_COLOR_MATERIAL);

        //
        setSceneCenter( qglviewer::Vec( 0 , 0 , 0 ) );
        setSceneRadius( 10.f );
        showEntireScene();
    }

    QString helpString() const {
        QString text("<h2>Our cool project</h2>");
        text += "<p>";
        text += "This is a research application. Be careful.";
        text += "<h3>Participants</h3>";
        text += "<ul>";
        text += "<li>Maud Lastic</li>";
        text += "<li>Alban Gauthier</li>";
        text += "</ul>";
        text += "<h3>Basics</h3>";
        text += "<p>";
        text += "<ul>";
        text += "<li>H   :   make this help appear</li>";
        text += "<li>O   :   open a mesh</li>";
        text += "<li>Ctrl + mouse right button double click   :   choose background color</li>";
        text += "</ul>";
        return text;
    }

    void keyPressEvent( QKeyEvent * event ) {
        if( event->key() == Qt::Key_H ) {
            help();
        }
        else if( event->key() == Qt::Key_O ) {
            // exemple of use of keyboard
            open_mesh();
        }
    }

    void mouseDoubleClickEvent( QMouseEvent * e ) {
        if( (e->modifiers() & Qt::ControlModifier)  &&  (e->button() == Qt::RightButton) ) {
            pickBackgroundColor();
            return;
        }

        QGLViewer::mouseDoubleClickEvent( e );
    }

    void mousePressEvent(QMouseEvent* e ) {
        if( ( e->modifiers() & Qt::ShiftModifier ) ) {
            // exemple of use of modifiers. usually: Qt::ShiftModifier , Qt::CtrlModifier
            // do something here
            update(); // update the rendering after interaction
            return;
        }

        QGLViewer::mousePressEvent(e);
    }

    void mouseMoveEvent(QMouseEvent* e  ){
        QGLViewer::mouseMoveEvent(e);
    }

    void mouseReleaseEvent(QMouseEvent* e  ) {
        QGLViewer::mouseReleaseEvent(e);
    }

public slots:
    void open_mesh() {
        bool success = false;
        QString fileName = QFileDialog::getOpenFileName(NULL,"","");
        if ( !fileName.isNull() ) { // got a file name
            if(fileName.endsWith(QString(".off")))
                success = OFFIO::openTriMesh(fileName.toStdString() , mesh.vertices , mesh.triangles );
            else if(fileName.endsWith(QString(".obj")))
                success = OBJIO::openTriMesh(fileName.toStdString() , mesh.vertices , mesh.triangles );

            if(success) {
                point3d bb(FLT_MAX,FLT_MAX,FLT_MAX) , BB(FLT_MIN,FLT_MIN,FLT_MIN);
                for(unsigned int v = 0 ; v < mesh.vertices.size() ; ++v) {
                    bb = point3d::min(bb , mesh.vertices[v].p);
                    BB = point3d::max(BB , mesh.vertices[v].p);
                }
                adjustCamera( bb , BB );
                update();
            }
        }
    }
};




#endif // MYVIEWER_H
