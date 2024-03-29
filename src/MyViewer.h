#ifndef MYVIEWER_H
#define MYVIEWER_H

// Mesh stuff:
#include "Mesh.h"
#include "tetgenhandler.h"

//KDTree
#include "KDTree.h"

//Graphcut
#include <graph.h>
#include <set>

// Parsing:
#include "BasicIO.h"
#include <iostream>
#include <fstream>
#include <string>

// analysis
#include <time.h>

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

#include <set>

static bool showKDTree = false;
static int mode = 0;

struct TriInt{
    int i0=0, i1=0, i2=0; // i0!=i1!=i2

    bool equals(TriInt const & t0, TriInt const & t1) const {
        if ((t0.i0 == t1.i0 || t0.i0 == t1.i1 || t0.i0 == t1.i2) &&
                (t0.i1 == t1.i0 || t0.i1 == t1.i1 || t0.i1 == t1.i2) &&
                (t0.i2 == t1.i0 || t0.i2 == t1.i1 || t0.i2 == t1.i2)){
            return true;
        }
        else return false;
    }

    bool inferior(TriInt const & t0, TriInt const& t1) const {
        TriInt t0_ordered;
        t0_ordered.i0 = t0.i0; t0_ordered.i1 = t0.i1; t0_ordered.i2 = t0.i2;
        if (t0_ordered.i0 > t0_ordered.i1){
            int temp = t0_ordered.i0;
            t0_ordered.i0 = t0_ordered.i1;
            t0_ordered.i1 = temp;
        }
        if (t0_ordered.i0 > t0_ordered.i2){
            int temp = t0_ordered.i0;
            t0_ordered.i0 = t0_ordered.i2;
            t0_ordered.i2 = temp;
        }
        if (t0_ordered.i2 < t0_ordered.i1){
            int temp = t0_ordered.i2;
            t0_ordered.i2 = t0_ordered.i1;
            t0_ordered.i1 = temp;
        }

        TriInt t1_ordered;
        t1_ordered.i0 = t1.i0; t1_ordered.i1 = t1.i1; t1_ordered.i2 = t1.i2;
        if (t1_ordered.i0 > t1_ordered.i1){
            int temp = t1_ordered.i0;
            t1_ordered.i0 = t1_ordered.i1;
            t1_ordered.i1 = temp;
        }
        if (t1_ordered.i0 > t1_ordered.i2){
            int temp = t1_ordered.i0;
            t1_ordered.i0 = t1_ordered.i2;
            t1_ordered.i2 = temp;
        }
        if (t1_ordered.i2 < t1_ordered.i1){
            int temp = t1_ordered.i2;
            t1_ordered.i2 = t1_ordered.i1;
            t1_ordered.i1 = temp;
        }

        //compare first number
        if (t0_ordered.i0 < t1_ordered.i0) return true;
        else if (t0_ordered.i0 > t1_ordered.i0) return false;
        else if (t0_ordered.i0 == t1_ordered.i0){
            //compare second number
            if(t0_ordered.i1 < t1_ordered.i1) return true;
            else if(t0_ordered.i1 > t1_ordered.i1) return false;
            else if(t0_ordered.i1 == t1_ordered.i1){
                //compare third number
                if (t0_ordered.i2 < t1_ordered.i2) return true;
                else if (t0_ordered.i2 >= t1_ordered.i2) return false;
            }
        }
        //by default
        return false;
    }

    bool operator< (TriInt const & otherTri) const {
        if (inferior(*this, otherTri)) return true;
        else return false;
    }
};

struct Tet{
    int index;
    int i0=0, i1=0, i2=0, i3=0;
    std::set<int> tetNeighbors;
    std::map<int, double> commonArea;
};

class MyViewer : public QGLViewer , public QOpenGLFunctions_3_0
{
    Q_OBJECT

    KDTree tree;
    Mesh mesh;
    TetGenHandler tetmesh;
    std::vector<Triplet> pointSet;
    std::vector< point3d > drawablePointCloud;
    std::vector< point3d > cloudPositions;
    std::vector<double> windingNumbers;
    std::vector<double> wnGraphcut;
    std::vector<Tet> tetList;
    double cutDepth = 0;
    double lambda = 0.0;
    std::vector<Tet> adjTets;
    double bBoxAxis = 0;

