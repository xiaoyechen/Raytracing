#pragma once
#include "Matrix.h"
#include "Model.h"
class Camera
{
public:
	Camera();
	~Camera();

	// getters
	Matrix<double>* getU() const;
	Matrix<double>* getV() const;
	Matrix<double>* getN() const;
	Matrix<double>* getE() const;
	Matrix<double>* getM() const;
	// setters
	void setEyeWorld(double x, double y, double z);
	void setGazeWorld(double x, double y, double z);
	void setUPWorld(double x, double y, double z);
	void setMperspective(double near, double far);
	void setTransformMatrices(double near, double angle, window_t w);
	void setRotationAngle(double angle);
	void buildCamera();
	void moveCamera(unsigned dir);

private:
	Matrix<double>* UP, *E, *G,
		*u, *v, *n,
		*Mv, /* transformation matrix of a point in viewing coord to world coord */
		*Mp, /* perspective transformation matrix in homogeneous coord system */
		*S1, *T1, /* scaling and translation matrix for coord other than pseudo depth*/
		*T2, /* translation matrix that move (x,y) to positive quadrant */
		*S2, /* scaling matrix so coord fit in display window */
		*W, /* translate coord system so (0,0) is at top left corner of window */
		*M;
	double rotate_angle;
	Matrix<double>* calculateRotationalMatrix(const Matrix<double> &axis, double rangle);
};

