#ifndef _seExprMeshNode_h_
#define _seExprMeshNode_h_

#include <maya/MPxNode.h>
#include <maya/MObject.h>
#include <maya/MMatrix.h>
#include <maya/MTypeId.h>
#include <maya/MPlugArray.h>
#include <maya/MItMeshVertex.h>
#include <maya/MFnMesh.h>
#include <map>
#include "MeshExpression.h"

class SeExprMeshNode: public MPxNode
{
public:
						SeExprMeshNode();
	virtual 			~SeExprMeshNode();
	virtual	MStatus 	compute(const MPlug&, MDataBlock&);
	static	void*		creator();
	static	MStatus		initialize();
	virtual MStatus		setDependentsDirty(const MPlug&, MPlugArray&);
			MStatus		getDynamicAttrs(const MObject&, MPlugArray&) const;
			MStatus 	dynamicAttrsAsMap(
							const MPlugArray&,
							DynScalarAttrValue&,
							DynVectorAttrValue&) const;
			MStatus		execSeExpr(
							      MObject&,
							const MFnMesh&,
							      MFnMesh&,
							      MObject&,
							const MMatrix&,
							const std::string&,
							const int,
							const float,
							const int,
							const double
							);

public:
	MPlugArray dynPlugs;
	static MTypeId id;
	static MObject aEnable;
	static MObject aInMesh;
	static MObject aOutMesh;
	static MObject aCtrlMesh;
	static MObject aInMatrix;
	static MObject aTime;
	static MObject aOutType;
	static MObject aSeExpr;
	static MObject aEnvelope;
};

#endif
