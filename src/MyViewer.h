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

    //KDTree
    BBox computeBoundingBox(std::vector<Triplet> points){
        BBox B;
        B.xMin = points[0].p[0];
        B.xMax = points[0].p[0];
        B.yMin = points[0].p[1];
        B.yMax = points[0].p[1];
        B.zMin = points[0].p[2];
        B.zMax = points[0].p[2];

        for (int i = 1 ; i<points.size() ; i++){
            if (points[i].p[0] < B.xMin){
                B.xMin = points[i].p[0];
            }
            else if (points[i].p[0] > B.xMax){
                B.xMax = points[i].p[0];
            }

            if (points[i].p[1] < B.yMin){
                B.yMin = points[i].p[1];
            }
            else if (points[i].p[1] > B.yMax){
                B.yMax = points[i].p[1];
            }

            if (points[i].p[2] < B.zMin){
                B.zMin = points[i].p[2];
            }
            else if (points[i].p[2] > B.zMax){
                B.zMax = points[i].p[2];
            }
        }
    }

    std::vector<Triplet> sortPtsAlongAxis(std::vector<Triplet> points, int axis){
        std::vector<Triplet> sortedPoints;
        int n = points.size();
        for (int i = 0 ; i<n ; i++){
            int minIndex = 0;
            int minCoord = points[0].p[axis];
            for (int j = 1 ; j<points.size() ; j++){
                if (points[j].p[axis] < minCoord){
                    minIndex = j;
                    minCoord = points[j].p[axis];
                }
            }
            sortedPoints.push_back(points[minIndex]);
            points.erase(points.begin() + minIndex);
        }
        return sortedPoints;
    }

    int findMaxAxis(BBox B){
        int longestAxis = 0;
        if (B.yMax-B.yMin > B.xMax-B.xMin){
            longestAxis = 1;
        }
        if (B.zMax-B.zMin > B.xMax-B.xMin || B.zMax-B.zMin > B.yMax-B.yMin){
            longestAxis = 2;
        }
        return longestAxis;
    }

    point3d findMedianSample(std::vector<Triplet> sortedPts){
        if (sortedPts.size() % 2 == 1){
            return sortedPts[(sortedPts.size()-1)/2].p;
        }
        else{
            return (sortedPts[sortedPts.size()/2].p + sortedPts[sortedPts.size()/2 + 1].p)/2;
        }
    }

    point3d findMedianNormal(std::vector<Triplet> sortedPts){
        if (sortedPts.size() % 2 == 1){
            return sortedPts[(sortedPts.size()-1)/2].n;
        }
        else{
            return (sortedPts[sortedPts.size()/2].n + sortedPts[sortedPts.size()/2 + 1].n)/2;
        }
    }

    KDNode buildKDTree(std::vector<Triplet> points){
        if (points.size() == 1){
            KDNode n;
            n.data = points[0];
            n.leftChild = NULL;
            n.rightChild = NULL;
            return n;
        }

        BBox B = computeBoundingBox(points);
        int maxAxis = findMaxAxis(B);
        std::vector<Triplet> sortedPts = sortPtsAlongAxis(points, maxAxis);
        point3d q = findMedianSample(sortedPts);
        point3d o = findMedianNormal(sortedPts);

        std::vector<Triplet> upperPartition;
        std::vector<Triplet> lowerPartition;
        for (int i = 0 ; i<sortedPts.size() ; i++){
            if (i < sortedPts.size()/2){
                lowerPartition.push_back(sortedPts[i]);
            }
            else{
                upperPartition.push_back(sortedPts[i]);
            }
        }

        KDNode n;
        n.data.area = 1;
        n.data.p = q;
        n.data.n = o;
        KDNode left = buildKDTree(upperPartition);
        KDNode right = buildKDTree(lowerPartition);
        KDNode *leftC = new KDNode(left);
        KDNode *rightC = new KDNode(right);
        n.leftChild = leftC;
        n.rightChild = rightC;
        return n;
    }

    //Initialization
    void createPointSet(){
        for (int i = 0 ; i<mesh.triangles.size() ; i++){
            point3d p0 = mesh.vertices[mesh.triangles[i][0]].p;
            point3d p1 = mesh.vertices[mesh.triangles[i][1]].p;
            point3d p2 = mesh.vertices[mesh.triangles[i][2]].p;
            Triplet t;
            t.area = 1;
            t.p = p0/3+p1/3+p2/3;
            t.n = point3d::cross(p1-p0, p2-p0);
            pointSet.push_back(t);
        }
    }


    void mainFunction(){
        createPointSet();
        std::cout << "PointSet created : " << pointSet.size() << " points" << std::endl;
        KDNode tree = buildKDTree(pointSet);
        std::cout << "KDTree created : " << std::endl;
    }

    //Draw
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
        else if( event->key() == Qt::Key_M ) {
            mainFunction();
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
