### Introduction:

In the current stage, XImage provides in situ pathtube generation and visualization. It is an image based technique that tremendously reduce the output data size yet retains certain user exploration techniques, such like transfer function, cut plane, etc. The library outputs a unique image format which can be opened as a static image by any png viewer. However, when it is opened by our specialized viewer (also provided in the repository), user interactions are enabled.

This repository provides both the library and the viewer of explorable image. The library can be easily plugged into a scientific simulation for in situ pathtube visualization. The output from the library is an explorable image. An explorable image allows the users to explore the generated volume with the most common techniques, including transfer function, lighting, and rotation. Also, an explorable image can be opened by any standard png viewer as a static image. However, when it is opened by our viewer (also provided in the repository), user can perform the exploration techniques that are mentioned before.

The XImage library for scalar field volume visualization: [https://chrisyeshi.github.io/ximage-scalar](https://chrisyeshi.github.io/ximage-scalar)

### Dependencies:

OSMesa (for offscreen rendering)
Any MPI library

### Installation:

#### The Library

Additional documentation: [assets/LibraryDocumentation.pdf](../assets/LibraryDocumentation.pdf)

For the library, we use the cmake system to build.

```bash
mkdir build
cmake ..
make
```

Options such like whether to build the examples and tests are available in cmake.

#### The Viewer

Additional documentation: [assets/ViewerDocumentation.pdf](../assets/ViewerDocumentation.pdf)

For the viewer, we use the Qt library. Just open the exmage-viewer.pro file in the viewer directory with Qt Creater, it will configure everything correctly.

### Usage:

Please view the documentation and the supernova example located in /insitu/examples/supernova for detailed usage.

### Related Publications

**In Situ Pathtube Visualization with Explorable Images**  
Yucong (Chris) Ye, Robert Miller, and Kwan-Liu Ma  
Eurographics 2013 Symposium on Parallel Graphics and Visualization, pp. 9-16.  
http://dx.doi.org/10.2312/EGPGV/EGPGV13/009-016

**Visualization by Proxy: A Novel Framework for Deferred Interaction with Volume Data**  
Anna Tikhonova, Carlos D. Correa, Kwan-Liu Ma  
IEEE Transactions on Visusalization and Computer Graphics, 16(6):1551-1559 (2010)  
http://dx.doi.org/10.1109/TVCG.2010.215

### Authors and Contributors

Chris Ye ([@chrisyeshi](https://github.com/chrisyeshi)), Bob Miller ([@dairukan](https://github.com/dairukan)), and Kwan-Liu Ma ([@](http://www.cs.ucdavis.edu/~ma))

For further information, please contact Kwan-Liu Ma at [ma@cs.ucdavis.edu](ma@cs.ucdavis.edu)

### Acknowledgments

These research projects were sponsored in part by U.S. Department of Energy SciDAC Program through grants DE-FC02-06ER25777, DE-CS0005334, and DE-FC02-12ER26072 with program managers Lucy Nowell and Ceren Susut-Bennett.
