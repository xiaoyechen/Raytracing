#pragma once
#include "Matrix.h"
#include "Model.h"

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
	void setColor(float r, float g, float b);
	void setColorCoeff(unsigned color_type, double coeff);
	void setFallout(double val);
	void setReflect(double val);
	void setRefract(double val);
	void setTransparency(double val);
	void setAffineTransMat(Matrix<double> &mat);
	virtual void setRayHit(Matrix<double> &start, Matrix<double> &direction, hit_t& rayOnObj) = 0;
	
	unsigned getType() const;
	double getFallout() const;
	double getReflect() const;
	double getRefract() const;
	double getTransparency() const;
	const color_t& getColor() const;
	double getCoeff(unsigned light_type) const;
  Matrix<double>* getMInverse() const;
	virtual Matrix<double>* calculateSurfaceNormal(const Matrix<double> &intersection, unsigned hit_type)=0;
protected:
	GenericObject();
	unsigned type;
	Matrix<double>* M, *M_i, *M_it;
	color_t color;
	double spec_coeff, diff_coeff, amb_coeff, fallout, reflect, refract, transparency;
	
	void setHitEnterAndExit(double hit1, double hit2, hit_t& rayOnObj);
	void resetHit(hit_t& rayOnObj);
	double calculateRealT(double t, Matrix<double> &e, Matrix<double> &e_s, Matrix<double> &d, Matrix<double> &d_s);
};

class Sphere : public GenericObject
{
public:
	Sphere();
	virtual ~Sphere();
	virtual void setRayHit(Matrix<double> &start, Matrix<double> &direction, hit_t& rayOnObj);
	virtual Matrix<double>* calculateSurfaceNormal(const Matrix<double> &intersection, unsigned hit_type);
};

class Plane : public GenericObject
{
public:
	Plane();
	virtual ~Plane();
	virtual void setRayHit(Matrix<double> &start, Matrix<double> &direction, hit_t& rayOnObj);
	virtual Matrix<double>* calculateSurfaceNormal(const Matrix<double> &intersection, unsigned hit_type);
};
class Cylinder : public GenericObject
{
public:
	Cylinder();
	virtual ~Cylinder();
	virtual void setRayHit(Matrix<double> &start, Matrix<double> &direction, hit_t& rayOnObj);
	virtual Matrix<double>* calculateSurfaceNormal(const Matrix<double> &intersection, unsigned hit_type);
};
class Cone : public GenericObject
{
public:
	Cone();
	virtual ~Cone();
	virtual void setRayHit(Matrix<double> &start, Matrix<double> &direction, hit_t& rayOnObj);
	virtual Matrix<double>* calculateSurfaceNormal(const Matrix<double> &intersection, unsigned hit_type);
};
