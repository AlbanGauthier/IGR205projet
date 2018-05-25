#ifndef KDTREE_H
#define KDTREE_H

#include "Mesh.h"
#include "point3.h"
#include <math.h>

struct BBox{
    double xMin, xMax, yMin, yMax, zMin, zMax;
    double radius() const {
        return sqrt( (xMax - xMin)*(xMax - xMin) + (yMax - yMin)*(yMax - yMin) + (zMax - zMin)*(zMax - zMin) ) / 2;
    }
};

struct KDNode{
    BBox bbox;
    std::vector<int> data;
    point3d meanP, meanN;
    double area;
    KDNode *leftChild;
    KDNode *rightChild;
};

struct KDTree {

    KDNode node;
    double beta = 2.3; // accuracy : the article cites 2 for triangles, 2.3 for points

    BBox computeBoundingBox(std::vector<int> indices, std::vector<Triplet> & pointSet){
        BBox B;
        B.xMin = pointSet[indices[0]].p[0];
        B.xMax = pointSet[indices[0]].p[0];
        B.yMin = pointSet[indices[0]].p[1];
        B.yMax = pointSet[indices[0]].p[1];
        B.zMin = pointSet[indices[0]].p[2];
        B.zMax = pointSet[indices[0]].p[2];

        for (unsigned int i = 1 ; i<indices.size() ; i++){
            if (pointSet[indices[i]].p[0] < B.xMin){
                B.xMin = pointSet[indices[i]].p[0];
            }
            else if (pointSet[indices[i]].p[0] > B.xMax){
                B.xMax = pointSet[indices[i]].p[0];
            }

            if (pointSet[indices[i]].p[1] < B.yMin){
                B.yMin = pointSet[indices[i]].p[1];
            }
            else if (pointSet[indices[i]].p[1] > B.yMax){
                B.yMax = pointSet[indices[i]].p[1];
            }

            if (pointSet[indices[i]].p[2] < B.zMin){
                B.zMin = pointSet[indices[i]].p[2];
            }
            else if (pointSet[indices[i]].p[2] > B.zMax){
                B.zMax = pointSet[indices[i]].p[2];
            }
        }
        return B;
    }

