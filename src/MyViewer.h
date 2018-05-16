#ifndef MYVIEWER_H
#define MYVIEWER_H

// Mesh stuff:
#include "Mesh.h"
#include "tetgenhandler.h"

//KDTree
#include "KDTree.h"

// Parsing:
#include "BasicIO.h"

// opengl and basic gl utilities:
#include <gl/openglincludeQtComp.h>
#include <QOpenGLFunctions_3_0>
#include <QOpenGLFunctions>
#include <QGLViewer/qglviewer.h>
#include <gl/GLUtilityMethods.h>

// Qt stuff:
#include <QColorDialog>
#include <QFileDialog>
#include <QKeyEvent>

static bool showKDTree = false;
static int mode = 0;

class MyViewer : public QGLViewer , public QOpenGLFunctions_3_0
{
    Q_OBJECT

    KDTree tree;
    Mesh mesh;
    TetGenHandler tetmesh;
    std::vector<Triplet> pointSet;
    std::vector<double> windingNumbers;
    int kdTreeDisplayDepth = 0;
    double lambda = 0.0;

public :

    MyViewer(QGLWidget * parent = NULL) : QGLViewer(parent) , QOpenGLFunctions_3_0() {
        setWindowTitle(QString("Our cool project. PRESS 'H' FOR HELP!"));
    }

    std::vector< point3d > fromMeshToPointSet(Mesh m, std::vector< Triplet > & TripletList) {
        std::vector< point3d > pointCloud;
        Triplet triplet;
        for( unsigned int t = 0 ; t < m.triangles.size() ; ++t ) {
            point3d const & p0 = m.vertices[ m.triangles[t][0] ].p;
            point3d const & p1 = m.vertices[ m.triangles[t][1] ].p;
            point3d const & p2 = m.vertices[ m.triangles[t][2] ].p;
            point3d n = point3d::cross( p1-p0 , p2-p0 ).direction();
            double area = point3d::cross( p1-p0 , p2-p0 ).norm();

            //4 points per triangles
            //barycenter of the 4 new created triangles inside the original
            point3d p01 = (p0 + p1)/2;
            point3d p12 = (p1 + p2)/2;
            point3d p02 = (p0 + p2)/2;
            point3d p3 = (p0+p01+p02)/3;
            point3d p4 = (p1+p01+p12)/3;
            point3d p5 = (p2+p02+p12)/3;
            point3d p6 = (p0+p1+p2)/3;

            //filling pointSet
            pointCloud.push_back(p3);
            pointCloud.push_back(p4);
            pointCloud.push_back(p5);
            pointCloud.push_back(p6);

            //filling TripletList
            triplet.n = n;
            triplet.area = area/4;
            triplet.p = p3;
            TripletList.push_back(triplet);
            triplet.p = p4;
            TripletList.push_back(triplet);
            triplet.p = p5;
            TripletList.push_back(triplet);
            triplet.p = p6;
            TripletList.push_back(triplet);
        }
        return pointCloud;
    }

    //Initialization
    void createPointSet(Mesh const & mesh, std::vector<Triplet> & pointSet){
        for (unsigned int i = 0 ; i<mesh.triangles.size() ; i++){
            point3d p0 = mesh.vertices[mesh.triangles[i][0]].p;
            point3d p1 = mesh.vertices[mesh.triangles[i][1]].p;
            point3d p2 = mesh.vertices[mesh.triangles[i][2]].p;
            Triplet t;
            t.p = p0/3+p1/3+p2/3;
            t.n = point3d::cross(p1-p0, p2-p0);
            t.area = t.n.norm()/2.0;
            pointSet.push_back(t);
        }
    }

