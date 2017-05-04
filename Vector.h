#pragma once

template<class T>
class Vector
{
private:
	int m_length;
	T *m_v;
public:
	Vector();
	Vector(int length, int height);
	~Vector();
	
	T& operator[](int idx);

	void Erase();
	int length();
};
