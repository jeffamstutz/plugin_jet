# Jet OSPRay Studio Plugin

PoC OSPRay Studio plugin to integrate the 'Jet' (fluid-engine-dev) library.

## Build Instructions

Clone this repository into your OSPRay Studio source tree under:
```ospray_studio/plugins```.  Requires an install of
[Jet](http://github.com/doyubkim/fluid-engine-dev) library. Note that the
latest release of Jet, v1.3.1, does not yet have required CMake exports.
However, changes are upstreamed now so simply build ```master``` of that
project until a newer release is available.

## Run Instructions

Run with:

```bash
./ospStudio --plugin jet
```

...then find the simulation control panel in the ```Panels``` menu.