    void mainFunction(){

        //computes points cloud
        //createPointSet(mesh, pointSet);
        /*std::vector< point3d > cloudPositions;
        for (int i = 0 ; i<pointSet.size() ; i++){
            cloudPositions.push_back(pointSet[i].p);
        }*/
        std::vector< point3d > const cloudPositions = fromMeshToPointSet(mesh, pointSet);
        tetmesh.tetMesh;
        tetmesh.TetGenHandler::computeTetMeshFromCloud ( cloudPositions );
        std::cout << "PointSet created : " << pointSet.size() << " points" << std::endl;

        //tetraedralization
        tetmesh.computeTetMeshFromCloud ( cloudPositions );
        std::cout << "Done: Tetraedralization" << std::endl;

        // initialize indices vector with : 0,1,2...
        std::vector<int> iota(pointSet.size()) ;
        std::iota (std::begin(iota), std::end(iota), 0);

        tree.node = tree.buildKDTree(iota, pointSet);
        std::cout << "Done: KDTree" << std::endl;

        //computes WN of tetra
        std::vector<int> iota2(pointSet.size()) ;
        std::iota (std::begin(iota2), std::end(iota2), 0);
        double wn = 0;
        for( unsigned int t = 0 ; t < tetmesh.nTetrahedra() ; ++t ) {
            point4ui tet = tetmesh.tetrahedron(t);
            point3d const & p0 = tetmesh.vertex(tet.x());
            point3d const & p1 = tetmesh.vertex(tet.y());
            point3d const & p2 = tetmesh.vertex(tet.z());
            point3d const & p3 = tetmesh.vertex(tet.w());
            wn = tree.fastWN( (p0+p1+p2+p3)/4, tree.node, pointSet);
            windingNumbers.push_back(wn);
        }
        std::cout << "Done: WindingNumbers of Tet" << std::endl;
    }

    //Draw
    void draw() {
        if (showKDTree) {
            std::cout << "show kdtree" << std::endl;
            drawKDTree(tree.node);
        }
        switch(mode){
        case 0:
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
            break;
        case 1:
            glEnable( GL_LIGHTING );
            glColor3f(0.5,0.5,0.8);
            for( unsigned int t = 0 ; t < tetmesh.nTetrahedra() ; ++t ) {
                point4ui const & tet = tetmesh.tetrahedron(t);
                drawTetra(tet);
            }
            break;
        case 2:
            drawWindingNumberTetra();
            break;
        }
    }

    void drawBox(point3d min, point3d max){
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // this tells it to only render lines
        glBegin(GL_LINES);

        glColor3f(1.0, 0.0, 0.0);
        //face below
        glVertex3f(min[0],min[1],min[2]);
        glVertex3f(max[0],min[1],min[2]);

        glVertex3f(min[0],min[1],min[2]);
        glVertex3f(min[0],max[1],min[2]);

        glVertex3f(min[0],max[1],min[2]);
        glVertex3f(max[0],max[1],min[2]);

        glVertex3f(max[0],min[1],min[2]);
        glVertex3f(max[0],max[1],min[2]);

        //face above
        glVertex3f(max[0],min[1],max[2]);
        glVertex3f(min[0],min[1],max[2]);

        glVertex3f(max[0],max[1],max[2]);
        glVertex3f(min[0],max[1],max[2]);

        glVertex3f(max[0],max[1],max[2]);
        glVertex3f(max[0],min[1],max[2]);

        glVertex3f(min[0],max[1],max[2]);
        glVertex3f(min[0],min[1],max[2]);

        //side lines
        glVertex3f(max[0],min[1],max[2]);
        glVertex3f(max[0],min[1],min[2]);

        glVertex3f(min[0],min[1],min[2]);
        glVertex3f(min[0],min[1],max[2]);

        glVertex3f(max[0],max[1],max[2]);
        glVertex3f(max[0],max[1],min[2]);

        glVertex3f(min[0],max[1],max[2]);
        glVertex3f(min[0],max[1],min[2]);

        glEnd();
        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    }

    void drawKDTree(KDNode node){
        if ((node.leftChild != nullptr) && (node.leftChild != nullptr)) {
            drawBox(point3d(node.bbox.xMin,node.bbox.yMin,node.bbox.zMin),point3d(node.bbox.xMax,node.bbox.yMax,node.bbox.zMax));
            drawKDTree(*node.leftChild);
            drawKDTree(*node.rightChild);
        } else {
            //drawBox(point3d(node.bbox.xMin,node.bbox.yMin,node.bbox.zMin),point3d(node.bbox.xMax,node.bbox.yMax,node.bbox.zMax));
        }
    }

