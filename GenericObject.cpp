#define _USE_MATH_DEFINES
#include <cmath>
#include "GenericObject.h"

GenericObject::GenericObject()
{
	M = new Matrix<double>(4, 4);

	// default coefficients
	reflect = 0;
	refract = 1;
	transparency = 0;
}

void GenericObject::setHitEnterAndExit(double hit1, double hit2)
{
  if (hit1 < -BIAS && hit2< -BIAS) return;
  
	//if (hit1 < -BIAS)
	//	rayOnObj.enter = hit2;
	//else if (hit2 < -BIAS)
	//	rayOnObj.enter = hit1;
	else if (hit1 <= hit2)
	{
		rayOnObj.enter = hit1;
		rayOnObj.exit = hit2;
	}
	else {
		rayOnObj.enter = hit2;
		rayOnObj.exit = hit1;
	}
}

void GenericObject::resetHit()
{
	rayOnObj.enter = INFINITY;
	rayOnObj.exit = INFINITY;
	rayOnObj.enter_type = type;
	rayOnObj.exit_type = type;
}

double GenericObject::calculateRealT(double t, Matrix<double>& e, Matrix<double>& e_s, Matrix<double>& d, Matrix<double>& d_s)
{
	if (M->isIden()) return t;

	double real_t = t;

	Matrix<double>* dt = d_s.multiplyDot(t);
	Matrix<double>* hitpoint = e_s + *dt;

	// transform the hit point to world space
	Matrix<double>* hitpoint_real = *M * *hitpoint;

	// compute the actual t
	Matrix<double>* dt_real = *hitpoint_real - e;
	real_t = (*dt_real)(Z, 1) / d(Z, 1);

	dt->Erase(); delete dt;
	hitpoint->Erase(); delete hitpoint;
	hitpoint_real->Erase(); delete hitpoint_real;
	dt_real->Erase(); delete dt_real;

	return real_t;
}


GenericObject * GenericObject::makeObject(unsigned id)
{
	switch (id)
	{
	case OBJ_PLANE:
		return new Plane();
	case OBJ_SPHERE:
		return new Sphere();
	case OBJ_CYLINDER:
		return new Cylinder();
	case OBJ_CONE:
		return new Cone();
	default:
		return NULL;
	}
}

GenericObject::~GenericObject()
{
	M->Erase(); delete M;
	M_i->Erase(); delete M_i;
	M_it->Erase(); delete M_it;
}

void GenericObject::setType(unsigned t)
{
	type = t;
}

void GenericObject::setColor(double r, double g, double b)
{
	color.r = r; color.g = g; color.b = b;
}

void GenericObject::setColorCoeff(unsigned color_type, double coeff)
{
	switch (color_type)
	{
	case COLOR_DIFFUSE:
		diff_coeff = coeff; break;
	case COLOR_SPEC:
		spec_coeff = coeff; break;
	case COLOR_AMBIENT:
		amb_coeff = coeff; break;
	}
}

void GenericObject::setFallout(double val)
{
	fallout = val;
}

void GenericObject::setReflect(double val)
{
	reflect = val;
}

void GenericObject::setRefract(double val)
{
	refract = val;
}

void GenericObject::setTransparency(double val)
{
	transparency = val;
}

void GenericObject::setAffineTransMat(Matrix<double>& mat)
{
	M->copy(mat);
	M_i = M->inverse();
	M_it = M_i->transpose();
}


unsigned GenericObject::getType() const
{ return type; }

double GenericObject::getFallout() const
{
	return fallout;
}

double GenericObject::getReflect() const
{
	return reflect;
}

double GenericObject::getRefract() const
{
	return refract;
}

double GenericObject::getTransparency() const
{
	return transparency;
}

hit_t GenericObject::getRayHit() const
{
	return rayOnObj;
}

color_t GenericObject::getColor() const
{
	return color;
}

double GenericObject::getCoeff(unsigned light_type) const
{
	switch (light_type)
	{
	case COLOR_AMBIENT:
		return amb_coeff;
	case COLOR_DIFFUSE:
		return diff_coeff;
	case COLOR_SPEC:
		return spec_coeff;
	default:
		return 0;
	}
}

Matrix<double>* GenericObject::getMInverse() const
{
  return M_i;
}

Cylinder::Cylinder()
{
	type = OBJ_CYLINDER;
}

Cylinder::~Cylinder()
{
}

