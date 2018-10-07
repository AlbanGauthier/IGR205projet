# Inside-Outside Segmentation using  <br /> Fast Winding Numbers Approximation
 Student Project by Alban Gauthier and Maud Lastic <br />
 IGR 205 Project Seminar - Computer Graphics and Interaction <br /> <br />
 
 This project implements a Robust Inside Outside Segmentation using a Fast Winding Numbers Algorithm to speed up the computation
 with the help of two different research papers. It was written in C++ using the Qt framework.

### Prerequisites

Qt Creator

### How to use

Tested on Ubuntu 16.04LTS

```
Commands:
H  :   Help appear
O  :   Open a mesh
I  :   initialize tetraedralizaton, kdtree construction & winding number
M  :   show mesh
T  :   show the result of the tetrahedrization
W  :   show tetraedrons with winding number > threshold
G  :   show graph cut optimization results
Q  :   show query point set
P  :   show original point set
Left & Right arrow : change cutting plane position
```

## Paper

The link of the resulting paper can be found here (in French): <br/>
https://www.dropbox.com/s/ti82xxb5t8mkkg1/igr205-volumetrisation.pdf?dl=0

## Built With

* [Qt](https://www.qt.io/) - The framework used.
* [libQGLViewer](http://libqglviewer.com/) A C++ library based on Qt that eases the creation of OpenGL 3D viewers.
* [TetGen](http://wias-berlin.de/software/tetgen/) A program to generate tetrahedral meshes of any 3D polyhedral domains.
* [MAXFLOW](http://pub.ist.ac.at/~vnk/software.html) A software for computing mincut/maxflow in a graph. <br /> 
Code taken from the project of Ngoc-Trung Tran at this address : https://github.com/tntrung/graphcut_segmentation


## Bibliography
* Jacobson et al. Robust Inside-Outside Segmentation using Generalized Winding Numbers,<br />
2013, ACM Transactions on Graphics, Vol. 32, No. 4, Article 33. <br />
http://igl.ethz.ch/projects/winding-number/
* Fast Winding Numbers for Soups and Clouds, <br />
2018, ACM Transactions on Graphics, s, Vol. 37, No. 4, Article 43. <br />
http://www.dgp.toronto.edu/projects/fast-winding-numbers/
* An Experimental Comparison of Min-Cut/Max-Flow Algorithms for Energy Minimization in
Vision. <br />
Yuri Boykov and Vladimir Kolmogorov. In IEEE Transactions on Pattern Analysis
and Machine Intelligence (PAMI), September 2004

## Authors
* **Alban Gauthier** - *Engineer Student at Télécom ParisTech* - [AlbanGauthier](https://github.com/AlbanGauthier)
* **Maud Lastic** - *Engineer Student at Ecole Polytechnique* - [maudlastic](https://github.com/maudlastic)

See the list of [contributors](https://github.com/AlbanGauthier/IGR205projet/contributors) who participated in this project.

## Acknowledgments
* **Jean-Marc Thierry** for his precious help during the whole project
