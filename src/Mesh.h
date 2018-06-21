#ifndef PROJECTMESH_H
#define PROJECTMESH_H

#include <vector>
#include "point3.h"
#include <map>

struct Pair{ //paire d'index de sommets
    int a, b;
    Pair(int i0, int i1){
        a = std::min(i0,i1);
        b = std::max(i0,i1);
    }

    bool inferior(Pair const & p0, Pair const& p1) const{
        if (p0.a < p1.a) return true;
        else if (p0.a > p1.a) return false;
        else return p0.b < p1.b;
    }

    bool operator< (Pair const & otherPair) const {
        if (inferior(*this, otherPair)) return true;
        else return false;
    }
};

struct Triplet{
    point3d p;
    point3d n;
    double area;
};

struct Vertex{
    point3d p;
    Vertex() {}
    Vertex(double x , double y , double z) : p(x,y,z) {}
};

struct Triangle{
    unsigned int corners[3];
    unsigned int & operator [] (unsigned int c) { return corners[c]; }
    unsigned int operator [] (unsigned int c) const { return corners[c]; }
};

struct Mesh{
    std::vector< Vertex > vertices;
    std::vector< Triangle > triangles;

    void subdivide() {
        std::vector< Vertex > newVertices = vertices;
        std::vector< Triangle > newTriangles;
        int curseur = newVertices.size();
        std::map<Pair, int> vertexPairIsVisited;
        for (int t = 0 ; t<this->triangles.size() ; t++){
            int i0 = this->triangles[t][0], i1 = this->triangles[t][1], i2 = this->triangles[t][2];
            Pair p0(i0,i1), p1(i1,i2), p2(i2,i0);
            point3d pt0(0,0,0), pt1(0,0,0), pt2(0,0,0);
            if (vertexPairIsVisited[p0] == 0){
                vertexPairIsVisited[p0] = 1;
                pt0 = (vertices[i0].p + vertices[i1].p)/2;
                newVertices.push_back(Vertex(pt0[0],pt0[1],pt0[2]));
            }
            if (vertexPairIsVisited[p1] == 0){
                vertexPairIsVisited[p1] = 1;
                pt1 = (vertices[i1].p + vertices[i2].p)/2;
                newVertices.push_back(Vertex(pt1[0],pt1[1],pt1[2]));
            }
            if (vertexPairIsVisited[p2] == 0){
                vertexPairIsVisited[p2] = 1;
                pt2 = (vertices[i2].p + vertices[i0].p)/2;
                newVertices.push_back(Vertex(pt2[0],pt2[1],pt2[2]));
            }
            Triangle t0, t1, t2, t3;
            t0[0] = i0; t0[1] = curseur; t0[2] = curseur+2;
            t1[0] = i1; t1[1] = curseur+1; t1[2] = curseur;
            t2[0] = i2; t2[1] = curseur+2; t2[2] = curseur+1;
            t3[0] = curseur; t3[1] = curseur+1; t3[2] = curseur+2;

            newTriangles.push_back(t0);
            newTriangles.push_back(t1);
            newTriangles.push_back(t2);
            newTriangles.push_back(t3);

            curseur += 3;
        }

        vertices = newVertices;
        triangles = newTriangles;
    }

    void computeVertexNormalsAndAreas() {

    }
};

#endif // PROJECTMESH_H