    std::vector<point3d> pointSetForTetrahedrisation;

    //used for the statistic analysis
    std::vector<int> visited_nodes;
    std::vector<double> distanceToSurface;

    point3d bb, BB;

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
    void test_graph_cut() {
        {
            typedef GraphCut_BK::Graph<int,int,int> MonTypeDeGraphePourGraphCut;
            MonTypeDeGraphePourGraphCut *g = new MonTypeDeGraphePourGraphCut(/*estimated # of nodes*/ 2, /*estimated # of edges*/ 1);

            g -> add_node();
            g -> add_node();

            g -> add_tweights( 0,   /* capacities */  5, 1 );
            g -> add_tweights( 1,   /* capacities */  2, 6 );
            g -> add_edge( 0, 1,    /* capacities */  3, 4 );

            int flow = g -> maxflow();

            printf("Flow = %d\n", flow);
            printf("Minimum cut:\n");
            if (g->what_segment(0) == MonTypeDeGraphePourGraphCut::SOURCE)
                printf("node0 is in the SOURCE set\n");
            else
                printf("node0 is in the SINK set\n");
            if (g->what_segment(1) == MonTypeDeGraphePourGraphCut::SOURCE)
                printf("node1 is in the SOURCE set\n");
            else
                printf("node1 is in the SINK set\n");

            delete g;
        }
    }

    void test_map_struct(){
        std::map<TriInt, std::set<int> > toBeFilled;
        TriInt f0, f1, f2, f3;
        f0.i0 = 0; f0.i1 = 1; f0.i2 = 2;
        f1.i0 = 0; f1.i1 = 1; f1.i2 = 2;
        f2.i0 = 0; f2.i1 = 2; f2.i2 = 1;
        f3.i0 = 0; f3.i1 = 2; f3.i2 = 3;
        toBeFilled[f0].insert(0);
        std::cout << toBeFilled.size() << std::endl;
        toBeFilled[f1].insert(1);
        std::cout << toBeFilled.size() << std::endl;
        toBeFilled[f2].insert(0);
        std::cout << toBeFilled.size() << std::endl;
        toBeFilled[f3].insert(2);
        std::cout << toBeFilled.size() << std::endl;

        std::cout << std::endl;
        std::cout << toBeFilled[f0].size() << std::endl;

        std::set<int>::iterator it;
        for (it = toBeFilled[f0].begin() ; it != toBeFilled[f0].end() ; ++it){
            std::cout << *it << std::endl;
        }
    }

    std::map<TriInt, std::set<int> > findAdjTets(TetGenHandler const & tmesh){
        std::map<TriInt, std::set<int> > tetsSurTriangle;
        for (unsigned int t = 0 ; t<tmesh.nTetrahedra() ; t++){
            point4ui tet = tmesh.tetrahedron(t);
            TriInt f0;
            f0.i0 = tet.x(); f0.i1 = tet.y(); f0.i2 = tet.z();
            tetsSurTriangle[f0].insert(t);
            TriInt f1;
            f1.i0 = tet.y(); f1.i1 = tet.z(); f1.i2 = tet.w();
            tetsSurTriangle[f1].insert(t);
            TriInt f2;
            f2.i0 = tet.z(); f2.i1 = tet.w(); f2.i2 = tet.x();
            tetsSurTriangle[f2].insert(t);
            TriInt f3;
            f3.i0 = tet.w(); f3.i1 = tet.x(); f3.i2 = tet.y();
            tetsSurTriangle[f3].insert(t);
        }
        return tetsSurTriangle;
    }

