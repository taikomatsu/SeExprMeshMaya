#ifndef _ClosestPointFunc_h_
#define _ClosestPointFunc_h_
#include <maya/MMeshIntersector.h>
#include <SeExprFunc.h>

class ClosestPointPositionFunc: public SeExprFuncX
{
public:
	ClosestPointPositionFunc(): SeExprFuncX(true) {}
	ClosestPointPositionFunc(MMeshIntersector* _mIntersect)
		:SeExprFuncX(true), mIntersect(_mIntersect){}

	virtual bool prep(SeExprFuncNode*, bool, std::string&);
	virtual void eval(const SeExprFuncNode*, SeVec3d&) const;
	static  void define(ClosestPointPositionFunc&);

public:
	static const char* funcName;
	static const char* doc_string;

private:
	MMeshIntersector* mIntersect;
};

class ClosestPointNormalFunc: public SeExprFuncX
{
public:
	ClosestPointNormalFunc(): SeExprFuncX(true) {}
	ClosestPointNormalFunc(MMeshIntersector* _mIntersect)
		:SeExprFuncX(true), mIntersect(_mIntersect){}

	virtual bool prep(SeExprFuncNode*, bool, std::string&);
	virtual void eval(const SeExprFuncNode*, SeVec3d&) const;
	static  void define(ClosestPointNormalFunc&);

public:
	static const char* funcName;
	static const char* doc_string;

private:
	MMeshIntersector* mIntersect;
};

#endif

