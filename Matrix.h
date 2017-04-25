#pragma once

template<class T>
class Matrix
{
public:
	Matrix();
	Matrix(int height, int length);
	Matrix(unsigned height, unsigned length);
	Matrix(int height, int length, T val);
	Matrix(int axis, double rotation_theta);
	~Matrix();
	
	T& operator()(int i, int j);
	T operator()(int i, int j) const;
	void Erase();
	void printFormat();

	void identity();
	Matrix<T>* add(const Matrix<T> &matB);
	Matrix<T>* subtract(const Matrix<T> &matB);
	Matrix<T>* multiply(const Matrix<T> &matB);
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