    void fillTetStruct(){
        adjTets.resize(tetmesh.nTetrahedra());

        //init adjTets list with each tetrahedron
        for (unsigned int t = 0 ; t<tetmesh.nTetrahedra() ; t++){
            point4ui tet = tetmesh.tetrahedron(t);
            Tet tetra;
            tetra.index = t;
            tetra.i0 = tet.x(); tetra.i1 = tet.y();
            tetra.i2 = tet.z(); tetra.i3 = tet.w();
            adjTets[t] = tetra;
        }
        //return for a given triangle triplet the index of its tetrahedra in tetmesh
        std::map<TriInt, std::set<int> > tetsSurTriangle = findAdjTets(tetmesh);
        //fill in the adjTets structure
        for (int t = 0 ; t< (int)tetmesh.nTetrahedra() ; t++){
            //for each tetra create 4  triangles
            Tet tetra = adjTets[t];
            TriInt f0;
            f0.i0 = tetra.i0; f0.i1 = tetra.i1; f0.i2 = tetra.i2;
            TriInt f1;
            f1.i0 = tetra.i1; f1.i1 = tetra.i2; f1.i2 = tetra.i3;
            TriInt f2;
            f2.i0 = tetra.i2; f2.i1 = tetra.i3; f2.i2 = tetra.i0;
            TriInt f3;
            f3.i0 = tetra.i3; f3.i1 = tetra.i0; f3.i2 = tetra.i1;
            //get the set of corresponding tetra indices
            std::set<int> trgs0 = tetsSurTriangle[f0];
            std::set<int> trgs1 = tetsSurTriangle[f1];
            std::set<int> trgs2 = tetsSurTriangle[f2];
            std::set<int> trgs3 = tetsSurTriangle[f3];

            std::set<int>::iterator it;
            for (it = trgs0.begin() ; it != trgs0.end() ; ++it){
                if(*it != t) {
                    double area = point3d::cross(tetmesh.vertex(tetra.i0) - tetmesh.vertex(tetra.i1),
                                        tetmesh.vertex(tetra.i0) - tetmesh.vertex(tetra.i2)).norm()/2;
                    adjTets[t].commonArea[*it] = area;
                    adjTets[t].tetNeighbors.insert(*it);
                }
            }
            for (it = trgs1.begin() ; it != trgs1.end() ; ++it){
                if(*it != t) {
                    double area = point3d::cross(tetmesh.vertex(tetra.i1) - tetmesh.vertex(tetra.i2),
                                        tetmesh.vertex(tetra.i1) - tetmesh.vertex(tetra.i3)).norm()/2;
                    adjTets[t].commonArea[*it] = area;
                    adjTets[t].tetNeighbors.insert(*it);
                }
            }
            for (it = trgs2.begin() ; it != trgs2.end() ; ++it){
                if(*it != t) {
                    double area = point3d::cross(tetmesh.vertex(tetra.i0) - tetmesh.vertex(tetra.i2),
                                        tetmesh.vertex(tetra.i0) - tetmesh.vertex(tetra.i3)).norm()/2;
                    adjTets[t].commonArea[*it] = area;
                    adjTets[t].tetNeighbors.insert(*it);
                }
            }
            for (it = trgs3.begin() ; it != trgs3.end() ; ++it){
                if(*it != t) {
                    double area = point3d::cross(tetmesh.vertex(tetra.i0) - tetmesh.vertex(tetra.i1),
                                        tetmesh.vertex(tetra.i0) - tetmesh.vertex(tetra.i3)).norm()/2;
                    adjTets[t].commonArea[*it] = area;
                    adjTets[t].tetNeighbors.insert(*it);
                }
            }
        }
    }