void Cylinder::setRayHit(Matrix<double>& start, Matrix<double>& direction)
{
	// 3 possible intersection: cylinder wall, top cap, and bottom cap
	// wall&top, wall&wall, wall&bottom, top&bottom
	resetHit();

	// calculate transformed ray
	Matrix<double>* temp = *M_i * direction;
	Matrix<double>* direction_s = temp->normalize();
	temp->Erase(); delete temp;

	Matrix<double>* start_s = *M_i * start;

	//-----------for cylinder walls

	//calculate discriminant of ray-cylinderwall intersection equation
	double a = (*direction_s)(X, 1)*(*direction_s)(X, 1) + (*direction_s)(Y, 1)*(*direction_s)(Y, 1);
	double b = (*start_s)(X, 1) * (*direction_s)(X, 1) + (*start_s)(Y, 1) * (*direction_s)(Y, 1);
	double c = (*start_s)(X, 1)*(*start_s)(X, 1) + (*start_s)(Y, 1)*(*start_s)(Y, 1) - 1;

	double isRayHit = b*b - a*c;

	// if ray does hit the object, check enter & exit types and distances
	if (isRayHit >= 0)
	{
		double t1 = -b / a + sqrt(isRayHit) / a,
			t2 = -b / a - sqrt(isRayHit) / a;
			
		if (t1>=-BIAS && t2>=-BIAS)
		{
			double z1 = (*start_s)(Z, 1) + (*direction_s)(Z, 1) * t1,
			     z2 = (*start_s)(Z, 1) + (*direction_s)(Z, 1) * t2;

		  t1 = calculateRealT(t1, start, *start_s, direction, *direction_s);
			t2 = calculateRealT(t2, start, *start_s, direction, *direction_s);

		  // if both z1 and z2 are between -1 and 1 (inclusive),
		  // then the ray enter and exit at cylinder walls
		  if (z1 >= -1 && z1 <= 1 && z2 >= -1 && z2 <= 1)
		  {
			  setHitEnterAndExit(t1, t2);

			  start_s->Erase(); delete start_s;
			  direction_s->Erase(); delete direction_s;
			  return;
		  }
		  // if only one of z1 z2 is between -1 and 1,
		  // then the ray only enter/exit at cylinder wall
		  else if (z1 >= -1 && z1 <= 1)
		  {
			  rayOnObj.enter = t1;
		  }
		  else if (z2 >= -1 && z2 <= 1)
		  {
			  rayOnObj.enter = t2;
		  }
		}
	}
	
	//-----------for cylinder caps

	if ((*direction_s)(Z, 1) != 0)
	{
		double t_cap = (((*direction_s)(Z, 1)<0?1:-1) - (*start_s)(Z, 1)) / (*direction_s)(Z, 1);
		
		if (t_cap>=0)
		{
		  double ray_x = (*start_s)(X, 1) + t_cap*(*direction_s)(X, 1);
		  double ray_y = (*start_s)(Y, 1) + t_cap*(*direction_s)(Y, 1);
		  if (sqrt(ray_x*ray_x + ray_y*ray_y) <= 1)
		  {
			  t_cap = calculateRealT(t_cap, start, *start_s, direction, *direction_s);
			  
			  // check if the ray intersects cylinder cap when entering
			  // or exiting the cylinder object
			  if (t_cap < rayOnObj.enter)
			  {
				  rayOnObj.exit_type = rayOnObj.enter_type;
				  rayOnObj.exit = rayOnObj.enter;
				  rayOnObj.enter_type = (*direction_s)(Z, 1)<0?CYLINDER_TOP:CYLINDER_BASE;
				  rayOnObj.enter = t_cap;
			  }
			  else 
			  {
				  rayOnObj.exit_type = (*direction_s)(Z, 1)<0 ? CYLINDER_TOP:CYLINDER_BASE;
				  rayOnObj.exit = t_cap;
			  }
		  }
		}
	}

	start_s->Erase(); delete start_s;
	direction_s->Erase(); delete direction_s;
}

Matrix<double>* Cylinder::calculateSurfaceNormal(const Matrix<double>& intersection, unsigned hit_type)
{
	Matrix<double>* intersection_s = *M_i * intersection;

	Matrix<double>* temp = new Matrix<double>(4, 1, 0);

	if (hit_type == CYLINDER_BASE)
		(*temp)(Z, 1) = -1;
	else if (hit_type == CYLINDER_TOP)
		(*temp)(Z, 1) = 1;
	else
	{
		double denom = sqrt((*intersection_s)(X, 1)*(*intersection_s)(X, 1) + (*intersection_s)(Y, 1)*(*intersection_s)(Y, 1));
		(*temp)(X, 1) = (*intersection_s)(X, 1) / denom;
		(*temp)(Y, 1) = (*intersection_s)(Y, 1) / denom;
	}

	Matrix<double>* surf_normal = *M_it * *temp;
	temp->Erase(); delete temp;
  (*surf_normal)(4, 1) = 0;

	temp = surf_normal->normalize();
	
	surf_normal->Erase(); delete surf_normal;
	intersection_s->Erase(); delete intersection_s;

	return temp;
}

