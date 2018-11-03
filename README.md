# Jet OSPRay Studio Plugin
PoC OSPRay Studio plugin to integrate the 'Jet' (fluid-engine-dev) library.

## Build Instructions

Clone this repository into your OSPRay Studio source tree under: ```ospray_studio/plugins```.
Requires an install of [Jet](http://github.com/jeffamstutz/fluid-engine-dev) library
(required CMake exports changes not yet upstreamed).

NOTE: Build Jet with ```JET_TASKING_SYSTEM=Serial```. TBB currently causes a runtime crash.

## Run Instructions

Run with:

```bash
./ospStudio --plugin jet
```

...then find the simulation control panel in the ```view``` menu.