    void mainFunction(){
        //mesh.subdivide();
        //computes points cloud
        cloudPositions = fromMeshToPointSet(mesh, pointSet);
        //tetmesh.tetMesh;
        pointSetForTetrahedrisation = cloudPositions;
        int rez = 10;
        double xStep = (BB[0]-bb[0])/rez;
        double yStep = (BB[1]-bb[1])/rez;
        double zStep = (BB[2]-bb[2])/rez;
        for (int a = 0 ; a<rez ; a++){
            for (int b = 0 ; b<rez ; b++){
                for (int c = 0 ; c<rez ; c++){
                    point3d pt(bb[0]+a*xStep, bb[1]+b*yStep, bb[2]+c*zStep);
                    pointSetForTetrahedrisation.push_back(pt);
                }
            }
        }
        tetmesh.TetGenHandler::computeTetMeshFromCloud ( pointSetForTetrahedrisation );
        std::cout << "PointSet created : " << pointSet.size() << " points" << std::endl;

        //tetraedralization
        tetmesh.computeTetMeshFromCloud ( pointSetForTetrahedrisation );
        std::cout << "Done: Tetraedralization" << std::endl;

        // initialize indices vector with : 0,1,2...
        std::vector<int> iota(pointSet.size()) ;
        std::iota (std::begin(iota), std::end(iota), 0);

        tree.node = tree.buildKDTree(iota, pointSet);
        std::cout << "Done: KDTree" << std::endl;

        clock_t tStart = clock();

        //computes WN of tetra
        std::vector<int> iota2(pointSet.size()) ;
        std::iota (std::begin(iota2), std::end(iota2), 0);
        double wn = 0;

        windingNumbers.resize(tetmesh.nTetrahedra());
        visited_nodes.resize(tetmesh.nTetrahedra());
        distanceToSurface.resize(tetmesh.nTetrahedra());

        int nb_nodes = 0;
        double dist = 0;
        point3d p;

        double xSize = tree.node.bbox.xMax - tree.node.bbox.xMin;
        double ySize = tree.node.bbox.yMax - tree.node.bbox.yMin;
        double zSize = tree.node.bbox.zMax - tree.node.bbox.zMin;
        bBoxAxis = sqrt(xSize*xSize + ySize*ySize + zSize*zSize);

        std::vector<point3d> query_pts;
        query_pts.resize(4);
        std::vector<double> wn_vec;
        wn_vec.resize(4);

        for( unsigned int t = 0 ; t < tetmesh.nTetrahedra() ; ++t ) {
            if (t%1000==0) std::cout << t << std::endl;
            point4ui tet = tetmesh.tetrahedron(t);
            point3d const & p0 = tetmesh.vertex(tet.x());
            point3d const & p1 = tetmesh.vertex(tet.y());
            point3d const & p2 = tetmesh.vertex(tet.z());
            point3d const & p3 = tetmesh.vertex(tet.w());
            point3d const & p4 = (p0/2) + (p1+p2+p3)/6;
            point3d const & p5 = (p1/2) + (p0+p2+p3)/6;
            point3d const & p6 = (p2/2) + (p0+p1+p3)/6;
            point3d const & p7 = (p3/2) + (p0+p1+p2)/6;
            query_pts[0] = p4;
            query_pts[1] = p5;
            query_pts[2] = p6;
            query_pts[3] = p7;
            //4 evaluations inside de tetrahedron
            for( int i = 0 ; i < 4 ; ++i ) {
                nb_nodes = 0;
                dist = std::numeric_limits<double>::infinity();
                wn_vec[i] = tree.fastWN( query_pts[i], tree.node, pointSet, nb_nodes);
                drawablePointCloud.push_back(query_pts[i]);
                visited_nodes[t] = nb_nodes;
                //tree.NNS(query_pts[i],tree.node,pointSet,p,dist);
                //distanceToSurface[t] = dist/bBoxAxis;
            }
            wn = 0;
            for( int i = 0 ; i < 4 ; ++i ) {
                wn += wn_vec[i];
            }
            wn /= 4;
            windingNumbers[t] = wn;
        }

        std::cout << "time: " << (double)(clock() - tStart)/CLOCKS_PER_SEC << std::endl;
        std::cout << "Done: WindingNumbers of Tet" << std::endl;

        fillTetStruct();
        std::cout << "Done: Tet structure filled" << std::endl;

        graph_cut();
        std::cout << "Done: Graph cut" << std::endl;

        int total_depth = tree.node.compute_depth();
        std::cout << "Depth of the KDTree : " << total_depth << std::endl;

        std::cout << "Diagonal of Bbox: " << bBoxAxis << std::endl;

        // fill a txt file for statistics purposes
        //fillTxtFile();
    }

