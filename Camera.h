#pragma once
#include "Matrix.h"
class Camera
{
public:
	Camera();
	~Camera();

	// getters

	// setters
	void setEyeWorld(double x, double y, double z);
	void setGazeWorld(double x, double y, double z);
	void setUPWorld(double x, double y, double z);
	void setNearFarPlaneDist(double near, double far);
	void setViewAngle(double angle);
	
private:
	Matrix<double>* UP, *E, *G,
		*u, *v, *n,
		*Mv, *Mp,
		*T1, *T2, *S1, *S2, *W,
		*M;

	void allocMemory();
};

