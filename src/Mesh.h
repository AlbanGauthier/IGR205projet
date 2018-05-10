#ifndef PROJECTMESH_H
#define PROJECTMESH_H

#include <vector>
#include "point3.h"



struct Triplet{
    point3d p;
    point3d n;
    double area;
};

struct BBox{
    double xMin, xMax, yMin, yMax, zMin, zMax;
};

struct KDNode{
    Triplet data;
    KDNode *leftChild;
    KDNode *rightChild;
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
};



#endif // PROJECTMESH_H
