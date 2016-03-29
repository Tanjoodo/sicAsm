#ifndef INT24_HPP
#define INT24_HPP
class int24
{
public:
	int24();
	int24(unsigned char low, unsigned char mid, unsigned char high);
	int24(int n);

	int24 operator=(int24);
	int24 operator=(int);
	int24 operator+(int24&);
	int24 operator-(int24&);
	int24 operator>>(int);
	int24 operator<<(int);
	operator int();

	void store_into(unsigned char * loc);
	void store_into_then_zero(unsigned char * loc);

private:
	unsigned char internals[3];
};
#endif
