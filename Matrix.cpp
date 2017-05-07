#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <math.h>
#include "Matrix.h"

//--------- Constructor and Destructor definition
template<typename T>
Matrix<T>::Matrix()
{
	m_length = 0;
	m_height = 0;
}

template<typename T>
Matrix<T>::Matrix(int height, int length)
	:Matrix((unsigned)height, (unsigned)length)
{
}

template<class T>
Matrix<T>::Matrix(unsigned height, unsigned length)
{
	m_m = new T[(length + 1)*(height + 1)];

	m_length = length;
	m_height = height;
}

template<class T>
Matrix<T>::Matrix(int height, int length, T val)
	: Matrix((unsigned)height, (unsigned)length)
{
	for (unsigned i = 1; i <= m_height; ++i)
	{
		for (unsigned j = 1; j <= m_length; ++j)
			(*this)(i, j) = val;
	}
}

template<class T>
Matrix<T>::Matrix(int axis, double rotation_theta)
	:Matrix(3,3,0)
{
	assert(axis >= 1 && axis <= 3);

	switch (axis)
	{
	case 1: 
		(*this)(1, 1) = 1;
		(*this)(2, 2) = cos(rotation_theta);
		(*this)(2, 3) = -sin(rotation_theta);
		(*this)(3, 2) = sin(rotation_theta);
		(*this)(3, 3) = cos(rotation_theta);
		break;
	case 2:
		(*this)(1, 1) = cos(rotation_theta);
		(*this)(1, 3) = sin(rotation_theta);
		(*this)(2, 2) = 1;
		(*this)(3, 1) = -sin(rotation_theta);
		(*this)(3, 3) = cos(rotation_theta);
		break;
	case 3:
		(*this)(1, 1) = cos(rotation_theta);
		(*this)(1, 2) = -sin(rotation_theta);
		(*this)(2, 1) = sin(rotation_theta);
		(*this)(2, 2) = cos(rotation_theta);
		(*this)(3, 3) = 1;
		break;
	default:
		break;
	}
}

template<typename T>
Matrix<T>::Matrix(const Matrix<T> &mat)
	:Matrix(mat.getHeight(), mat.getLength())
{
	copy(mat);
}

template<typename T>
Matrix<T>::~Matrix()
{
	delete[] m_m;
}

template<class T>
unsigned Matrix<T>::getLength() const
{
	return m_length;
}

template<class T>
unsigned Matrix<T>::getHeight() const
{
	return m_height;
}

template<class T>
void Matrix<T>::setLength(unsigned l)
{
	m_length = l;
}

template<class T>
void Matrix<T>::setHeight(unsigned h)
{
	m_height = h;
}

// free memory and reset pointer & h & l
template<typename T>
void Matrix<T>::Erase()
{
	delete[] m_m;
	m_m = 0;
	m_length = 0;
	m_height = 0;
}

// Matrix element accessor
template<typename T>
T & Matrix<T>::operator()(int i, int j)
{
	assert(i > 0 && i <= m_height && j > 0 && j <= m_length);

	return m_m[i * (m_length + 1) + j];
}

template<class T>
T Matrix<T>::operator()(int i, int j) const
{
	assert(i > 0 && i <= m_height && j > 0 && j <= m_length);

	return m_m[i * (m_length + 1) + j];
}

template<class T>
void Matrix<T>::copy(const Matrix<T> &mat)
{
	assert(mat.getHeight() == m_height && mat.getLength() == m_length);

	for (unsigned i = 1; i <= m_height; ++i)
	{
		for (unsigned j = 1; j <= m_length; ++j)
			(*this)(i, j) = mat(i, j);
	}
}

template<typename T>
void Matrix<T>::identity()
{
	for (unsigned i = 1; i <= m_height; ++i)
	{
		for (unsigned j = 1; j <= m_length; ++j)
		{
			if (i == j)
				(*this)(i, j) = 1;
			else
				(*this)(i, j) = 0;
		}
	}
}

template<typename T>
Matrix<T>* Matrix<T>::add(const Matrix<T>& matB)
{
	assert(m_length == matB.m_length && m_height == matB.m_height);

	Matrix<T>* result = new Matrix<T>(m_height, m_length);

	for (unsigned i = 1; i <= m_height; ++i)
	{
		for (unsigned j = 1; j <= m_length; ++j)
			(*result)(i,j) = (*this)(i, j) + matB(i, j);
	}

	return result;
}

template<class T>
Matrix<T>* Matrix<T>::subtract(const Matrix<T>& matB)
{
	assert(m_length == matB.m_length && m_height == matB.m_height);

	Matrix<T>* result = new Matrix<T>(m_height, m_length);

	for (unsigned i = 1; i <= m_height; ++i)
	{
		for (unsigned j = 1; j <= m_length; ++j)
			(*result)(i, j) = (*this)(i, j) - matB(i, j);
	}

	return result;
}

template<typename T>
Matrix<T> * Matrix<T>::multiply(const Matrix<T> &matB)
{
	assert(m_length == matB.m_height);

	Matrix<T> * result = new Matrix<T>(m_height, matB.m_length);

	for (unsigned i = 1; i <= result->m_height; ++i)
	{
		for (unsigned j = 1; j <= result->m_length; ++j)
		{
			T sum = 0;
			for (unsigned k = 1; k <= m_length; ++k)
				sum += (*this)(i, k) * matB(k, j);

			(*result)(i, j) = sum;
		}
	}
	return result;
}

