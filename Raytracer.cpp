#include <iostream>
#include "Camera.h"
#include "GenericObject.h"

unsigned finObjIdx(GenericObject *objects[], unsigned num, unsigned target_type)
{
	unsigned i = 0;
	while (i < num && objects[i]->getType() != target_type)
		++i;

	return i;
}

unsigned findMinHitIdx(hit_t hits[], unsigned num)
{
	if (num == 1) return 0;

	unsigned min_idx = 0;
	for (unsigned i = 1; i < num; ++i)
	{
		if (hits[i].enter < hits[min_idx].enter)
			min_idx = i;
	}

	return min_idx;
}

Matrix<double>* projTrans(Matrix<double> &P)
{
	Matrix<double>* Q = new Matrix<double>(4,1);

	if (P.getLength() == 4 && P.getHeight() == 1)
	{
		(*Q)(1, 1) = P(1, 1) / P(4, 1);
		(*Q)(2, 1) = P(2, 1) / P(4, 1);
		(*Q)(3, 1) = P(3, 1) / P(4, 1);
		(*Q)(4, 1) = 1;
	}

	return Q;
}