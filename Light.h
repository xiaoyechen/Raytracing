#pragma once
#include "Model.h"
#include "Matrix.h"
class Light
{
public:
	Light();
	Light(double posx, double posy, double posz, float cr, float cg, float cb, double intensity);
	virtual ~Light();

	color_t getColor() const;
	Matrix<double>* getPosition() const;
  	virtual Matrix<double>* getDirOpposite() const = 0;
	double getIntensity() const;
	virtual void setPosition(double x, double y, double z);
	void setIntensity(double it);
	void setColor(float r, float g, float b);
	virtual double calculateIntensity(Matrix<double> &intersection) const = 0;

protected:
	Matrix<double>* mPosition;
	color_t mColor;
	double mIntensity;
};

class PointLight: public Light
{
public:
	PointLight();
	PointLight(double posx, double posy, double posz, float cr, float cg, float cb, double intensity);
	virtual ~PointLight();
	virtual void setPosition(double x, double y, double z) override;
	virtual double calculateIntensity(Matrix<double> &intersection) const;
  	virtual Matrix<double>* getDirOpposite() const;
};

class DirectedLight : public Light
{
public:
	DirectedLight();
	DirectedLight(double posx, double posy, double posz, float cr, float cg, float cb, double intensity);
	virtual ~DirectedLight();
	virtual void setPosition(double x, double y, double z) override;
	virtual double calculateIntensity(Matrix<double> &intersection) const;
  	virtual Matrix<double>* getDirOpposite() const;
protected:
  Matrix<double>* dir_neg;
};