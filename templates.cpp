/* This class explicitly tells compiler to instantiate
	the template class for required types so that we 
	don't get linker errors
*/

// include header and definition of all template classes
#include "Matrix.h"
#include "Matrix.cpp"

#include "Vector.h"
#include "Vector.cpp"

// classes for all required data types
template class Matrix<double>;