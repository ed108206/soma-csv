# CSV Viewer + Splitter - soma-csv

![CMake Build](https://github.com/ed108206/soma-csv/actions/workflows/cmake-multi-platform.yml/badge.svg)

# Old Project (Deprecated)

⚠️ This project is **deprecated** and no longer recommended for use.  
It has been **superseded** by a newer, more efficient, and actively maintained project.  

👉 Please use the latest project instead: [soma-hex](https://github.com/ed108206/soma-hex)

---

This project is a small Win32 desktop app that combines a custom splitter control, a tree view, and fast CSV loading.  
It's written in C++ using plain Win32 APIs with a focus on speed and simplicity.

## Features

- **Custom splitter control**  
  A lightweight splitter implemented from scratch.  
  It lets you resize panels inside the main window easily.

- **Fast CSV loading**  
  Large CSV files are memory-mapped and parsed quickly.  
  The app can handle millions of rows without freezing the UI.

- **Parallel parsing**  
  CSV parsing is done in parallel threads to keep things responsive.  
  This makes opening big files noticeably faster.

- **Tree view with drag & drop**  
  A side panel shows a tree view with categories.  
  Items can be dragged and dropped to reorganize them.


## UI Layout

Here's a simple ASCII diagram of the main window layout:


```text
+---------------------------------------------------+
|                  Main Window                      |
|                                                   |
|  +-----------+-------------------------------+    |
|  | ExtraTree |       CEView /CLView          |    |
|  | (Tree)    |   (CSV Data / ListView)       |    |
|  |           |                               |    |
|  +-----------+-------------------------------+    |
|  |              (Logs / Bottom)              |    |
|  +-------------------------------------------+    |
|                                                   |
+---------------------------------------------------+
```

- Left panel: **ExtraTree** (tree view with drag & drop).  
- Right panel: **EView** (CSV data) / **LView** (CSV fix data)  
- Bottom panel: Logs  
- Panels are resizable thanks to the custom **SplitterE**.

## CSV Parser 


Performance results  (10.5 GB dataset)
- Ratio: 25M rows / 13.56 s ≈ 1.84M rows/second. 

## Build

You can build the project in two ways:

- **Makefile** (GNU toolchain)  
- **CMakeLists.txt** (cross-platform build system)

> **Note:** The project has been tested with GNU compilers (MinGW).  
> It may also compile with Microsoft Visual C++ (MSVC), but has not been tested.

### Example commands

```bash
Using Makefile:
make

Using CMake:
mkdir build
cd build
cmake ..
make
```

## CSV Generator (generator.cpp)

There's also a small helper program included: **generator.cpp**.  
Its only purpose is to create large CSV files for testing the viewer and parser.<br>
This makes it easy to stress-test the app with big datasets and see how the splitter and views behave under load.
