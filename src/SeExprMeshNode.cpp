#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MDataHandle.h>
#include <maya/MAnimControl.h>
#include <maya/MMatrix.h>
#include <maya/MFloatMatrix.h>
#include <maya/MTime.h>
#include <maya/MTypeId.h>
#include <maya/MPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MString.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnStringData.h>
#include <maya/MFnAttribute.h>
#include <maya/MUintArray.h>
#include <maya/MGlobal.h>
#include "SeExprMeshNode.h"
#include "MeshExpression.h"
#include "ClosestPointFunc.h"
#include <string>
#include <algorithm>

// This is a tmporary value.
// You can change this.
MTypeId SeExprMeshNode::id(0x97512);

MObject SeExprMeshNode::aEnable;
MObject SeExprMeshNode::aInMesh;
MObject SeExprMeshNode::aOutMesh;
MObject SeExprMeshNode::aCtrlMesh;
MObject SeExprMeshNode::aInMatrix;
MObject SeExprMeshNode::aTime;
MObject SeExprMeshNode::aOutType;
MObject SeExprMeshNode::aSeExpr;
MObject SeExprMeshNode::aEnvelope;

#define MAKE_INPUT_WITH_PARAM(attr, k, s, r, w)					\
    CHECK_MSTATUS (attr.setKeyable((k)));	\
	CHECK_MSTATUS (attr.setStorable((s)));	\
    CHECK_MSTATUS (attr.setReadable((r)));	\
	CHECK_MSTATUS (attr.setWritable((w)));

#define MAKE_INPUT(attr)					\
    CHECK_MSTATUS (attr.setKeyable(true));	\
	CHECK_MSTATUS (attr.setStorable(true));	\
    CHECK_MSTATUS (attr.setReadable(true));	\
	CHECK_MSTATUS (attr.setWritable(true));

#define MAKE_OUTPUT(attr)						\
    CHECK_MSTATUS (attr.setKeyable(false));		\
	CHECK_MSTATUS (attr.setStorable(false));	\
    CHECK_MSTATUS (attr.setReadable(true));		\
	CHECK_MSTATUS (attr.setWritable(false));


template<class T> inline void calcFinalVal(int i, SeVec3d& val, T& in, float envelope)
{
	val[0] = in[i][0] + (val[0] - in[i][0]) * envelope;
	val[1] = in[i][1] + (val[1] - in[i][1]) * envelope;
	val[2] = in[i][2] + (val[2] - in[i][2]) * envelope;
}

inline void getVertexIndexs(MIntArray& vlist)
{
	for (int i=0; i<vlist.length(); ++i)
		vlist[i] = i;
}

SeExprMeshNode::SeExprMeshNode() {}

SeExprMeshNode::~SeExprMeshNode() {}

MStatus SeExprMeshNode::compute(const MPlug& plug, MDataBlock& data)
{
	if (plug == aOutMesh) {
		bool isEnabled = data.inputValue(aEnable).asBool();
		MObject inMesh = data.inputValue(aInMesh).asMeshTransformed();
		MObject ctrlMesh = data.inputValue(aCtrlMesh).asMeshTransformed();
		MMatrix mat = data.inputValue(aInMatrix).asMatrix();
		MTime time = data.inputValue(aTime).asTime();
		int outType = data.inputValue(aOutType).asShort();
		MString exprStr = data.inputValue(aSeExpr).asString();
		float envelope = data.inputValue(aEnvelope).asFloat();
		MDataHandle outMesh_hdl = data.outputValue(aOutMesh);

		if (inMesh.isNull()) {
			MString nodeName = MFnDependencyNode(thisMObject()).name();
			MGlobal::displayWarning(nodeName + " has no inMesh.");
			return MS::kFailure;
		}

		MFnMesh inMeshFn(inMesh);
		MObject outMesh;
		MFnMeshData meshDataFn;
		MFnMesh outMeshFn;
		MObject outMeshData = meshDataFn.create();
		outMesh = inMeshFn.copy(inMesh, outMeshData);
		outMeshFn.setObject(outMesh);

		// same as has no effect
		if (isEnabled == false) {
			outMesh_hdl.set(outMeshData);
			data.setClean(plug);
			return MS::kSuccess;
		}

		int frame = static_cast<int>(time.value());
		double second = time.as(MTime::kSeconds);

		// deform meshes here
		execSeExpr(
			inMesh,
			inMeshFn,
			outMeshFn,
			ctrlMesh,
			mat,
			std::string(exprStr.asChar()),
			outType,
			envelope,
			frame, 
			second);

		outMesh_hdl.set(outMeshData);
		data.setClean(plug);
	} else {
		return MS::kUnknownParameter;
	}

	return MS::kSuccess;
}

