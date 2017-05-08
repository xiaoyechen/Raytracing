#pragma once
#include "Matrix.h"
#include "model.h"

#define COLOR_ORIGIN 0
#define COLOR_SPEC 1
#define COLOR_DIFFUSE 2
#define COLOR_AMBIENT 3

class GenericObject
{
public:
	static GenericObject* makeObject(unsigned id);
	virtual ~GenericObject();
	void setType(unsigned t);
	void setColor(double r, double g, double b);
	void setColorCoeff(unsigned color_type, double coeff);
	void setFallout(double val);
	void setReflect(double val);
	void setAffineTransMat(Matrix<double> &mat);
	virtual void setRayHit(Matrix<double> &start, Matrix<double> &direction) = 0;
	
	const unsigned getType();
	const double getFallout();
	const double getReflect();
	const hit_t getRayHit();
	color_t getColor();
	double getCoeff(unsigned light_type);
	virtual Matrix<double>* calculateSurfaceNormal(const Matrix<double> &intersection, unsigned hit_type)=0;
protected:
	GenericObject();
	unsigned type;
	Matrix<double>* M, *MInverse;
	color_t color;
	double spec_coeff, diff_coeff, amb_coeff, fallout, reflect;
	hit_t rayOnObj;

	void setHitEnterAndExit(double hit1, double hit2);
	void resetHit();
};

class Sphere : public GenericObject
{
public:
	Sphere();
	virtual ~Sphere();
	virtual void setRayHit(Matrix<double> &start, Matrix<double> &direction);
	virtual Matrix<double>* calculateSurfaceNormal(const Matrix<double> &intersection, unsigned hit_type);
};

class Plane : public GenericObject
{
public:
	Plane();
	virtual ~Plane();
	virtual void setRayHit(Matrix<double> &start, Matrix<double> &direction);
	virtual Matrix<double>* calculateSurfaceNormal(const Matrix<double> &intersection, unsigned hit_type);
};
class Cylinder : public GenericObject
{
public:
	Cylinder();
	virtual ~Cylinder();
	virtual void setRayHit(Matrix<double> &start, Matrix<double> &direction);
	virtual Matrix<double>* calculateSurfaceNormal(const Matrix<double> &intersection, unsigned hit_type);
};
class Cone : public GenericObject
{
public:
	Cone();
	virtual ~Cone();
	virtual void setRayHit(Matrix<double> &start, Matrix<double> &direction);
	virtual Matrix<double>* calculateSurfaceNormal(const Matrix<double> &intersection, unsigned hit_type);
};