    std::vector<int> sortIndAlongAxis(std::vector<int> const & indices, int axis, std::vector<Triplet> & pointSet){
        std::vector<int> sortedInd;
        std::vector<int> tempIndices = indices;
        int n = indices.size();
        for (int i = 0 ; i<n ; i++){
            int minIndex = 0;
            int minCoord = pointSet[tempIndices[0]].p[axis];
            for (int j = 1 ; j<tempIndices.size() ; j++){
                if (pointSet[tempIndices[j]].p[axis] < minCoord){
                    minIndex = j;
                    minCoord = pointSet[tempIndices[j]].p[axis];
                }
            }
            sortedInd.push_back(tempIndices[minIndex]);
            tempIndices.erase(tempIndices.begin() + minIndex);
        }
        return sortedInd;
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

    int findMedianSample(std::vector<int> const & sortedIndices, std::vector<Triplet> & pointSet){
        if (sortedIndices.size() % 2 == 1){
            return sortedIndices[(sortedIndices.size()-1)/2];
        }
        else{
            return (sortedIndices[sortedIndices.size()/2] + sortedIndices[sortedIndices.size()/2 + 1])/2;
        }
    }

    point3d findMeanPoint(std::vector<int> const & sortedIndices, std::vector<Triplet> & pointSet){
        point3d meanP = point3d(0,0,0);
        double areaSum = 0;

        for(unsigned i = 0; i<sortedIndices.size(); i++) {
            unsigned int v = sortedIndices[i];
            double area = pointSet[v].area;
            meanP += area * pointSet[v].p;
            areaSum += area;
        }
        meanP /= areaSum;
        return meanP;
    }

    point3d findMeanNormal(std::vector<int> const & sortedIndices, std::vector<Triplet> & pointSet){
        point3d meanN = point3d(0,0,0);
        double areaSum = 0;

        for(unsigned i = 0; i<sortedIndices.size(); i++) {
            unsigned int v = sortedIndices[i];
            double area = pointSet[v].area;
            meanN += area * pointSet[v].n;
            //areaSum += area;
        }
        //meanN /= areaSum;
        return meanN;
    }

    KDNode buildKDTree(std::vector<int> const & indices, std::vector<Triplet> & pointSet){
        if (indices.size() == 1){
            KDNode n;
            BBox B;
            // bouding box is the point itself (to avoid errors)
            B.xMin = pointSet[indices[0]].p[0];
            B.xMax = pointSet[indices[0]].p[0];
            B.yMin = pointSet[indices[0]].p[1];
            B.yMax = pointSet[indices[0]].p[1];
            B.zMin = pointSet[indices[0]].p[2];
            B.zMax = pointSet[indices[0]].p[2];

            n.bbox = B;
            n.data = indices;
            n.meanP = pointSet[indices[0]].p;
            n.meanN = pointSet[indices[0]].n;
            n.area = pointSet[indices[0]].area;
            n.leftChild = nullptr;
            n.rightChild = nullptr;
            return n;
        }

        BBox B = computeBoundingBox(indices, pointSet);
        int maxAxis = findMaxAxis(B);
        std::vector<int> const & sortedInd = sortIndAlongAxis(indices, maxAxis, pointSet);
        point3d mp = findMeanPoint(sortedInd, pointSet);
        point3d mn = findMeanNormal(sortedInd, pointSet);

        std::vector<int> upperPartition;
        std::vector<int> lowerPartition;
        for (unsigned int i = 0 ; i<sortedInd.size() ; i++){
            if (i < sortedInd.size()/2){
                lowerPartition.push_back(sortedInd[i]);
            }
            else{
                upperPartition.push_back(sortedInd[i]);
            }
        }

        KDNode n;
        KDNode left = buildKDTree(upperPartition,pointSet);
        KDNode right = buildKDTree(lowerPartition,pointSet);
        KDNode *leftC = new KDNode(left);
        KDNode *rightC = new KDNode(right);

        n.bbox = B;
        n.data = indices;
        n.meanP = mp;
        n.meanN = mn;
        n.area = left.area + right.area;
        n.leftChild = leftC;
        n.rightChild = rightC;
        return n;
    }

    double fastWN(point3d const & q, KDNode const & node, std::vector<Triplet> const & pointSet) {

        //initialization
        point3d treeP = node.meanP; // = ptilde
        double treeR = node.bbox.radius();
        point3d nTilde = node.meanN;

        //computes treeR, maximum distance from tree.p to any of its elements
    /*    for(unsigned i = 0; i<node.data.size(); i++) {
            double temp = (treeP-pointSet[node.data[i]].p).norm();
            if (temp > treeR) treeR = temp;
        }
        */

        if ((q - treeP).norm() > beta * treeR && treeR != 0) {
            double dist = (treeP - q).norm();
            return point3d::dot(treeP - q,nTilde)/(4*M_PI*dist*dist*dist); // = wtilde
        }
        else {
            double val = 0;
            if (node.leftChild == nullptr && node.rightChild == nullptr) {
                for (unsigned int j = 0 ; j<node.data.size() ; j++) {
                    point3d p = pointSet[node.data[j]].p;
                    point3d n = pointSet[node.data[j]].n;
                    double dist = (p - q).norm();
                    val += pointSet[node.data[j]].area*point3d::dot(p - q,n)/(4*M_PI*dist*dist*dist);
                }
            }
            else {
                val += fastWN(q, *node.leftChild, pointSet);
                val += fastWN(q, *node.rightChild, pointSet);
            }
            return val;
        }
    }

};

#endif // KDTREE_H
