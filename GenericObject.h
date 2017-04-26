#pragma once
#include "Matrix.h"
#include "model.h"

class GenericObject
{
public:
	GenericObject();
	~GenericObject();
private:
	unsigned type;
	Matrix<double> M, MInverse;
	color_t color, spec_color, diff_color, amb_color;
	double spec_coeff, diff_coeff, amb_coeff, fallout;
};

