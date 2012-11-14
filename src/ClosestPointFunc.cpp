#include <SeExprFunc.h>
#include <SeExprNode.h>
#include <maya/MObject.h>
#include <maya/MMeshIntersector.h>
#include "ClosestPointFunc.h"


void getMPointOnMesh(
	const MMeshIntersector* mIntersect,
	const SeExprFuncNode* node,
	MPointOnMesh& mp)
{
	SeVec3d p;
	node->child(0)->eval(p);
	mIntersect->getClosestPoint(MPoint(p[0], p[1], p[2]), mp);
}

const char* ClosestPointPositionFunc::funcName = "closestPoint";
const char* ClosestPointPositionFunc::doc_string =
	"vector closestPoint(vector p)\nGet closest point position on ctrlMesh.";

bool ClosestPointPositionFunc::prep(SeExprFuncNode* node, bool wantVec, std::string& error)
{
	return SeExprFuncX::prep(node, true);
}

void ClosestPointPositionFunc::eval(const SeExprFuncNode* node, SeVec3d& result) const
{
	MPointOnMesh mp;
	getMPointOnMesh(mIntersect, node, mp);
	MFloatPoint cp = mp.getPoint();
	result.setValue(cp[0], cp[1], cp[2]);
}

void ClosestPointPositionFunc::define(ClosestPointPositionFunc& cpfunc)
{
	SeExprFunc::define(ClosestPointPositionFunc::funcName, SeExprFunc(cpfunc, 1, 1));
}

const char* ClosestPointNormalFunc::funcName = "closestNormal";
const char* ClosestPointNormalFunc::doc_string =
	"vector closestNormal(vector p)\nGet closest point normal on ctrlMesh.";

bool ClosestPointNormalFunc::prep(SeExprFuncNode* node, bool wantVec, std::string& error)
{
	return SeExprFuncX::prep(node, true);
}

void ClosestPointNormalFunc::eval(const SeExprFuncNode* node, SeVec3d& result) const
{
	MPointOnMesh mp;
	getMPointOnMesh(mIntersect, node, mp);
	MFloatPoint cp = mp.getNormal();
	result.setValue(cp[0], cp[1], cp[2]);
}

void ClosestPointNormalFunc::define(ClosestPointNormalFunc& cpfunc)
{
	SeExprFunc::define(ClosestPointNormalFunc::funcName, SeExprFunc(cpfunc, 1, 1));
}