Plane::Plane()
{
	type = OBJ_PLANE;
}

Plane::~Plane()
{
}

void Plane::setRayHit(Matrix<double>& start, Matrix<double>& direction)
{
	resetHit();
	Matrix<double>* temp = *M_i * direction;
	Matrix<double>* direction_s = temp->normalize();
	
	Matrix<double>* start_s = *M_i * start;

	if ((*direction_s)(Z, 1) < -BIAS)
	{
		double t = -(*start_s)(Z, 1) / (*direction_s)(Z, 1);
		if (t >= 0)
		{
		  //rayOnObj.enter = t;
		  rayOnObj.enter = calculateRealT(t, start, *start_s, direction, *direction_s);
		  rayOnObj.exit = rayOnObj.enter;
		}
	}
	
	temp->Erase(); delete temp;
	start_s->Erase(); delete start_s;
	direction_s->Erase(); delete direction_s;
}

Matrix<double>* Plane::calculateSurfaceNormal(const Matrix<double>& intersection, unsigned hit_type)
{
	Matrix<double>* surf_normal = new Matrix<double>(4, 1, 0);

	(*surf_normal)(Z, 1) = 1;

	Matrix<double>* temp = *M_it * *surf_normal;
	surf_normal->Erase(); delete surf_normal;
	
  (*temp)(4, 1) = 0;
	surf_normal = temp->normalize();
	
	temp->Erase(); delete temp;

	return surf_normal;
}

Sphere::Sphere()
{
	type = OBJ_SPHERE;
}

Sphere::~Sphere()
{
}

void Sphere::setRayHit(Matrix<double>& start, Matrix<double>& direction)
{
	resetHit();

	Matrix<double>* start_s = *M_i * start;
	
	Matrix<double>* temp = *M_i * direction;
	Matrix<double>* direction_s = temp->normalize();
	temp->Erase(); delete temp;
  //Matrix<double>* direction_s = new Matrix<double>(direction);
	start_s->setHeight(3);
	direction_s->setHeight(3);

	double a = direction_s->multiplyDot(*direction_s);
	double b = start_s->multiplyDot(*direction_s);
	double c = start_s->multiplyDot(*start_s) - 1;

	start_s->setHeight(4);
	direction_s->setHeight(4);

	double isRayHit = b*b - a*c;

	if (isRayHit >= 0)
	{
		double t1 = -b / a + sqrt(isRayHit) / a,
			t2 = -b / a - sqrt(isRayHit) / a;

		double t1r = calculateRealT(t1, start, *start_s, direction, *direction_s);

		if (t2 != t1)
			t2 = calculateRealT(t2, start, *start_s, direction, *direction_s);
		
		setHitEnterAndExit(t1r, t2);
	}

	start_s->Erase(); delete start_s;
	direction_s->Erase(); delete direction_s;
}

Matrix<double>* Sphere::calculateSurfaceNormal(const Matrix<double>& intersection, unsigned hit_type)
{
  Matrix<double>* intersection_s = *M_i * intersection;

	Matrix<double>* surf_normal = new Matrix<double>(*intersection_s);
	(*surf_normal)(4, 1) = 0;
  intersection_s->Erase(); delete intersection_s;

	Matrix<double>* temp = *M_it * *surf_normal;
	surf_normal->Erase(); delete surf_normal;
  (*temp)(4, 1) = 0;
	surf_normal = temp->normalize();
	
	temp->Erase(); delete temp;

	return surf_normal;
}

Cone::Cone()
{
	type = OBJ_CONE;
}

Cone::~Cone()
{
}

