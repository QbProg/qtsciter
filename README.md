**Qt Sciter**

A ready to use example to embed sciter lite as a Qt6 Widget.

**Please consider this an example, not a complete library**

Notes:
- Works with Qt6 + Visual Studio 2022 x64 + CMake
- sciter usage is private, so no sciter dependencies are exposed
- Uses raster skia offline rendering, bridged to Qt using QImage+QPainter
- QCursor and keyboard support

**Instructions**

- Extract sciter SDK (versions 5.x) into the "sdk" folder (so that the sdk/include and sdk/bin.lite exist).
- **Edit** the *setup.bat* file to match your Qt folder and version
- run setup.bat
- find your build environment in build/QtSciter.sln
- build & run

Alternatively, just use CMake in your preferred way :)
