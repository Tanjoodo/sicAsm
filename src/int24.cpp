#include "../include/int24.hpp"

int24::int24()
{
	this->internals[0] = this->internals[1] = this->internals[2] = 0;
}

int24::int24(int n)
{
	for (int i = 0; i < 3; ++i)
		internals[i] = (n >> (i * 8)) & 0xFF;
}

int24::int24(unsigned char low, unsigned char mid, unsigned char high)
{
	this->internals[0] = low;
	this->internals[1] = mid;
	this->internals[2] = high;
}

int24 int24::operator=(int24 o)
{
	for (int i = 0; i < 3; ++i)
		this->internals[i] = o.internals[i];
	return int24(internals[0], internals[1], internals[2]);
}

int24 int24::operator=(int n)
{
	*this = int24(n);
	return *this;
}
int24::operator int()
{
	int r = 0;
	for (int i = 0; i < 3; ++i)
		r |= internals[i] << i * 8;
	
	return r;
}

int24 int24::operator+(int24& o)
{
	return int24((int)*this + (int)o);
}

int24 int24::operator-(int24& o)
{
	return int24((int)*this - (int)o);
}

int24 int24::operator>>(int n)
{
	return int24((int)*this >> n);
}

int24 int24::operator<<(int n)
{
	return int24((int)*this << n);
}

void int24::store_into(unsigned char * loc)
{
	loc[0] = internals[0];
	loc[1] = internals[1];
	loc[2] = internals[2];
}

void int24::store_into_then_zero(unsigned char * loc)
{
	this->store_into(loc);
	this->internals[0] = 
		this->internals[1] =
		this->internals[2] = 0;
}
