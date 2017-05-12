#define _USE_MATH_DEFINES
#include <cmath>
#include "Light.h"

Light::Light()
{
	position = new Matrix<double>(4, 1);
}

color_t Light::getColor() const
{
	return color;
}

Matrix<double>* Light::getPosition() const
{
	return position;
}

double Light::getIntensity() const
{
	return intensity;
}

void Light::setPosition(double x, double y, double z)
{
	(*position)(1, 1) = x;
	(*position)(2, 1) = y;
	(*position)(3, 1) = z;
	(*position)(4, 1) = 0;
}

void Light::setIntensity(double it)
{
	intensity = it;
}


void Light::setColor(double r, double g, double b)
{
	color.r = r; color.g = g; color.b = b;
}

Light::~Light()
{
	position->Erase(); delete position;
}

PointLight::PointLight()
{
}

PointLight::~PointLight()
{
}

void PointLight::setPosition(double x, double y, double z)
{
	Light::setPosition(x, y, z);
	(*position)(4, 1) = 1;
}

double PointLight::calculateIntensity(Matrix<double> &intersection) const
{
	Matrix<double>* intersection_to_light = *position - intersection;
	
	double dir_normal = intersection_to_light->normal();

	intersection_to_light->Erase(); delete intersection_to_light;

	return intensity / (4*M_PI*dir_normal);
}

Matrix<double>* PointLight::getDirOpposite() const
{
  return nullptr;
}

DirectedLight::DirectedLight()
{
}

DirectedLight::~DirectedLight()
{
  dir_neg->Erase(); delete dir_neg;
}

void DirectedLight::setPosition(double x, double y, double z)
{
	Light::setPosition(x, y, z);

	Matrix<double>* temp = position->normalize();
	
	position->Erase(); delete position;
	position = temp;
  dir_neg = -*position;
}

double DirectedLight::calculateIntensity(Matrix<double>& intersection) const
{
	return Light::getIntensity();
}

Matrix<double>* DirectedLight::getDirOpposite() const
{
  return dir_neg;
}

