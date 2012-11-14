#ifndef _seExprMeshCmd_h_
#define _seExprMeshCmd_h_

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MArgList.h>
#include <maya/MDGModifier.h>

class SeExprMeshCmd : public MPxCommand
{
public:
	virtual MStatus	doIt(const MArgList&);
	virtual MStatus	undoIt();
	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; };
	static void* creator();
	static MSyntax newSyntax();

private:
	MDGModifier dgMod;
};

#endif

