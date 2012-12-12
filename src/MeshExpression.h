#ifndef _MeshExpression_h_
#define _MeshExpression_h_

#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MFloatPoint.h>
#include <maya/MFloatVector.h>
#include <maya/MColor.h>
#include <maya/MFnMesh.h>
#include <maya/MObject.h>
#include <maya/MMeshIntersector.h>
#include <SeExpression.h>
#include <SeExprFunc.h>
#include <SeExprBuiltins.h>
#include <SeExprNode.h>
#include <cstdlib>
#include <cstdio>
#include <map>
#include <iostream>
#include <string>

struct SimpleScalarVar: public SeExprScalarVarRef
{
	double val;
	SimpleScalarVar() {}
	SimpleScalarVar(double v) :val(v) {}

	void eval(const SeExprVarNode*, SeVec3d& result)
	{
		result[0] = val;
	}
};

struct SimpleVectorVar: public SeExprVectorVarRef
{
	double x, y, z;
	SimpleVectorVar() {}
	SimpleVectorVar(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {}

	void eval(const SeExprVarNode*, SeVec3d& result)
	{
		result[0] = x;
		result[1] = y;
		result[2] = z;
	}

};

typedef std::map<std::string, SimpleScalarVar> DynScalarAttrVar;
typedef std::map<std::string, SimpleVectorVar> DynVectorAttrVar;
typedef std::map<std::string, double> DynScalarAttrValue;
typedef std::map<std::string, MVector> DynVectorAttrValue;

class MeshExpression: public SeExpression
{
public:
	MeshExpression(): SeExpression() {}
	MeshExpression(const std::string& expr): SeExpression(expr) {}

	void setDynamicScalarAttrs(DynScalarAttrValue&);
	void setDynamicVectorAttrs(DynVectorAttrValue&);
	void setP(const MPoint&);
	void setN(const MFloatVector&);
	void setCd(const MColor &);
	void setUV(const float, const float);
	void setFrame(const int);
	void setTime(const double);

private:
	virtual SeExprVarRef* resolveVar(const std::string&) const;

private:
	mutable SimpleScalarVar u, v, frame, time;
	mutable SimpleVectorVar P, N, Cd;
	mutable DynScalarAttrVar dynScalarAttrs;
	mutable DynVectorAttrVar dynVectorAttrs;

};


inline void MeshExpression::setDynamicScalarAttrs(DynScalarAttrValue &attrs)
{
	DynScalarAttrValue::iterator iter = attrs.begin();
	std::pair<DynScalarAttrVar::iterator, bool> pr;
	while(iter != attrs.end()) {
		pr = dynScalarAttrs.insert(
			DynScalarAttrVar::value_type(iter->first, SimpleScalarVar(iter->second)));
		if (pr.second == false) // already exists
			dynScalarAttrs[iter->first] = SimpleScalarVar(iter->second);
		++iter;
	}
}

inline void MeshExpression::setDynamicVectorAttrs(DynVectorAttrValue &attrs)
{
	DynVectorAttrValue::iterator iter = attrs.begin();
	std::pair<DynVectorAttrVar::iterator, bool> pr;
	while(iter != attrs.end()) {
		double x = iter->second[0];
		double y = iter->second[1];
		double z = iter->second[2];
		pr = dynVectorAttrs.insert(
			DynVectorAttrVar::value_type(iter->first, SimpleVectorVar(x, y, z)));
		if (pr.second == false) // already exists
			dynVectorAttrs[iter->first] = SimpleVectorVar(x, y, z);
		++iter;
	}
}

inline void MeshExpression::setP(const MPoint &v)
{
	P.x = v[0];
	P.y = v[1];
	P.z = v[2];
}

inline void MeshExpression::setN(const MFloatVector &v)
{
	N.x = v[0];
	N.y = v[1];
	N.z = v[2];
}

inline void MeshExpression::setCd(const MColor &v)
{
	Cd.x = v[0];
	Cd.y = v[1];
	Cd.z = v[2];
}

inline void MeshExpression::setUV(const float _u, const float _v)
{
	u.val = _u;
	v.val = _v;
}

inline void MeshExpression::setFrame(const int _frame)
{
	frame.val = static_cast<double>(_frame);
}

inline void MeshExpression::setTime(const double _time)
{
	time.val = _time;
}

inline SeExprVarRef* MeshExpression::resolveVar(const std::string& name) const
{
	if (name == "P")			return &P;
	else if (name == "N")		return &N;
	else if (name == "Cd")		return &Cd;
	else if (name == "u")		return &u;
	else if (name == "v")		return &v;
	else if (name == "frame")	return &frame;
	else if (name == "time")	return &time;
	else {
		// return scalar
		DynScalarAttrVar::iterator iterS = dynScalarAttrs.find(name);
		if (iterS != dynScalarAttrs.end())
			return &(iterS->second);
		// return vector
		DynVectorAttrVar::iterator iterV = dynVectorAttrs.find(name);
		return (iterV != dynVectorAttrs.end()) ? &(iterV->second) : 0;
	}
}

#endif