    void drawWindingNumberTetra() {
        unsigned int nNeg = 0;
        for( unsigned int t = 0 ; t < tetmesh.nTetrahedra() ; ++t ) {
            point4ui tet = tetmesh.tetrahedron(t);
            point3d const & p0 = tetmesh.vertex(tet.x());
            point3d const & p1 = tetmesh.vertex(tet.y());
            point3d const & p2 = tetmesh.vertex(tet.z());
            point3d const & p3 = tetmesh.vertex(tet.w());
            double wn = windingNumbers[t];
            if (wn > 0.5) {
                drawTetra(tet);
            }
            else ++nNeg;
        }
        //std::cout << nNeg << "  /  " << tetmesh.nTetrahedra() << std::endl;
    }

    void drawTetra(point4ui const & tet) {

        point3d const & p0 = tetmesh.vertex(tet.x());
        point3d const & p1 = tetmesh.vertex(tet.y());
        point3d const & p2 = tetmesh.vertex(tet.z());
        point3d const & p3 = tetmesh.vertex(tet.w());
        point3d const & center = (p0+p1+p2+p3)/4;

        point3d p0b = p0 + lambda*(center - p0);
        point3d p1b = p1 + lambda*(center - p1);
        point3d p2b = p2 + lambda*(center - p2);
        point3d p3b = p3 + lambda*(center - p3);

        point3d const & n0 = -point3d::cross( p1b-p0b , p2b-p0b ).direction();
        point3d const & n1 = -point3d::cross( p3b-p0b , p1b-p0b ).direction();
        point3d const & n2 = -point3d::cross( p3b-p1b , p2b-p1b ).direction();
        point3d const & n3 = -point3d::cross( p2b-p0b , p3b-p0b ).direction();

        glBegin(GL_TRIANGLES);

        glNormal3f(n0[0],n0[1],n0[2]);
        glVertex3f(p0b[0],p0b[1],p0b[2]);
        glVertex3f(p1b[0],p1b[1],p1b[2]);
        glVertex3f(p2b[0],p2b[1],p2b[2]);

        glNormal3f(n1[0],n1[1],n1[2]);
        glVertex3f(p0b[0],p0b[1],p0b[2]);
        glVertex3f(p3b[0],p3b[1],p3b[2]);
        glVertex3f(p1b[0],p1b[1],p1b[2]);

        glNormal3f(n2[0],n2[1],n2[2]);
        glVertex3f(p1b[0],p1b[1],p1b[2]);
        glVertex3f(p3b[0],p3b[1],p3b[2]);
        glVertex3f(p2b[0],p2b[1],p2b[2]);

        glNormal3f(n3[0],n3[1],n3[2]);
        glVertex3f(p0b[0],p0b[1],p0b[2]);
        glVertex3f(p2b[0],p2b[1],p2b[2]);
        glVertex3f(p3b[0],p3b[1],p3b[2]);

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
        text += "<li>I   :   initialize tetraedralizaton, kdtree & winding number</li>";
        text += "<li>M   :   show mesh</li>";
        text += "<li>T   :   show tetraedrons</li>";
        text += "<li>W   :   show winding number choice</li>";
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
        else if( event->key() == Qt::Key_I ) {
            // initialize
            mainFunction();
        }
        else if ( event->key() == Qt::Key_M ){
            // show initial mesh
            mode = 0;
        }
        else if( event->key() == Qt::Key_T ) {
            // show tetraedralization
            mode = 1;
        }
        else if ( event->key() == Qt::Key_W ){
            // show selected tetra with winding number
            mode = 2;
        }
        else if ( event->key() == Qt::Key_K ){
            // show kdtree
            showKDTree = showKDTree ? false : true;
        }
        //kdtree depth displayed
        else if ( event->key() == Qt::Key_Right){
            kdTreeDisplayDepth++;
        }
        else if ( event->key() == Qt::Key_Left){
            kdTreeDisplayDepth--;
            if (kdTreeDisplayDepth < 0) kdTreeDisplayDepth = 0;
        }
        //scaling of displayed tetra
        else if ( event->key() == Qt::Key_Up){
            lambda += 0.05;
            if (lambda > 1) lambda = 1.0;
        }
        else if ( event->key() == Qt::Key_Down){
            lambda -= 0.05;
            if (lambda < 0) lambda = 0.0;
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