void Cone::setRayHit(Matrix<double>& start, Matrix<double>& direction)
{
	resetHit();

	Matrix<double>* start_s = *M_i * start;
	
	Matrix<double>* temp = *M_i * direction;
	(*temp)(4, 1) = 0;
	Matrix<double>* direction_s = temp->normalize();
	temp->Erase(); delete temp;
	
	//--------for cone wall

	double a = (*direction_s)(X, 1)*(*direction_s)(X, 1) + (*direction_s)(Y, 1)*(*direction_s)(Y, 1) - 0.25 * (*direction_s)(Z, 1)*(*direction_s)(Z, 1);
	double b = (*start_s)(X, 1)*(*direction_s)(X, 1) + (*start_s)(Y, 1)*(*direction_s)(Y, 1) + (*direction_s)(Z, 1)*0.25*(1 - (*start_s)(Z, 1));
	double c = (*start_s)(X, 1)*(*start_s)(X, 1) + (*start_s)(Y, 1)*(*start_s)(Y, 1) - 0.25*(1 - (*start_s)(Z, 1))*(1 - (*start_s)(Z, 1));
	
	double isRayHit = b*b - a*c;

	if (isRayHit >= 0)
	{
		double t1 = -b / a + sqrt(isRayHit) / a,
			t2 = -b / a - sqrt(isRayHit) / a;

		if (t1 >= -BIAS && t2 >= -BIAS)
		{
		  double z1 = (*start_s)(Z, 1) + (*direction_s)(Z, 1)*t1,
			  z2 = (*start_s)(Z, 1) + (*direction_s)(Z, 1)*t2;

		  t1 = calculateRealT(t1, start, *start_s, direction, *direction_s);
		  t2 = calculateRealT(t2, start, *start_s, direction, *direction_s);

		  // when the ray enter & exit at cone walls,
		  // we've found both hit times
		  if (z1 >= -1 && z1 <= 1 && z2 >= -1 && z2 <= 1)
		  {
			  setHitEnterAndExit(t1, t2);

			  start_s->Erase(); delete start_s;
			  direction_s->Erase(); delete direction_s;
			  return;
		  }
		  // when ray enter or exit (but not both) cone wall,
		  // need to find another hit time
		  else if (z1 >= -1 && z1 <= 1)
		  {
			  rayOnObj.enter = t1;
		  }
		  else if (z2 >= -1 && z2 <= 1)
		  {
			  rayOnObj.enter = t2;
		  }
		}
	}

	if ((*direction_s)(Z, 1) != 0)
	{
		double t_cap = (-1 - (*start_s)(Z, 1)) / (*direction_s)(Z, 1);
		
		if (t_cap >= 0)
		{
		  double ray_x = (*start_s)(X, 1) + t_cap*(*direction_s)(X, 1);
		  double ray_y = (*start_s)(Y, 1) + t_cap*(*direction_s)(Y, 1);

		  if (sqrt(ray_x*ray_x + ray_y*ray_y) <= 1)
		  {
			  t_cap = calculateRealT(t_cap, start, *start_s, direction, *direction_s);
			  if (t_cap < rayOnObj.enter)
			  {
				  rayOnObj.exit_type = rayOnObj.enter_type;
				  rayOnObj.exit = rayOnObj.enter;
				  rayOnObj.enter_type = CONE_BASE;
				  rayOnObj.enter = t_cap;
			  }
			  else
			  {
				  rayOnObj.exit_type = CONE_BASE;
				  rayOnObj.exit = t_cap;
			  }
		  }
		}
	}

	start_s->Erase(); delete start_s;
	direction_s->Erase(); delete direction_s;
}

Matrix<double>* Cone::calculateSurfaceNormal(const Matrix<double>& intersection, unsigned hit_type)
{
	Matrix<double>* intersection_s = *M_i * intersection;
	Matrix<double>* temp = new Matrix<double>(4, 1, 0);

	if (hit_type == CONE_BASE)
		(*temp)(Z, 1) = -1;
	else
	{
		double coeff = -1 / sqrt(2*(*intersection_s)(X,1)*(*intersection_s)(X,1)+2* (*intersection_s)(Y,1)*(*intersection_s)(Y,1)+((1- (*intersection_s)(Z,1))/2)*((1 - (*intersection_s)(Z, 1)) / 2));
		(*temp)(X, 1) = 2 * (*intersection_s)(X, 1);
		(*temp)(Y, 1) = 2 * (*intersection_s)(Y, 1);
		(*temp)(Z, 1) = (1 - (*intersection_s)(Z, 1)) / 2;
		Matrix<double>* result = temp->multiplyDot(coeff);
		temp->Erase(); delete temp;
		temp = result;
	}

	Matrix<double>* surf_normal = *M_it * *temp;
	temp->Erase(); delete temp;
  (*surf_normal)(4, 1) = 0;

	temp = surf_normal->normalize();
	
	surf_normal->Erase(); delete surf_normal;
	intersection_s->Erase(); delete intersection_s;

	return temp;
}