    void fillTxtFile() {
      std::ofstream myfile ("beta.txt",std::ios::app);
      if (myfile.is_open()) {
          std::cout << "File Opened" << std::endl;
          //double max_dist = *std::max_element(distanceToSurface.begin(),distanceToSurface.end());
          //double min_dist = *std::min_element(distanceToSurface.begin(),distanceToSurface.end());
          for(unsigned int i = 0; i<visited_nodes.size(); i++) {

            //double dist = distanceToSurface[i];
            //dist = (dist - min_dist)/(max_dist-min_dist);
            myfile << std::to_string(10) + " ";
            myfile << std::to_string(visited_nodes[i]) + "\n";
            // R G B
            //myfile << std::to_string( (int)( 255*(1-dist) + 100*(dist)) ) + " ";
            //myfile << std::to_string( 127 ) + " ";
            //myfile << std::to_string( 127 ) + "\n" ;
            //myfile << std::to_string((int) (dist*10000)) + " ";
            //myfile << std::to_string(visited_nodes[i]) + "\n";
          }
          myfile.close();
          std::cout << "File Closed" << std::endl;
      }
      else std::cout << "Unable to open file";
    }

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
        case 3:
            drawGraphCutDisplay();
            break;
        case 4:
            drawPointSetQuery();
            break;
        case 5:
            drawOriginalPointSet();
            break;
        }
    }

    void computeCutDepth(){
        double closestPtDepth = 1, farestPtDepth = 0;
        for (unsigned int i = 0 ; i<mesh.vertices.size() ; i++){
            point3d pt = mesh.vertices[i].p;
            qglviewer::Vec ptproj = camera()->projectedCoordinatesOf(qglviewer::Vec(pt[0],pt[1],pt[2]));
            if (closestPtDepth > ptproj[2]){
                closestPtDepth = ptproj[2];
            }
            else if (farestPtDepth < ptproj[2]){
                farestPtDepth = ptproj[2];
            }
        }
        cutDepth = (closestPtDepth+farestPtDepth)/2;
    }

    bool isInsideTetra(point3d const & pt, point4ui const & tet){
        point3d const & p0 = tetmesh.vertex(tet.x());
        point3d const & p1 = tetmesh.vertex(tet.y());
        point3d const & p2 = tetmesh.vertex(tet.z());
        point3d const & p3 = tetmesh.vertex(tet.w());

        point3d normal0 = point3d::cross(p1-p0,p2-p0);
        point3d normal1 = point3d::cross(p2-p0,p3-p0);
        point3d normal2 = point3d::cross(p3-p0,p1-p0);
        point3d normal3 = point3d::cross(p3-p1,p2-p1);
        point3d vector0 = pt - p0;
        point3d vector1 = pt - p1;

        if (point3d::dot(normal0,vector0) >= 0 && point3d::dot(normal1,vector0) >= 0 && point3d::dot(normal2,vector0) >= 0 && point3d::dot(normal3,vector1) >= 0){
            return true;
        }

        return false;
    }

    void drawPointSetQuery() {
        //glColor3f(1,0.5,0.5);
        glBegin(GL_POINTS);
        for (unsigned int i = 0; i< drawablePointCloud.size(); i++) {
            glVertex3d(drawablePointCloud[i].x(),drawablePointCloud[i].y(),drawablePointCloud[i].z());
        }
        glEnd();
    }

    void drawOriginalPointSet() {
        //glColor3f(1,0.5,0.5);
        glBegin(GL_POINTS);
        for (unsigned int i = 0; i< cloudPositions.size(); i++) {
            glVertex3d(cloudPositions[i].x(),cloudPositions[i].y(),cloudPositions[i].z());
        }
        glEnd();
    }

    void drawCutDisplay(){
        point3d axis(camera()->viewDirection());
        point3d center(sceneCenter());
        double radius = sceneRadius();
        double centerAlongAxis = point3d::dot(axis ,center);
        double alongAxisCut = centerAlongAxis - radius + cutDepth * 2 * radius;
        for( unsigned int t = 0 ; t < tetmesh.nTetrahedra() ; ++t ) {
            double wn = windingNumbers[t];
            if (wn > 0.5){
                point4ui tet = tetmesh.tetrahedron(t);
                point3d const & p0 = tetmesh.vertex(tet.x());
                point3d const & p1 = tetmesh.vertex(tet.y());
                point3d const & p2 = tetmesh.vertex(tet.z());
                point3d const & p3 = tetmesh.vertex(tet.w());
                point3d tetcenter = (p0 + p1 + p2 + p3) / 4;

                //if plane and tetrahedron intersect
                if (alongAxisCut <= point3d::dot(axis ,tetcenter)) {
                    for (int x = 0 ; x<camera()->screenWidth() ; x++){
                        for (int y = 0 ; y<camera()->screenHeight() ; y++){
                            point3d pt = camera()->unprojectedCoordinatesOf(qglviewer::Vec(x, y, cutDepth));
                            if (isInsideTetra(pt, tet)){
                                glBegin(GL_POINTS);
                                glColor3f(255,0,0);
                                glVertex3f(pt[0],pt[1],pt[2]);
                                glEnd();
                            }
                        }
                    }
                }
            }
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
        point3d axis(camera()->viewDirection());
        point3d center(sceneCenter());
        double radius = sceneRadius();
        double centerAlongAxis = point3d::dot(axis ,center);
        double alongAxisCut = centerAlongAxis - radius + cutDepth * 2 * radius;
        for( unsigned int t = 0 ; t < tetmesh.nTetrahedra() ; ++t ) {
            point4ui const tet = tetmesh.tetrahedron(t);
            point3d const & p0 = tetmesh.vertex(tet.x());
            point3d const & p1 = tetmesh.vertex(tet.y());
            point3d const & p2 = tetmesh.vertex(tet.z());
            point3d const & p3 = tetmesh.vertex(tet.w());
            double wn = windingNumbers[t];
            if (wn > 0.5) {
                point3d tetcenter = (p0 + p1 + p2 + p3) / 4;
                //if plane and tetrahedron intersect
                if (alongAxisCut <= point3d::dot(axis ,tetcenter))
                    drawTetra(tet);
            }
            else ++nNeg;
        }
        //std::cout << nNeg << "  /  " << tetmesh.nTetrahedra() << std::endl;
    }

    void drawGraphCutDisplay() {
        unsigned int nNeg = 0;
        point3d axis(camera()->viewDirection());
        point3d center(sceneCenter());
        double radius = sceneRadius();
        double centerAlongAxis = point3d::dot(axis ,center);
        double alongAxisCut = centerAlongAxis - radius + cutDepth * 2 * radius;
        for( unsigned int t = 0 ; t < tetmesh.nTetrahedra() ; ++t ) {
            point4ui tet = tetmesh.tetrahedron(t);
            point3d const & p0 = tetmesh.vertex(tet.x());
            point3d const & p1 = tetmesh.vertex(tet.y());
            point3d const & p2 = tetmesh.vertex(tet.z());
            point3d const & p3 = tetmesh.vertex(tet.w());
            double wn = wnGraphcut[t];
            if (wn > 0.5) {
                point3d tetcenter = (p0 + p1 + p2 + p3) / 4;
                //if plane and tetrahedron intersect
                if (alongAxisCut <= point3d::dot(axis ,tetcenter))
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

    void glSphere(float x, float y, float z, float radius){
        int N = 40;
        glBegin(GL_TRIANGLES);
        //glColor3f(255, 0, 0);
        for (int j = -N/2 ; j<N/2 ; j++){
            for (int i = 0 ; i<N ; i++){
                float x1 = radius*cos(2*M_PI*i/N)*cos(2*M_PI*j/N);
                float z1 = radius*sin(2*M_PI*i/N)*cos(2*M_PI*j/N);
                float y1 = radius*sin(2*M_PI*j/N);
                float x2 = radius*cos(2*M_PI*(i+1)/N)*cos(2*M_PI*j/N);
                float z2 = radius*sin(2*M_PI*(i+1)/N)*cos(2*M_PI*j/N);
                float y2 = radius*sin(2*M_PI*j/N);
                float x3 = radius*cos(2*M_PI*i/N)*cos(2*M_PI*(j+1)/N);
                float z3 = radius*sin(2*M_PI*i/N)*cos(2*M_PI*(j+1)/N);
                float y3 = radius*sin(2*M_PI*(j+1)/N);
                float x4 = radius*cos(2*M_PI*(i+1)/N)*cos(2*M_PI*(j+1)/N);
                float z4 = radius*sin(2*M_PI*(i+1)/N)*cos(2*M_PI*(j+1)/N);
                float y4 = radius*sin(2*M_PI*(j+1)/N);
                //glColor3f(x1, y1, z1);
                glVertex3f(x1+x, y1+y, z1+z);
                //glColor3f(x2, y2, z2);
                glVertex3f(x2+x, y2+y, z2+z);
                //glColor3f(x4, y4, z4);
                glVertex3f(x4+x, y4+y, z4+z);

                //glColor3f(x1, y1, z1);
                glVertex3f(x1+x, y1+y, z1+z);
                //glColor3f(x4, y4, z4);
                glVertex3f(x4+x, y4+y, z4+z);
                //glColor3f(x3, y3, z3);
                glVertex3f(x3+x, y3+y, z3+z);
            }
        }
        glEnd();
    }

    void graph_cut(double sigma = 1 , double gamma = 1000){

        gamma /= bBoxAxis*bBoxAxis;

        typedef GraphCut_BK::Graph<double,double,double> MonTypeDeGraphePourGraphCut;
        MonTypeDeGraphePourGraphCut *g = new MonTypeDeGraphePourGraphCut(/*estimated # of nodes*/ tetmesh.nTetrahedra(), /*estimated # of edges*/ tetmesh.nTetrahedra());

        std::vector<std::map<int,bool>> hasEdge(tetmesh.nTetrahedra());
        double wn = 0;

        //each tetrahedron has a node
        for( unsigned int i = 0 ; i < tetmesh.nTetrahedra() ; i++ ) {
            g -> add_node();
            wn = windingNumbers[i];
            g -> add_tweights( i,   /* capacities */  std::max<double>(1.0 - wn,0.0),  std::max<double>(wn,0.0) );
        }
        double epsilonMin = 0.0000000000001;
        for( unsigned int i = 0 ; i < adjTets.size() ; i++ ) {
            wn = windingNumbers[adjTets[i].index];
            for( auto j : adjTets[i].tetNeighbors) {
                if (!hasEdge[i][j] && !hasEdge[j][i]) {
                    double tmp = windingNumbers[j];
                    double weight = gamma * exp(-(tmp-wn)*(tmp-wn)) * adjTets[i].commonArea[j]/(2*sigma*sigma);
                    weight = std::max<double>(epsilonMin,weight);
                    g -> add_edge( i, j,    /* capacities */  weight, weight );
                    hasEdge[i][j] = true;
                    hasEdge[j][i] = true;
                }
            }
        }

        g -> maxflow();

        wnGraphcut.resize(adjTets.size());
        int n = 0;
        for( unsigned int i = 0 ; i < adjTets.size() ; i++ ) {
            if (g->what_segment(i) == MonTypeDeGraphePourGraphCut::SOURCE) {
                wnGraphcut[i] = 0;
            } else {
                wnGraphcut[i] = 1 ;
            }
            n++;
        }
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
        setSceneRadius( 0.7f * ( BB - bb ).norm() );
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
            update();
        }
        else if( event->key() == Qt::Key_T ) {
            // show tetraedralization
            mode = 1;
            update();
        }
        else if ( event->key() == Qt::Key_W ){
            // show selected tetra with winding number
            mode = 2;
            update();
        }
        else if ( event->key() == Qt::Key_G ){
            // computes graphcut
            graph_cut();
        }
        else if ( event->key() == Qt::Key_K ){
            // show kdtree
            showKDTree = showKDTree ? false : true;
        }
        else if ( event->key() == Qt::Key_C ){
            // show graph cut minimization
            mode = 3;
            update();
        }
        else if ( event->key() == Qt::Key_Q ) {
            mode = 4;
            update();
        }
        else if ( event->key() == Qt::Key_P ) {
            mode = 5;
            update();
        }
        else if ( event->key() == Qt::Key_Right){
            cutDepth += 0.001;
            if (cutDepth > 1) cutDepth = 1;
            update();
        }
        else if ( event->key() == Qt::Key_Left){
            cutDepth -= 0.001;
            if (cutDepth < 0) cutDepth = 0;
            update();
        }
        //scaling of displayed tetra
        else if ( event->key() == Qt::Key_Up){
            lambda += 0.05;
            if (lambda > 1) lambda = 1.0;
            update();
        }
        else if ( event->key() == Qt::Key_Down){
            lambda -= 0.05;
            if (lambda < 0) lambda = 0.0;
            update();
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

    //https://stackoverflow.com/questions/8520560/get-a-file-name-from-a-path
    std::string basename( std::string const & pathname ) {
        // Remove directory if present.
        // Do this before extension removal incase directory has a period character.
        std::string cpy = pathname;
        const size_t last_slash_idx = cpy.find_last_of("\\/");
        if (std::string::npos != last_slash_idx) {
            cpy.erase(0, last_slash_idx + 1);
        }
        // Remove extension if present.
        const size_t period_idx = cpy.rfind('.');
        if (std::string::npos != period_idx)
        {
            cpy.erase(period_idx);
        }
        return (std::string)(cpy);
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
                bb = point3d(FLT_MAX,FLT_MAX,FLT_MAX) ; BB = point3d(FLT_MIN,FLT_MIN,FLT_MIN);
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
