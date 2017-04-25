#include "Vector.h"


template <typename T>
Vector<T>::Vector()
{
	m_length = 0;
}

template <typename T>
Vector<T>::Vector(int length, int height)
{
	m_v = new T[m_length+1];
	m_length = length;
}

template <typename T>
Vector<T>::~Vector()
{
	delete[] m_v;
}

template <typename T>
T& Vector<T>::operator[](int idx)
{
	assert(idx > 0 && idx <= m_length);
	return m_v[idx];
}

template <typename T>
int Vector<T>::length() { return m_length; }

template <typename T>
void Vector<T>::Erase()
{
	delete[] m_v;
	m_v = 0;
	m_length = 0;
}
