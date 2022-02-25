#define _USE_MATH_DEFINES
#include <cmath>
#include "Light.h"

Light::Light()
{
	mPosition = new Matrix<double>(4, 1);
}

Light::Light(double posx, double posy, double posz, float cr, float cg, float cb, double intensity)
{
	mPosition = new Matrix<double>(4, 1);
	setPosition(posx, posy, posz);
	setColor(cr, cg, cb);
	setIntensity(intensity);
}

color_t Light::getColor() const
{
	return mColor;
}

Matrix<double>* Light::getPosition() const
{
	return mPosition;
}

double Light::getIntensity() const
{
	return mIntensity;
}

void Light::setPosition(double x, double y, double z)
{
	(*mPosition)(1, 1) = x;
	(*mPosition)(2, 1) = y;
	(*mPosition)(3, 1) = z;
	(*mPosition)(4, 1) = 0;
}

void Light::setIntensity(double it)
{
	mIntensity = it;
}


void Light::setColor(float r, float g, float b)
{
	mColor.r = r; mColor.g = g; mColor.b = b;
}

Light::~Light()
{
	mPosition->Erase(); delete mPosition;
}

PointLight::PointLight() : Light() {}
PointLight::PointLight(double posx, double posy, double posz, float cr, float cg, float cb, double intensity)
	:Light(posx, posy, posz, cr, cg, cb, intensity)
{
	setPosition(posx, posy, posz);
}
PointLight::~PointLight()
{}

void PointLight::setPosition(double x, double y, double z)
{
	Light::setPosition(x, y, z);
	(*mPosition)(4, 1) = 1;
}

double PointLight::calculateIntensity(Matrix<double> &intersection) const
{
	Matrix<double>* intersection_to_light = *mPosition - intersection;
	
	double dir_normal = intersection_to_light->normal();

	intersection_to_light->Erase(); delete intersection_to_light;

	return mIntensity / (4*M_PI*dir_normal);
}

Matrix<double>* PointLight::getDirOpposite() const
{
  return nullptr;
}

DirectedLight::DirectedLight() : Light() {}

DirectedLight::DirectedLight(double posx, double posy, double posz, float cr, float cg, float cb, double intensity)
	:Light(posx, posy, posz, cr, cg, cb, intensity)
{
	setPosition(posx, posy, posz);
}
DirectedLight::~DirectedLight()
{
  dir_neg->Erase(); delete dir_neg;
}

void DirectedLight::setPosition(double x, double y, double z)
{
	Light::setPosition(x, y, z);

	Matrix<double>* temp = mPosition->normalize();
	
	mPosition->Erase(); delete mPosition;
	mPosition = temp;
	dir_neg = -*mPosition;
}

double DirectedLight::calculateIntensity(Matrix<double>& intersection) const
{
	return Light::getIntensity();
}

Matrix<double>* DirectedLight::getDirOpposite() const
{
  return dir_neg;
}