MStatus SeExprMeshNode::getDynamicAttrs(const MObject& node, MPlugArray& dynPlugs) const
{
	MFnDependencyNode nodeFn(node);
	MObject attr;
	unsigned int numAttrs = nodeFn.attributeCount();
	for (int i=0; i<numAttrs; ++i) {
		attr = nodeFn.attribute(i);
		MFnAttribute attrFn(attr);
		if (attrFn.isDynamic()) {
			dynPlugs.append(MPlug(node, attr));
		}
	}
	return MS::kSuccess;
}

char* getNameFromPlug(const MPlug& plug)
{
	return const_cast<char*>(plug.partialName(false, false, false, false, false, true).asChar());
}

MStatus SeExprMeshNode::dynamicAttrsAsMap(
	const MPlugArray& plugs,
	DynScalarAttrValue& dynScalarAttrs,
	DynVectorAttrValue& dynVectorAttrs) const
{
	std::string name;
	double v = 0.0;
	MVector vec;
	unsigned int numPlugs = plugs.length();
	for (int i=0; i<numPlugs; ++i) {
		if (plugs[i].isCompound()) {
			plugs[i].child(0).getValue(vec[0]);
			plugs[i].child(1).getValue(vec[1]);
			plugs[i].child(2).getValue(vec[2]);
			name.assign(getNameFromPlug(plugs[i]));
			dynVectorAttrs.insert(DynVectorAttrValue::value_type(name, vec));
		} else {
			v = plugs[i].asDouble();
			name.assign(getNameFromPlug(plugs[i]));
			dynScalarAttrs.insert(DynScalarAttrValue::value_type(name, v));
		}
	}
	return MS::kSuccess;
}

MStatus SeExprMeshNode::setDependentsDirty(const MPlug& plugBeingDirtied, MPlugArray& affectedPlugs)
{
	unsigned int numPlugs = dynPlugs.length();
	for (int i=0; i<numPlugs; ++i) {
		if (plugBeingDirtied == dynPlugs[i]) {
			MObject thisNode = thisMObject();
			MPlug outMeshPlug(thisNode, SeExprMeshNode::aOutMesh);
			affectedPlugs.append(outMeshPlug);
		}
	}
	return MS::kSuccess;
}

void* SeExprMeshNode::creator()
{
	return new SeExprMeshNode();
}

