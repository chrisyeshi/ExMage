ExMage -- Explorable Image
======

Introduction:
------

This repository provides both the library and the viewer of explorable image. The library can be easily plugged into a scientific simulation for in situ pathtube visualization. The output from the library is an explorable image. An explorable image allows the users to explore the generated volume with the most common techniques, including transfer function, lighting, and rotation. Also, an explorable image can be opened by any standard png viewer as a static image. However, when it is opened by our viewer (also provided in the repository), user can perform the exploration techniques that are mentioned before.

Dependencies:
------

OSMesa (for offscreen rendering)
Any MPI library

Installation:
------

### The Library

For the library, we use the cmake system to build.

```bash
mkdir build
cmake ..
make
```

Options such like whether to build the examples and tests are available in cmake.

### The Viewer

For the viewer, we use the Qt library. Just open the exmage-viewer.pro file in the viewer directory with Qt Creater, it will configure everything correctly.

Usage:
------

Please view the documentation and the supernova example located in /insitu/examples/supernova for detailed usage.
