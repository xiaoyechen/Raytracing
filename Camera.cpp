#define _USE_MATH_DEFINES
#include <cmath>
#include "Camera.h"

Camera::Camera()
{
	allocMemory();
}


Camera::~Camera()
{
	E->Erase(); delete E;
	G->Erase(); delete G;
	UP->Erase(); delete UP;
	Mp->Erase(); delete Mp;
	S1->Erase(); delete S1;
	T1->Erase(); delete T1;
	S2->Erase(); delete S2;
	T2->Erase(); delete T2;
	W->Erase(); delete W;
	Mv->Erase(); delete Mv;
	M->Erase(); delete M;
	u->Erase(); delete u;
	v->Erase(); delete v;
	n->Erase(); delete n;
	
	rmat_left->Erase(); delete rmat_left;
	rmat_right->Erase(); delete rmat_right;
	rmat_up->Erase(); delete rmat_up;
	rmat_down->Erase(); delete rmat_down;
}

Matrix<double>* Camera::getU()
{
	return u;
}

Matrix<double>* Camera::getV()
{
	return v;
}

Matrix<double>* Camera::getN()
{
	return n;
}

Matrix<double>* Camera::getE()
{
	return E;
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
	UP->setHeight(3);
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

void Camera::setTransformMatrices(double near, double angle, window_t w)
{
	double aspect_ratio = w.width*1.0 / w.height;

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

void Camera::setRotationAngle(double angle)
{
	rotate_angle = angle;

	// calculate all rotational matrices
	rmat_left = calculateRotationalMatrix(*UP, -rotate_angle);
	rmat_right = calculateRotationalMatrix(*UP, rotate_angle);
	rmat_up = calculateRotationalMatrix(*u, -rotate_angle);
	rmat_down = calculateRotationalMatrix(*u, rotate_angle);
}

void Camera::buildCamera()
{
	n = E->subtract(*G);
	Matrix<double>* temp = n->normalize();
	n->Erase(); delete n;
	n = temp; 
	
	n->setHeight(3);

	u = UP->multiplyCross(*n);
	temp = u->normalize();
	u->Erase(); delete u;
	u = temp;

	v = n->multiplyCross(*u);
	temp = v->normalize();
	v->Erase(); delete v;
	v = temp;

	E->setHeight(3);

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

	E->setHeight(4);

	(*Mv)(4, 1) = 0;
	(*Mv)(4, 2) = 0;
	(*Mv)(4, 3) = 0;
	(*Mv)(4, 4) = 1;

	M = Mp->multiply(*Mv);
	temp = T1->multiply(*M);
	M->Erase(); delete M;
	M = temp;

	temp = S1->multiply(*M);
	M->Erase(); delete M;
	M = temp;

	temp = T2->multiply(*M);
	M->Erase(); delete M;
	M = temp;

	temp = S2->multiply(*M);
	M->Erase(); delete M;
	M = temp;

	temp = W->multiply(*M);
	M->Erase(); delete M;
	M = temp;
}

void Camera::moveCamera(unsigned dir)
{
	Matrix<double>* temp = new Matrix<double>(*E);
	temp->setHeight(3);

  Matrix<double>* newPos;
  
	switch (dir)
	{
	case CAM_L:
		newPos = rmat_left->multiply(*temp);
		break;
	case CAM_R:
		newPos = rmat_right->multiply(*temp);
		break;
	case CAM_U:
		newPos = rmat_up->multiply(*temp);
		break;
	case CAM_D:
		newPos = rmat_down->multiply(*temp);
		break;
	case CAM_N:
		newPos = temp->subtract(*n);
		break;
	case CAM_F:
		newPos = temp->add(*n);
		break;
	default:
		newPos = temp;
	}

	setEyeWorld((*newPos)(X, 1), (*newPos)(Y, 1), (*newPos)(Z, 1));

	temp->Erase(); delete temp;
	newPos->Erase(); delete newPos;
}

void Camera::allocMemory()
{
	E = new Matrix<double>(4, 1);
	G = new Matrix<double>(4, 1);
	UP = new Matrix<double>(4, 1);
	Mp = new Matrix<double>(4, 4);
	S1 = new Matrix<double>(4, 4);
	T1 = new Matrix<double>(4, 4);
	S2 = new Matrix<double>(4, 4);
	T2 = new Matrix<double>(4, 4);
	W = new Matrix<double>(4, 4);
	Mv = new Matrix<double>(4, 4);
}

Matrix<double>* Camera::calculateRotationalMatrix(const Matrix<double> &axis, double rangle)
{
	Matrix<double> Jv(3, 3);
	Jv(X, 1) = 0;
	Jv(X, 2) = -axis(Z, 1);
	Jv(X, 3) = axis(Y, 1);

	Jv(Y, 1) = axis(Z, 1);
	Jv(Y, 2) = 0;
	Jv(Y, 3) = -axis(X, 1);

	Jv(Z, 1) = -axis(Y, 1);
	Jv(Z, 2) = axis(X, 1);
	Jv(Z, 3) = 0;

	Matrix<double> identity(3, 3);
	identity.identity();

	Matrix<double>* temp = Jv.multiplyDot(sin(rangle));
	Matrix<double>* rmat = identity.add(*temp);
	temp->Erase(); delete temp;
	Matrix<double>* JvSq = Jv.multiply(Jv);
	temp = JvSq->multiplyDot(1 - cos(rangle));
	Matrix<double>* result = rmat->add(*JvSq);
	rmat->Erase(); delete rmat;
	rmat = result;

	JvSq->Erase(); delete JvSq;
	temp->Erase(); delete temp;

	return rmat;
}