MStatus SeExprMeshNode::initialize()
{
	MFnTypedAttribute tAttr;
	MFnMatrixAttribute mAttr;
	MFnUnitAttribute uAttr;
	MFnEnumAttribute eAttr;
	MFnNumericAttribute nAttr;
	MFnStringData strData;

	aEnable = nAttr.create("enable", "en", MFnNumericData::kBoolean);
	nAttr.setDefault(true);
	MAKE_INPUT(nAttr);

	aInMesh = tAttr.create("inMesh", "im", MFnData::kMesh);
	MAKE_INPUT(tAttr);

	aOutMesh = tAttr.create("outMesh", "om", MFnData::kMesh);
	MAKE_OUTPUT(tAttr);

	aCtrlMesh = tAttr.create("ctrlMesh", "cm", MFnData::kMesh);
	MAKE_INPUT(tAttr);

	aInMatrix = mAttr.create("inMatrix", "imat", MFnMatrixAttribute::kDouble);
	MAKE_INPUT_WITH_PARAM(tAttr, true, false, true, true);

	aTime = uAttr.create("time", "t", MFnUnitAttribute::kTime, 0.0);
	MAKE_INPUT(uAttr);

	aOutType = eAttr.create("outType", "ot", 0);
	eAttr.addField("Position", 0);
	eAttr.addField("Normal", 1);
	eAttr.addField("Color", 2);
	eAttr.addField("uv", 3);
	MAKE_INPUT(eAttr);

	MString initialExpr("P");
	aSeExpr = tAttr.create("seExprStr", "sestr", MFnData::kString, strData.create(initialExpr));
	MAKE_INPUT(tAttr);

	aEnvelope = nAttr.create("envelope", "e", MFnNumericData::kFloat, 1.f);
	MAKE_INPUT(nAttr);
	nAttr.setSoftMin(0.f);
	nAttr.setSoftMax(1.f);

	CHECK_MSTATUS(addAttribute(aEnable));
	CHECK_MSTATUS(addAttribute(aInMesh));
	CHECK_MSTATUS(addAttribute(aOutMesh));
	CHECK_MSTATUS(addAttribute(aCtrlMesh));
	CHECK_MSTATUS(addAttribute(aInMatrix));
	CHECK_MSTATUS(addAttribute(aTime));
	CHECK_MSTATUS(addAttribute(aOutType));
	CHECK_MSTATUS(addAttribute(aSeExpr));
	CHECK_MSTATUS(addAttribute(aEnvelope));

	CHECK_MSTATUS(attributeAffects(aEnable, aOutMesh));
	CHECK_MSTATUS(attributeAffects(aInMesh, aOutMesh));
	CHECK_MSTATUS(attributeAffects(aCtrlMesh, aOutMesh));
	CHECK_MSTATUS(attributeAffects(aInMatrix, aOutMesh));
	CHECK_MSTATUS(attributeAffects(aTime, aOutMesh));
	CHECK_MSTATUS(attributeAffects(aOutType, aOutMesh));
	CHECK_MSTATUS(attributeAffects(aSeExpr, aOutMesh));
	CHECK_MSTATUS(attributeAffects(aEnvelope, aOutMesh));

	return MS::kSuccess;
}

