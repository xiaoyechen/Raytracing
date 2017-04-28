#include <climits>
//#include <algorithm>
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


GenericObject::~GenericObject()
{
	M->Erase();
	delete M;
	MInverse->Erase();
	delete MInverse;
}

const unsigned GenericObject::getType()
{
	return type;
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
	Matrix<double> direction_s = *MInverse->multiply(direction)->normalize();
	Matrix<double> start_s = *MInverse->multiply(start);

	//-----------for cylinder walls

	//calculate discriminant of ray-cylinderwall intersection equation
	double a = direction_s(X, 1)*direction_s(X, 1) + direction_s(Y, 1)*direction_s(Y, 1);
	double b = start_s(X, 1) * direction_s(X, 1) + start_s(Y, 1) * direction_s(Y, 1);
	double c = start_s(X, 1)*start_s(X, 1) + start_s(Y, 1)*start_s(X, 1) - 1;

	double isRayHit = b*b - a*c;

	// if ray does hit the object, check enter & exit types and distances
	if (isRayHit >= 0)
	{
		double t1 = -b / a + sqrt(isRayHit) / a,
			t2 = -b / a - sqrt(isRayHit) / a;
		double z1 = start_s(Z, 1) + direction_s(Z, 1) * t1,
			   z2 = start_s(Z, 1) + direction_s(Z, 1) * t2;

		// if both z1 and z2 are between -1 and 1 (inclusive),
		// then the ray enter and exit at cylinder walls
		if (z1 >= -1 && z1 <= 1 && z2 >= -1 && z2 <= 1)
		{
			setHitEnterAndExit(t1, t2);
			start_s.Erase();
			direction_s.Erase();
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

	if (direction_s(Z, 1) != 0)
	{
		double t_cap = ((direction_s(Z, 1)<0?1:-1) - start_s(Z, 1)) / direction_s(Z, 1);
		double ray_x = start_s(X, 1) + t_cap*direction_s(X, 1);
		double ray_y = start_s(Y, 1) + t_cap*direction_s(Y, 1);
		if (sqrt(ray_x*ray_x + ray_y*ray_y) <= 1)
		{
			// check if the ray intersects cylinder cap when entering
			// or exiting the cylinder object
			if (t_cap < rayOnObj.enter)
			{
				rayOnObj.exit_type = rayOnObj.enter_type;
				rayOnObj.exit = rayOnObj.enter;
				rayOnObj.enter_type = direction_s(Z, 1)<0?CYLINDER_TOP:CYLINDER_BASE;
				rayOnObj.enter = t_cap;
			}
			else 
			{
				rayOnObj.exit_type = direction_s(Z, 1)<0 ? CYLINDER_TOP:CYLINDER_BASE;
				rayOnObj.exit = t_cap;
			}
		}
	}

	start_s.Erase();
	direction_s.Erase();
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
	Matrix<double> direction_s = *MInverse->multiply(direction)->normalize();
	Matrix<double> start_s = *MInverse->multiply(start);

	if (direction_s(Z, 1) < 0)
	{
		rayOnObj.enter = -start_s(Z, 1) / direction_s(Z, 1);
		rayOnObj.exit = rayOnObj.enter;
	}

	start_s.Erase();
	direction_s.Erase();
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
	Matrix<double> start_s = *MInverse->multiply(start);
	Matrix<double> direction_s = *MInverse->multiply(direction);

	double a = direction_s.normal;
	a *= a;
	double b = start_s.multiplyDot(direction_s);
	double c = start_s.normal;
	c *= c;

	double isRayHit = b*b - a*c;

	if (isRayHit >= 0)
	{
		double t1 = -b / a + sqrt(isRayHit) / a,
			t2 = -b / a - sqrt(isRayHit) / a;

		setHitEnterAndExit(t1, t2);
	}

	start_s.Erase();
	direction_s.Erase();
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

	Matrix<double> start_s = *MInverse->multiply(start);
	Matrix<double> direction_s = *MInverse->multiply(direction);

	//--------for cone wall

	double a = direction_s(X, 1)*direction_s(X, 1) + direction_s(Y, 1)*direction_s(Y, 1) - 0.25 * direction_s(Z, 1)*direction_s(Z, 1);
	double b = start_s(X, 1)*direction_s(X, 1) + start_s(Y, 1)*direction_s(Y, 1) + direction_s(Z, 1)*0.25*(1 - start_s(Z, 1));
	double c = start_s(X, 1)*start_s(X, 1) + start_s(Y, 1)*start_s(Y, 1) - 0.25*(1 - start_s(Z, 1))*(1 - start_s(Z, 1));
	
	double isRayHit = b*b - a*c;

	if (isRayHit >= 0)
	{
		double t1 = -b / a + sqrt(isRayHit) / a,
			t2 = -b / a - sqrt(isRayHit) / a;

		double z1 = start_s(Z, 1) + direction_s(Z, 1)*t1,
			z2 = start_s(Z, 1) + direction_s(Z, 1)*t2;

		// when the ray enter & exit at cone walls,
		// we've found both hit times
		if (z1 >= -1 && z1 <= 1 && z2 >= -1 && z2 <= 1)
		{
			setHitEnterAndExit(t1, t2);

			start_s.Erase();
			direction_s.Erase();
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

	if (direction_s(Z, 1) > 0)
	{
		double t_cap = (-1 - start_s(Z, 1)) / direction_s(Z, 1);
		double ray_x = start_s(X, 1) + t_cap*direction_s(X, 1);
		double ray_y = start_s(Y, 1) + t_cap*direction_s(Y, 1);

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

	start_s.Erase();
	direction_s.Erase();
}