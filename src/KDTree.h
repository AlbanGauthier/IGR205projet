#ifndef KDTREE_H
#define KDTREE_H

#include "Mesh.h"
#include "point3.h"
#include <math.h>

struct BBox{
    double xMin, xMax, yMin, yMax, zMin, zMax;
};

struct KDNode{
    BBox bbox;
    std::vector<int> data;
    KDNode *leftChild;
    KDNode *rightChild;
};

struct KDTree {

    KDNode root;

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

    int findMedianNormal(std::vector<int> const & sortedIndices, std::vector<Triplet> & pointSet){
        if (sortedIndices.size() % 2 == 1){
            return sortedIndices[(sortedIndices.size()-1)/2];
        }
        else{
            return (sortedIndices[sortedIndices.size()/2] + sortedIndices[sortedIndices.size()/2 + 1])/2;
        }
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
            n.leftChild = nullptr;
            n.rightChild = nullptr;
            return n;
        }

        BBox B = computeBoundingBox(indices, pointSet);
        int maxAxis = findMaxAxis(B);
        std::vector<int> const & sortedInd = sortIndAlongAxis(indices, maxAxis, pointSet);
        int q = findMedianSample(sortedInd, pointSet);
        int o = findMedianNormal(sortedInd, pointSet);

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
        n.leftChild = leftC;
        n.rightChild = rightC;
        return n;
    }

    double fastWN(point3d const & q, std::vector<int> const & indices, std::vector<Triplet> const & pointSet) {

        //initialization
        double beta = 2.3; // accuracy : the article cites 2 for triangles, 2.3 for points
        point3d treeP = point3d(0,0,0); // = ptilde
        double treeR = 0;
        point3d nTilde = point3d(0,0,0);
        double areaSum = 0;

        //computes treeP=ptilde and nTilde
        for(unsigned i = 0; i<indices.size(); i++) {
            double area = pointSet[indices[0]].area;
            treeP += area * pointSet[indices[0]].p;
            nTilde += area * pointSet[indices[0]].n;
            areaSum += area;
        }
        treeP /= areaSum;

        //computes treeR, maximum distance from tree.p to any of its elements
        for(unsigned i = 0; i<indices.size(); i++) {
            double temp = (treeP-pointSet[indices[0]].p).norm();
            if (temp > treeR) treeR = temp;
        }

        if ((q - treeP).norm() > beta * treeR) {
            return point3d::dot(treeP - q,nTilde)/(4*M_PI*(treeP - q).norm()); // = wtilde
        } else {
            val = 0;
            if (root.data.size() == 0 && root.data.size() == 0) {
                for (unsigned int j = 0 ; j<root.data.size() ; j++) {
                    point3d p = pointSet[root.data[j]].p;
                    point3d n = pointSet[root.data[j]].n;
                    val += point3d::dot(p - q,n)/(4*M_PI*(p - q).norm());
                }
            } else {
                val += fastWN(q, root.leftChild->data, pointSet);
                val += fastWN(q, root.rightChild->data, pointSet);
            }
            return val;
        }
    }

};

#endif // KDTREE_H