MStatus SeExprMeshNode::execSeExpr(
	      MObject& inMesh,
	const MFnMesh& inMeshFn,
	      MFnMesh& outMeshFn, // output value
	      MObject& ctrlMesh,
	const MMatrix& mat,
	const std::string& expstr,
	const int outType,
	const float envelope,
	const int frame,
	const double second
)
{
	// this object is used for only validation check.
	// not for deformation.
	MeshExpression expr(expstr);

	// make inverse matrix for return to current space
	const MMatrix imat = mat.inverse();

	// make MFloatMatrix for normal transforming
	float matElems[4][4];
	mat.get(matElems);
	const MFloatMatrix ifmat = MFloatMatrix(matElems).inverse();

	MPointArray inPs, outPs;
	//MFloatVectorArray inNs, outNs;
	MFloatVectorArray inNs;
	MVectorArray outNs;
	MColorArray inCds, outCds;
	MString colorSetName, uvSetName;
	MFloatArray inUs, inVs, outUs, outVs;

	// get necessary values for SeExpr eval from the mesh
	inMeshFn.getPoints(inPs);
	unsigned int pNums = inPs.length();

	// set normal if it was used
	if (expr.usesVar("N") || outType==1)
		inMeshFn.getVertexNormals(false, inNs, MSpace::kWorld);

	// set color if it was used
	if (expr.usesVar("Cd") || outType==2) { 
		inMeshFn.getCurrentColorSetName(colorSetName);
		if (colorSetName == "")
			colorSetName = "colorSet1";
		MUintArray uints;
		outMeshFn.createColorSetWithName(colorSetName, NULL, &uints);
		inCds.setLength(pNums);
		for (int i=0; i<inCds.length(); ++i)
			inCds.set(MColor::kRGB, 0, 0, 0, 1);
		inMeshFn.getColors(inCds);
		if (inCds.length() == 0)
			inCds.setLength(pNums);
	}

	// set uv if it was used
	if (expr.usesVar("u") || expr.usesVar("v") || outType==3) {
		inMeshFn.getCurrentUVSetName(uvSetName);
		inMeshFn.getUVs(inUs, inVs, &uvSetName);
	}

	// Set up dynamic attrs to be SeExpr's local variables
	DynScalarAttrValue dynScalarAttrs;
	DynVectorAttrValue dynVectorAttrs;
	getDynamicAttrs(thisMObject(), dynPlugs);
	dynamicAttrsAsMap(dynPlugs, dynScalarAttrs, dynVectorAttrs);
	if (dynScalarAttrs.size() > 0)
		expr.setDynamicScalarAttrs(dynScalarAttrs);
	if (dynVectorAttrs.size() > 0)
		expr.setDynamicVectorAttrs(dynVectorAttrs);

	// set value to uniform variables
	if (expr.usesVar("frame"))
		expr.setFrame(frame);
	if (expr.usesVar("time"))
		expr.setTime(second);

	// setup MMeshIntersector for define closestXXX Func
	MMeshIntersector mIntersect;
	mIntersect.create(ctrlMesh);
	ClosestPointPositionFunc cpFunc(&mIntersect);
	ClosestPointNormalFunc cnFunc(&mIntersect);
	if (expr.usesFunc(ClosestPointPositionFunc::funcName))
		ClosestPointPositionFunc::define(cpFunc);
	if (expr.usesFunc(ClosestPointNormalFunc::funcName))
		ClosestPointNormalFunc::define(cnFunc);

	// check expression validity
	if (!expr.isValid()) {
		MGlobal::displayError("[SeExpr] Invalid Expression.");
		return MS::kFailure;
	}

	if (outType == 0) // Position
		outPs.setLength(pNums);
	else if (outType == 1) // Normal
		outNs.setLength(pNums);
	else if (outType == 2) // Color
		outCds.setLength(pNums);
	else { // uv
		outUs.setLength(pNums);
		outVs.setLength(pNums);
	}


#ifdef _OPENMP
#pragma omp parallel for
#endif
	for(int i=0; i<pNums; ++i) {
		MeshExpression exprmt(expstr);
		// set value to uniform variables //
		if (exprmt.usesVar("frame"))
			exprmt.setFrame(frame);
		if (exprmt.usesVar("time"))
			exprmt.setTime(second);
		if (dynScalarAttrs.size() > 0)
			exprmt.setDynamicScalarAttrs(dynScalarAttrs);
		if (dynVectorAttrs.size() > 0)
			exprmt.setDynamicVectorAttrs(dynVectorAttrs);

		// set value to varying variables //
		exprmt.setP(inPs[i] * mat);
		if (exprmt.usesVar("N") || outType==1)
			exprmt.setN(MVector(inNs[i]) * mat);
		if (exprmt.usesVar("Cd") || outType==2)
			exprmt.setCd(inCds[i]);
		if (exprmt.usesVar("u") || exprmt.usesVar("v") || outType==3)
			exprmt.setUV(inUs[i], inVs[i]);

		// Evaluate //
		SeVec3d val = exprmt.evaluate();

		// set calculated value
		if (outType == 0) { // Position
			if (envelope < 1.f) {
				calcFinalVal<MPointArray>(i, val, inPs, envelope);
			}
			outPs.set(MPoint(val[0], val[1], val[2]) * imat, i);
		} else if (outType == 1) { // Normal
			if (envelope < 1.f) {
				calcFinalVal<MFloatVectorArray>(i, val, inNs, envelope);
			}
			outNs.set(MVector(val[0], val[1], val[2]) * imat, i);
		} else if (outType == 2) { //  Color
			if (envelope < 1.f) {
				calcFinalVal<MColorArray>(i, val, inCds, envelope);
			}
			outCds.set(i, 
				static_cast<float>(val[0]),
				static_cast<float>(val[1]),
				static_cast<float>(val[2]));
		} else { // uv
			if (envelope < 1.f) {
				val[0] = inUs[i] + (val[0] - inUs[i]) * envelope;
				val[1] = inVs[i] + (val[1] - inVs[i]) * envelope;
			}
			outUs.set(val[0], i);
			outVs.set(val[1], i);
		}
	}

	MIntArray vlist(inMeshFn.numVertices());
	if (outType == 0) { // Position
		outMeshFn.setPoints(outPs);
	} else if (outType == 1) { // Normal
		getVertexIndexs(vlist);
		outMeshFn.setVertexNormals(outNs, vlist, MSpace::kObject);
	} else if (outType == 2) { // Color
		getVertexIndexs(vlist);
		outMeshFn.setVertexColors(
			outCds,
			vlist,
			NULL,
			inMeshFn.getColorRepresentation(colorSetName)
		);
	} else { // uv
		outMeshFn.setUVs(outUs, outVs, &uvSetName);
	}

	return MS::kSuccess;
}

