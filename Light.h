#pragma once
#include "model.h"
#include "Matrix.h"
class Light
{
public:
	Light();
	color_t getColor() const;
	Matrix<double>* getPosition() const;
  virtual Matrix<double>* getDirOpposite() const = 0;
	double getIntensity() const;
	virtual void setPosition(double x, double y, double z);
	void setIntensity(double it);
	void setColor(double r, double g, double b);
	virtual ~Light();
	virtual double calculateIntensity(Matrix<double> &intersection) const = 0;

protected:
	Matrix<double>* position;
	color_t color;
	double intensity;
};

class PointLight: public Light
{
public:
	PointLight();
	virtual ~PointLight();
	virtual void setPosition(double x, double y, double z);
	virtual double calculateIntensity(Matrix<double> &intersection) const;
  virtual Matrix<double>* getDirOpposite() const;
};

class DirectedLight : public Light
{
public:
	DirectedLight();
	virtual ~DirectedLight();
	virtual void setPosition(double x, double y, double z);
	virtual double calculateIntensity(Matrix<double> &intersection) const;
  virtual Matrix<double>* getDirOpposite() const;
protected:
  Matrix<double>* dir_neg;
};