#ifndef FREEDOM_DRAGONICA_SCENE_WORLD_PGNIFOPTIMIZER_H
#define FREEDOM_DRAGONICA_SCENE_WORLD_PGNIFOPTIMIZER_H

#include <nimain.h>


class	PgNifOptimizer
{

public:

	static	NiAVObject*	CreateOptimizedObject(NiAVObject *pkOriginal);

private:

	static	int	AnalyseVaildTriCount(NiTriStrips *pkTriStrips);


	static	NiTriShape*	CreateOptimizedTriShape(NiTriStrips *pkTriStrips,int iValidTriCount);
};

#endif // FREEDOM_DRAGONICA_SCENE_WORLD_PGNIFOPTIMIZER_H