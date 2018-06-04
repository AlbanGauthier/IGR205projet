#ifndef TETGENHANDLER_H
#define TETGENHANDLER_H

#include "point3.h"
#include "point4.h"
#include "tetgen.h" // Defined tetgenio, tetrahedralize().

struct TetGenHandler{
    tetgenio tetMesh;

    void computeTetMeshFromCloud(std::vector< point3d > const & cloudPositions) {
        tetgenio in;

        in.numberofpoints = cloudPositions.size();
        in.pointlist = new REAL[ 3 * in.numberofpoints ];

        for( unsigned int v = 0 ; v < in.numberofpoints ; ++v ) {
            for( unsigned int c = 0 ; c < 3 ; ++c )
                in.pointlist[3*v+c] = cloudPositions[v][c];
        }

        in.firstnumber = 0; // All indices start from 0.
        in.numberoffacets = 0;

        tetgenbehavior desiredBehavior;
       // desiredBehavior.quality = 1; desiredBehavior.minratio = 2;
        tetrahedralize(&desiredBehavior, &in, &tetMesh);
    }

    // CAREFUL ! I'M NOT SURE THAT THE FOLLOWING FUNCTION WORKS.
    void computeTetMeshFromCloud(std::vector< point3d > const & cloudPositions , std::vector< std::vector< unsigned int > > & faces) {
        tetgenio in;

        in.numberofpoints = cloudPositions.size();
        in.pointlist = new REAL[ 3 * in.numberofpoints ];

        for( unsigned int v = 0 ; v < in.numberofpoints ; ++v ) {
            for( unsigned int c = 0 ; c < 3 ; ++c )
                in.pointlist[3*v+c] = cloudPositions[v][c];
        }

        in.firstnumber = 0; // All indices start from 0.

        in.numberoffacets = faces.size();
        in.facetlist = new tetgenio::facet[in.numberoffacets];
        in.facetmarkerlist = new int[in.numberoffacets];

        tetgenio::facet *f;
        tetgenio::polygon *p;

        for( unsigned int fIt = 0 ; fIt < faces.size() ; ++fIt ) {
            f = &in.facetlist[fIt];
            f->numberofpolygons = 1;
            f->polygonlist = new tetgenio::polygon[f->numberofpolygons];
            f->numberofholes = 0;
            f->holelist = NULL;
            p = &f->polygonlist[0];
            p->numberofvertices = faces[fIt].size();
            p->vertexlist = new int[p->numberofvertices];
            for( unsigned int vIt = 0 ; vIt < faces[fIt].size() ; ++vIt ) {
                p->vertexlist[vIt] = faces[fIt][vIt];
            }
            in.facetmarkerlist[fIt] = 0;
        }

        tetgenbehavior desiredBehavior;
        tetrahedralize(&desiredBehavior, &in, &tetMesh);
    }

    unsigned int nVertices() const {
        return tetMesh.numberofpoints;
    }
    point3d vertex(unsigned int v) const {
        return point3d( tetMesh.pointlist[3*v + 0] , tetMesh.pointlist[3*v + 1] , tetMesh.pointlist[3*v + 2] );
    }
    unsigned int nTetrahedra() const {
        return tetMesh.numberoftetrahedra;
    }
    point4ui tetrahedron( unsigned int tIt) const {
        return point4ui( tetMesh.tetrahedronlist[4*tIt + 0] , tetMesh.tetrahedronlist[4*tIt + 1] , tetMesh.tetrahedronlist[4*tIt + 2] , tetMesh.tetrahedronlist[4*tIt + 3] );
    }

    ~TetGenHandler() {
    }
};





#endif // TETGENHANDLER_H

