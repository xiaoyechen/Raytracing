#pragma once
#include "Matrix.h"
#include "model.h"

class GenericObject
{
public:
	virtual ~GenericObject();
	virtual void setRayHit(Matrix<double> &start, Matrix<double> &direction) = 0;
	
	const unsigned getType();
	const hit_t getRayHit();
	double getAbmient(unsigned channel);
	virtual Matrix<double>* calculateSurfaceNormal(const Matrix<double> &intersection, unsigned hit_type)=0;
protected:
	GenericObject();
	unsigned type;
	Matrix<double>* M, *MInverse;
	color_t color, spec_color, diff_color, amb_color;
	double spec_coeff, diff_coeff, amb_coeff, fallout;
	hit_t rayOnObj;
	void setHitEnterAndExit(double hit1, double hit2);
	void resetHit();
};

class Sphere : GenericObject
{
public:
	Sphere();
	virtual ~Sphere();
	virtual void setRayHit(Matrix<double> &start, Matrix<double> &direction);
	virtual Matrix<double>* calculateSurfaceNormal(const Matrix<double> &intersection, unsigned hit_type);
};

class Plane : GenericObject
{
public:
	Plane();
	virtual ~Plane();
	virtual void setRayHit(Matrix<double> &start, Matrix<double> &direction);
	virtual Matrix<double>* calculateSurfaceNormal(const Matrix<double> &intersection, unsigned hit_type);
};
class Cylinder : GenericObject
{
public:
	Cylinder();
	virtual ~Cylinder();
	virtual void setRayHit(Matrix<double> &start, Matrix<double> &direction);
	virtual Matrix<double>* calculateSurfaceNormal(const Matrix<double> &intersection, unsigned hit_type);
};
class Cone : GenericObject
{
public:
	Cone();
	virtual ~Cone();
	virtual void setRayHit(Matrix<double> &start, Matrix<double> &direction);
	virtual Matrix<double>* calculateSurfaceNormal(const Matrix<double> &intersection, unsigned hit_type);
};
