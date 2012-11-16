SeExprMeshMaya
==============

Polygon deformer node plugin for Maya that using Disney's SeExpr


Introduction
------------
This plug-in is allowed you to edit meshes with Disney's __SeExpr__ language.  
You can develop your unique mesh deformer, furthermore, can coding just the method without troublesome C++ codings and plug-in declaration. 


What's SeExpr?
--------------
[Walt Disney Animation Studios: SeExpr](http://www.disneyanimation.com/technology/seexpr.html "SeExpr")  
SeExpr is a expression language and the library developed by Disney.  
It's a simple and easy, but has enough functions what you want in your daily work.  
You can find it from [here](http://wdas.github.com/SeExpr/doxygen/userdoc.html "User Documentation").  

Build
-----
I developed this plug-in on Linux and I don't have any environment.  
So I didn't build it on Win/Mac environment. 

If you are a user of Maya2013/Linux, you can use pre-compiled binaly. 
Please download it from [here(https://github.com/taikomatsu/SeExprMeshMaya/downloads) "Download"). 

#### Linux
1. [Download the source.](https://github.com/taikomatsu/SeExprMeshMaya/downloads "Download")
2. Unzip and cd to src directory.
3. Edit the Makefile for your environment.
4. run `make`

_This plug-in uses OpenMP._ 
_So it's compiled by gcc4.2 or higher._ 

Install
-------
On Linux, the simplest way is `make install` after Makefile modification.  
If you don't want it, you have to move some files to specified path.

- __build/SeExprMesh.so__ to any MAYA_PLUG_IN_PATH directory.
- __scripts/AESeExprMeshTemplate.mel__ to any MAYA_SCRIPT_PATH directory.


Usage
-----
1. Start Maya

2. Open Plug-in Manager from Window > Settings/Preferences > Plug-in Manager.

3. Load SeExprMesh.so(mll).

4. Select a mesh.

5. Execute seExprMesh command.

  `seExprMesh;`

6. You can find seExprMesh node from the network.


Attributes
----------
#### enable
Do process or not.

#### outType
You can select some output type.
- __Position__ 
- __Normal__ 
- __Color__ 
- __uv__ 

*color and uv is processed on current colorSet or uvSet.*

#### seExprStr
Enter your SeExpr expression here. 
In this field, you can use extra attributes as SeExpr variable directly. 
But it's just limited __float__ and __vector__. 

#### envelope
Deformation influence. 
0 is no deformation, 1 is full.

#### time
You have to connect here from _time1.outTime_ if you use __time__ or __frame__ local variable. 
This parameter drives all time value on the node.

#### inMesh
The mesh be processed.

#### outMesh
Processed mesh.

#### ctrlMesh
Additional control mesh. 
If you want to use __closestPoint()__ or __closestNormal()__, have to connect any mesh.

#### inMatrix
If you want to move the object by its transform, and follow the deformation, you should connect matrix attribute here. 
Furthermore, this attribute is shown in Attribute Editor. 
Because when you unconnect it, its values may not reset right. 
Then you have to clear the unnecessary values manually.

Local Variables
---------------
#### P (vector)
Position

#### N (vector)
Normal

#### Cd (vector)
Vertex color

#### u (scalar)
Texture U coord

#### v (scalar)
Texture V coord

#### time (scalar)
Current time in second

#### frame (scalar)
Current time in frame


Additional Functions
--------------------
#### closestPoint(pos)
Get closest point position on __ctrlMesh__ from given pos.

#### closestNormal(pos)
Get closest normal on __ctrlMesh__ from given pos.


Contact
-------
If you have some ideas, or found bugs, please tell me it.

mail: taikomatsu __AT__ gmail.com
(Please replace __AT__ to @)

