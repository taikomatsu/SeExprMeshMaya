#include "SeExprMeshNode.h"
#include "SeExprMeshCmd.h"
#include <maya/MFnPlugin.h>
#include <iostream>

MStatus initializePlugin(MObject obj)
{ 
	MStatus   status;
	MFnPlugin plugin(obj, "Tai Komatsu", "0.5", "Any");
	// register node
	status = plugin.registerNode("seExprMesh", SeExprMeshNode::id, SeExprMeshNode::creator,
								  SeExprMeshNode::initialize);
	if (!status) {
		status.perror("//Node Registration Error");
		return status;
	}
	status = plugin.registerCommand("seExprMesh", SeExprMeshCmd::creator, SeExprMeshCmd::newSyntax);
	if (!status) {
		status.perror("//Command Registration Error");
		return status;
	}

	return status;
}

MStatus uninitializePlugin(MObject obj)
{
	MStatus   status;
	MFnPlugin plugin(obj);
	status = plugin.deregisterNode(SeExprMeshNode::id);
	if (!status) {
		status.perror("//Node Deregistration Error");
		return status;
	}
	status = plugin.deregisterCommand("seExprMesh");
	if (!status) {
		status.perror("//Command Deregistration Error");
		return status;
	}

	return status;
}
