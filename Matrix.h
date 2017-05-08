#pragma once
#include <iostream>

template<class T>
class Matrix
{
public:
	Matrix();
	Matrix(int height, int length);
	Matrix(unsigned height, unsigned length);
	Matrix(int height, int length, T val);
	Matrix(int axis, double rotation_theta);
	Matrix(const Matrix<T> &mat);
	~Matrix();
	
	unsigned getLength() const;
	unsigned getHeight() const;

	void setLength(unsigned l);
	void setHeight(unsigned h);
	T& operator()(int i, int j);
	T operator()(int i, int j) const;
	void copy(const Matrix<T> &mat);
	void Erase();

	friend std::ostream& operator<<(std::ostream& out, const Matrix<T> &mat)
	{
		for (unsigned i = 1; i <= mat.getHeight(); ++i)
		{
			for (unsigned j = 1; j <= mat.getLength(); ++j)
				out << mat(i, j) << " ";
			out << "\n";
		}
		return out;
	}

	void identity();
	Matrix<T>* operator +(const Matrix<T> &matB);
	Matrix<T>* operator -(const Matrix<T> &matB);
	Matrix<T>* operator *(const Matrix<T> &matB);
	Matrix<T>* multiplyDot(T coeff);
	T multiplyDot(const Matrix<T> &matB);
	Matrix<T>* multiplyCross(Matrix<T> &matB);
	double normal();
	Matrix<T>* normalize();
	Matrix<T>* transpose();
	double determinant();
	Matrix<T>* cofactor();
	Matrix<T>* inverse();

private:
	unsigned m_length, m_height;
	T *m_m;
};
