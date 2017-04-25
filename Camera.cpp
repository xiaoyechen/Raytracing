#include "Camera.h"



Camera::Camera()
{
	allocMemory();
}


Camera::~Camera()
{
	E->Erase();
	delete E;

	G->Erase();
	delete G;
}

void Camera::setEyeWorld(double x, double y, double z)
{
	(*E)(1, 1) = x;
	(*E)(2, 1) = y;
	(*E)(3, 1) = z;
	(*E)(4, 1) = 1.0;
}

void Camera::setGazeWorld(double x, double y, double z)
{
	(*G)(1, 1) = x;
	(*G)(2, 1) = y;
	(*G)(3, 1) = z;
	(*G)(4, 1) = 1.0;
}

void Camera::setUPWorld(double x, double y, double z)
{
}

void Camera::allocMemory()
{
	E = new Matrix<double>(4, 1);
	G = new Matrix<double>(4, 1);
	UP = new Matrix<double>(4, 1);
}