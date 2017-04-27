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
	UP->Erase();
	delete UP;
	Mp->Erase();
	delete Mp;
	S1->Erase();
	delete S1;
	T1->Erase();
	delete T1;
	S2->Erase();
	delete S2;
	T2->Erase();
	delete T2;
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
	(*UP)(1, 1) = x;
	(*UP)(2, 1) = y;
	(*UP)(3, 1) = z;
	(*UP)(4, 1) = 1.0;
}

void Camera::setMperspective(double near, double far)
{
	double a = -(near + far) / (far - near);
	double b = -2 * (near*far) / (far - near);

	Mp->identity();
	(*Mp)(1, 1) = near;
	(*Mp)(2, 2) = near;
	(*Mp)(3, 3) = a;
	(*Mp)(3, 4) = b;
	(*Mp)(4, 3) = -1;
	(*Mp)(4, 4) = 0;
}

void Camera::setTransformMatrices(double near, double, double angle, window_t &w)
{
	double aspect_ratio = w.width * 1.0 / w.height;

	double top = near * tan((M_PI / 180) * (angle / 2));
	double bottom = -top;
	double right = aspect_ratio * top;
	double left = -right;

	S1->identity();
	(*S1)(1, 1) = 2 / (right - left);
	(*S1)(2, 2) = 2 / (top - bottom);

	T1->identity();
	(*T1)(1, 4) = -(left + right) / 2;
	(*T1)(2, 4) = -(top + bottom) / 2;

	S2->identity();
	(*S2)(1, 1) = w.width / 2.0;
	(*S2)(2, 2) = w.height / 2.0;

	T2->identity();
	(*T2)(1, 4) = 1;
	(*T2)(2, 4) = 1;

	W->identity();
	(*W)(2, 2) = -1;
	(*W)(2, 4) = (double)(w.height);
}

void Camera::buildCamera()
{
	n = E->subtract(*G)->normalize();
	u = UP->multiplyCross(*n)->normalize();
	v = n->multiplyCross(*u)->normalize();

	(*Mv)(1, 1) = (*u)(1, 1);
	(*Mv)(1, 2) = (*u)(2, 1);
	(*Mv)(1, 3) = (*u)(3, 1);
	(*Mv)(1, 4) = -(E->multiplyDot(*u));

	(*Mv)(2, 1) = (*v)(1, 1);
	(*Mv)(2, 2) = (*v)(2, 1);
	(*Mv)(2, 3) = (*v)(3, 1);
	(*Mv)(2, 4) = -(E->multiplyDot(*v));

	(*Mv)(3, 1) = (*n)(1, 1);
	(*Mv)(3, 2) = (*n)(2, 1);
	(*Mv)(3, 3) = (*n)(3, 1);
	(*Mv)(3, 4) = -(E->multiplyDot(*n));

	(*Mv)(4, 1) = 0;
	(*Mv)(4, 2) = 0;
	(*Mv)(4, 3) = 0;
	(*Mv)(4, 4) = 1;

	M = Mp->multiply(*Mv)->multiply(*T1)->multiply(*S1)->multiply(*T2)->multiply(*S2);
}

void Camera::allocMemory()
{
	E = new Matrix<double>(4, 1);
	G = new Matrix<double>(4, 1);
	UP = new Matrix<double>(4, 1);
	Mv = new Matrix<double>(4, 4);
	S1 = new Matrix<double>(4, 4);
	T1 = new Matrix<double>(4, 4);
	S2 = new Matrix<double>(4, 4);
	T2 = new Matrix<double>(4, 4);
	W = new Matrix<double>(4, 4);
	Mv = new Matrix<double>(4, 4);
}