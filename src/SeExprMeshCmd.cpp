#include <maya/MDagPath.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MSelectionList.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnMesh.h>
#include <maya/MGlobal.h>
#include "SeExprMeshCmd.h"
#include "SeExprMeshNode.h"
#include <iostream>

const char *envFlag = "-en", *envFlagLong = "-envelope";
const char *expstrFlag = "-str", *expstrFlagLong = "-expstr";
const char *modeFlag = "-m", *modeFlagLong = "-mode";

void getTimeNode(MObject& timeNode)
{
	MSelectionList sel;
	sel.clear();
	sel.add(MString("time1"));
	sel.getDependNode(0, timeNode);
}

void* SeExprMeshCmd::creator()
{
	return new SeExprMeshCmd;
}

MSyntax SeExprMeshCmd::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag(envFlag, envFlagLong, MSyntax::kDouble);
	syntax.addFlag(expstrFlag, expstrFlagLong, MSyntax::kString);
	syntax.addFlag(modeFlag, modeFlagLong, MSyntax::kLong);
	syntax.useSelectionAsDefault(true);
	syntax.setObjectType(MSyntax::kSelectionList, 0, 1);
	return syntax;
}

MStatus SeExprMeshCmd::doIt(const MArgList &args)
{
	double env = 1.0;
	int mode = 0;
	MString expstr("P");
	MSelectionList tgtObj;

	MArgDatabase argData(syntax(), args);
	if (argData.isFlagSet(envFlag))
		argData.getFlagArgument(envFlag, 0, env);
	if (argData.isFlagSet(expstrFlag))
		argData.getFlagArgument(expstrFlag, 0, expstr);
	if (argData.isFlagSet(modeFlag))
		argData.getFlagArgument(modeFlag, 0, mode);

	argData.getCommandArgument(0, tgtObj);

	// get target object
	if (tgtObj.isEmpty()) {
		argData.getObjects(tgtObj);
		if (tgtObj.isEmpty()) {
			MGlobal::displayError("No object given. Please select a mesh object.");
			return MS::kFailure;
		}
	}

	// get MDagPath from string
	MDagPath dagMesh;
	tgtObj.getDagPath(0, dagMesh);

	// extend to shape when transform was given 
	if (dagMesh.apiType() == MFn::kTransform)
		dagMesh.extendToShape();

	// occuring error when invalid object was given
	if (dagMesh.apiType() != MFn::kMesh) {
		MGlobal::displayError("No mesh object given. Please select a mesh object.");
		return MS::kFailure;
	}

	// insert seExprMesh node to current network
	MObject origMesh = dagMesh.node();
	MFnDependencyNode meshFn(origMesh);
	
	MPlug inMeshPlug = meshFn.findPlug("inMesh");
	MPlug outMeshPlug;
	
	MPlugArray conns;
	inMeshPlug.connectedTo(conns, true, false); // find dst-side only
	if (conns.length() > 0) { // has connection
		outMeshPlug = conns[0]; // set first connection
		dgMod.disconnect(outMeshPlug, inMeshPlug);
		dgMod.doIt();
	} else { // has no connection
		// get transform
		// it is needed to generate construction history on the mesh
		MDagPath transform(dagMesh);
		// from mesh hierarchy to transform hierarchy 
		transform.pop(); 

		// copy geometry as intermediateObject
		MFnMesh fnMesh;
		MObject intermediate = fnMesh.copy(origMesh, transform.node());

		// set to intermediate object
		meshFn.setObject(intermediate);
		MPlug intermediatePlug = fnMesh.findPlug("intermediateObject");
		intermediatePlug.setValue(true);

		outMeshPlug = fnMesh.findPlug("outMesh");
	}

	// create and connect seExprNode
	MObject seMeshNode = dgMod.createNode(SeExprMeshNode::id);
	MFnDependencyNode seMeshNodeFn(seMeshNode);
	MString nodename = seMeshNodeFn.name();

	dgMod.connect(outMeshPlug, seMeshNodeFn.findPlug("inMesh"));
	dgMod.connect(seMeshNodeFn.findPlug("outMesh"), inMeshPlug);

	// set initial value
	seMeshNodeFn.findPlug("envelope").setValue(env);
	seMeshNodeFn.findPlug("outType").setValue(mode);
	seMeshNodeFn.findPlug("seExprStr").setValue(expstr);
	
	// connect to time
	MObject timeNode;
	getTimeNode(timeNode);
	MFnDependencyNode fnTime(timeNode);
	dgMod.connect(fnTime.findPlug("outTime"), seMeshNodeFn.findPlug("time"));

	// set result (node name)
	clearResult();
	setResult(seMeshNodeFn.name());

	return redoIt();
}

MStatus SeExprMeshCmd::undoIt()
{
	return dgMod.undoIt();
}

MStatus SeExprMeshCmd::redoIt()
{
	return dgMod.doIt();
}


