#define _USE_MATH_DEFINES
#include <cmath>
#include "GenericObject.h"

GenericObject::GenericObject()
{
	M = new Matrix<double>(4, 4);
}

void GenericObject::setHitEnterAndExit(double hit1, double hit2)
{
	if (hit1 <= hit2)
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
	M->Erase();
	delete M;
	MInverse->Erase();
	delete MInverse;
}

void GenericObject::setType(unsigned t)
{
	type = t;
}

void GenericObject::setColor(unsigned color_type, double r, double g, double b)
{
	switch (color_type)
	{
	case COLOR_ORIGIN:
		color.r = r; color.g = g; color.b = b; break;
	case COLOR_DIFFUSE:
		diff_color.r = r; diff_color.g = g; diff_color.b = b; break;
	case COLOR_SPEC:
		spec_color.r = r; spec_color.g = g; spec_color.b = b; break;
	case COLOR_AMBIENT:
		amb_color.r = r; amb_color.g = g; amb_color.b = b; break;
	}
	
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

void GenericObject::setAffineTransMat(Matrix<double>& mat)
{
	M->copy(mat);
	MInverse = M->inverse();
}


const unsigned GenericObject::getType()
{
	return type;
}

const double GenericObject::getFallout()
{
	return fallout;
}

const hit_t GenericObject::getRayHit()
{
	return rayOnObj;
}

double GenericObject::getAbmient(unsigned channel)
{
	switch (channel)
	{
	case COLOR_R: return amb_color.r * amb_coeff;
	case COLOR_G: return amb_color.g * amb_coeff;
	case COLOR_B: return amb_color.b * amb_coeff;
	default:
		return 0;
	}
}

double GenericObject::getDiffuse(unsigned channel)
{
	switch (channel)
	{
	case COLOR_R: return diff_color.r * diff_coeff;
	case COLOR_G: return diff_color.g * diff_coeff;
	case COLOR_B: return diff_color.b * diff_coeff;
	default:
		return 0;
	}
}

double GenericObject::getSpecular(unsigned channel)
{
	switch (channel)
	{
	case COLOR_R: return spec_color.r * spec_coeff;
	case COLOR_G: return spec_color.g * spec_coeff;
	case COLOR_B: return spec_color.b * spec_coeff;
	default:
		return 0;
	}
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
	Matrix<double>* temp = MInverse->multiply(direction);
	Matrix<double>* direction_s = temp->normalize();
	temp->Erase(); delete temp;

	Matrix<double>* start_s = MInverse->multiply(start);

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
		double z1 = (*start_s)(Z, 1) + (*direction_s)(Z, 1) * t1,
			   z2 = (*start_s)(Z, 1) + (*direction_s)(Z, 1) * t2;

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
	
	//-----------for cylinder caps

	if ((*direction_s)(Z, 1) != 0)
	{
		double t_cap = (((*direction_s)(Z, 1)<0?1:-1) - (*start_s)(Z, 1)) / (*direction_s)(Z, 1);
		double ray_x = (*start_s)(X, 1) + t_cap*(*direction_s)(X, 1);
		double ray_y = (*start_s)(Y, 1) + t_cap*(*direction_s)(Y, 1);
		if (sqrt(ray_x*ray_x + ray_y*ray_y) <= 1)
		{
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

	start_s->Erase(); delete start_s;
	direction_s->Erase(); delete direction_s;
}

Matrix<double>* Cylinder::calculateSurfaceNormal(const Matrix<double>& intersection, unsigned hit_type)
{
	Matrix<double>* surf_normal = new Matrix<double>(4, 1, 0);

	if (hit_type == CYLINDER_BASE)
		(*surf_normal)(Z, 1) = -1;
	else if (hit_type == CYLINDER_TOP)
		(*surf_normal)(Z, 1) = 1;
	else
	{
		double denom = sqrt(intersection(X, 1)*intersection(X, 1) + intersection(Y, 1)*intersection(Y, 1));
		(*surf_normal)(X, 1) = intersection(X, 1) / denom;
		(*surf_normal)(Y, 1) = intersection(Y, 1) / denom;
	}

	return surf_normal;
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

	Matrix<double>* temp = MInverse->multiply(direction);
	Matrix<double>* direction_s = temp->normalize();
	Matrix<double>* start_s = MInverse->multiply(start);

	if ((*direction_s)(Z, 1) != 0)
	{
		rayOnObj.enter = -(*start_s)(Z, 1) / (*direction_s)(Z, 1);
		rayOnObj.exit = rayOnObj.enter;
	}

	temp->Erase(); delete temp;
	start_s->Erase(); delete start_s;
	direction_s->Erase(); delete direction_s;
}

Matrix<double>* Plane::calculateSurfaceNormal(const Matrix<double>& intersection, unsigned hit_type)
{
	Matrix<double>* surf_normal = new Matrix<double>(4, 1, 0);

	(*surf_normal)(Z, 1) = 1;
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

	Matrix<double>* start_s = MInverse->multiply(start);
	
	Matrix<double>* temp = MInverse->multiply(direction);
	Matrix<double>* direction_s = temp->normalize();
	temp->Erase(); delete temp;

	double a = direction_s->multiplyDot(*direction_s);
	double b = start_s->multiplyDot(*direction_s);
	double c = start_s->multiplyDot(*start_s) - 1;

	double isRayHit = b*b - a*c;

	if (isRayHit >= 0)
	{
		double t1 = -b / a + sqrt(isRayHit) / a,
			t2 = -b / a - sqrt(isRayHit) / a;

		setHitEnterAndExit(t1, t2);
	}

	start_s->Erase(); delete start_s;
	direction_s->Erase(); delete direction_s;
}

Matrix<double>* Sphere::calculateSurfaceNormal(const Matrix<double>& intersection, unsigned hit_type)
{
	Matrix<double>* surf_normal = new Matrix<double>(4, 1, 0);

	surf_normal->copy(intersection);
	(*surf_normal)(4, 1) = 0;

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

	Matrix<double>* start_s = MInverse->multiply(start);
	
	Matrix<double>* temp = MInverse->multiply(direction);
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

		double z1 = (*start_s)(Z, 1) + (*direction_s)(Z, 1)*t1,
			z2 = (*start_s)(Z, 1) + (*direction_s)(Z, 1)*t2;

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

	if ((*direction_s)(Z, 1) > 0)
	{
		double t_cap = (-1 - (*start_s)(Z, 1)) / (*direction_s)(Z, 1);
		double ray_x = (*start_s)(X, 1) + t_cap*(*direction_s)(X, 1);
		double ray_y = (*start_s)(Y, 1) + t_cap*(*direction_s)(Y, 1);

		if (sqrt(ray_x*ray_x + ray_y*ray_y) <= 1)
		{
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

	start_s->Erase(); delete start_s;
	direction_s->Erase(); delete direction_s;
}

Matrix<double>* Cone::calculateSurfaceNormal(const Matrix<double>& intersection, unsigned hit_type)
{
	Matrix<double>* surf_normal = new Matrix<double>(4, 1, 0);

	if (hit_type == CONE_BASE)
		(*surf_normal)(Z, 1) = -1;
	else
	{
		double coeff = -1 / sqrt(2*intersection(X,1)*intersection(X,1)+2*intersection(Y,1)*intersection(Y,1)+((1-intersection(Z,1))/2)*((1 - intersection(Z, 1)) / 2));
		(*surf_normal)(X, 1) = 2 * intersection(X, 1);
		(*surf_normal)(Y, 1) = 2 * intersection(Y, 1);
		(*surf_normal)(Z, 1) = (1 - intersection(Z, 1)) / 2;
		Matrix<double>* temp = surf_normal->multiplyDot(coeff);
		surf_normal->Erase(); delete surf_normal;
		surf_normal = temp;
	}
	return surf_normal;
}