template<class T>
Matrix<T>* Matrix<T>::multiplyDot(T coeff)
{
	Matrix<T>* result = new Matrix<T>(m_height, m_length);

	for (unsigned i = 1; i <= m_height; ++i)
	{
		for (unsigned j = 1; j <= m_length; ++j)
			(*result)(i, j) = coeff * (*this)(i,j);
	}
	return result;
}

template<class T>
T Matrix<T>::multiplyDot(const Matrix<T>& matB)
{
	assert((m_length == 1 && matB.m_length == 1) && m_height == matB.m_height);

	T sum = 0;
	for (unsigned i = 1; i <= m_height; ++i)
		sum += (*this)(i, 1) * matB(i, 1);
	
	return sum;
}

template<typename T>
Matrix<T>* Matrix<T>::multiplyCross(Matrix<T>& matB)
{
	assert((m_height == 1 || m_length == 1) && (matB.m_height == 1 || matB.m_length == 1));
	assert(m_height == 3 && matB.m_height == 3);

	Matrix<T>* C = transpose();
	Matrix<T>* D = matB.transpose();

	Matrix<T> E(3, 3, 0);
	E(1, 2) = -(*D)(1, 3);
	E(1, 3) = (*D)(1, 2);
	E(2, 1) = (*D)(1, 3);
	E(2, 3) = -(*D)(1, 1);
	E(3, 1) = -(*D)(1, 2);
	E(3, 2) = (*D)(1, 1);

	Matrix<T>* temp = C->multiply(E);
	Matrix<T>* result = temp->transpose();
	temp->Erase(); delete temp;
	
	C->Erase(); delete C;
	D->Erase(); delete D;
	E.Erase();

	return result;
}

template<class T>
double Matrix<T>::normal()
{
	// make sure this is a vector
	assert(m_length == 1 || m_height == 1);
	
	double sum = 0;
	for (unsigned i = 1; i <= m_height; ++i)
	{
		for (unsigned j = 1; j <= m_length; ++j)
			sum += (*this)(i, j) * (*this)(i, j);
	}

	return sqrt(sum);
}

template<class T>
Matrix<T>* Matrix<T>::normalize()
{
	Matrix<T>* normalized = new Matrix<T>(m_height, m_length);

	double n = normal();

	for (unsigned i = 1; i <= m_height; ++i)
	{
		for (unsigned j = 1; j <= m_length; ++j)
			(*normalized)(i, j) = (*this)(i, j) / n;
	}

	return normalized;
}

template<class T>
Matrix<T>* Matrix<T>::transpose()
{
	Matrix<T>* result = new Matrix<T>(m_length, m_height);

	for (unsigned i = 1; i <= m_height; ++i)
	{
		for (unsigned j = 1; j <= m_length; ++j)
			(*result)(j,i) = (*this)(i, j);
	}
	return result;
}

template<class T>
double Matrix<T>::determinant()
{
	assert(m_height >=1 && m_length >= 1);
	assert(m_height == m_length);

	double det = 0;
	if (m_height == 1)
		det = (*this)(1, 1);
	else if (m_height == 2)
		det = (*this)(1, 1) * (*this)(2, 2) - (*this)(2, 1) * (*this)(1, 2);
	else
	{
		for (unsigned j1 = 1; j1 <= m_height; ++j1)
		{
			Matrix<T> M(m_height - 1, m_height - 1);

			for (unsigned i = 2; i <= m_height; ++i)
			{
				unsigned j2 = 1;
				for (unsigned j = 1; j <= m_height; ++j)
				{
					if (j == j1)
						continue;

					M(i - 1, j2) = (*this)(i, j);
					++j2;
				}
			}

			det += pow(-1.0, j1 + 1) * (*this)(1, j1) * M.determinant();

			M.Erase();
		}
	}

	return det;
}

template<typename T>
Matrix<T>* Matrix<T>::cofactor()
{
	Matrix<T> *result = new Matrix<T>(m_height, m_length);
	Matrix<T> C(m_height - 1, m_length - 1);

	for (unsigned i = 1; i <= m_height; ++i)
	{
		for (unsigned j = 1; j <= m_height; ++j)
		{
			unsigned j1 =1;
			for (unsigned jj = 1; jj <= m_height; ++jj)
			{
				if (jj == j)
					continue;

				unsigned i1=1;
				for (unsigned ii = 1; ii <= m_height; ++ii)
				{
					if (ii == i)
						continue;
					C(j1, i1) = (*this)(jj, ii);
					++i1;
				}
				++j1;
			}

			(*result)(i, j) = pow(-1.0, i + j + 2) * C.determinant();
		}
	}

	C.Erase();

	return result;
}

template<typename T>
Matrix<T>* Matrix<T>::inverse()
{
	double det = determinant();

	Matrix<T>* temp = cofactor();
	Matrix<T>* result = temp->transpose();
	temp->Erase(); delete temp;

	for (unsigned i = 1; i <= result->m_height; ++i)
	{
		for (unsigned j = 1; j <= result->m_length; ++j)
			(*result)(i, j) /= det;
	}
	return result;
}
/*
template<typename T>
std::ostream & operator<<(std::ostream & out, const Matrix<T> &mat)
{
	for (unsigned i = 1; i <= mat.getHeight(); ++i)
	{
		for (unsigned j = 1; j < mat.getLength(); ++j)
			out << mat(i, j) << ", ";
		out << "\n";
	}
	return out;
}
*/
