# Custom-format-Maya-C++-API
My own custom format using the Maya C++ API

Ongoing project

Creating my own custom format using the C++ API for Autodesk Maya. 
The plugin traverse the selected objects and its children, in order to gather the appropriate information needed for usage in a game engine. 
The information is then printed to a binary file with in a logical manner. 

The file format currently involves:
1. Meshes and vertex information
2. Transformation and hierarchy
3. Lambert material
4. Diffuse and Normal textures


Future implementations:
1. Skeleton animations using skinned meshes
2. Morph animations
3. Phong material
4. Various textures such as specular, displacement e.t.c..
5. PBR material
6. Import tool, so that the user could import the custom format into Maya afterwards
