#pragma once

#include "vector.h"
#include <math.h>

namespace nm 
{

struct index2 
{
// 	index2() { operator=( outIndex ); }
// 	//index2(int sx, int sy) : x(sx), y(sy) {}
// 
// 	template<class T0, class T1>
// 	index2(T0 sx, T1 sy) : x(static_cast<int>(sx)), y(static_cast<int>(sy)) {}
		
	int x, y;
	
	static const index2 outIndex;
	static const index2 zeroIndex;
	static const index2 oneIndex;

	
	bool operator==( const index2& i ) const { return ( i.x == x ) && ( i.y == y ); }
	bool operator!=( const index2& i ) const { return ! operator==( i ); }

	index2 operator+( const index2& i ) const	{ index2 res(*this); res += i; return res; }
	index2 operator-( const index2& i ) const	{ index2 res(*this); res -= i; return res; }
	index2 operator-() const { index2 res{ -x, -y }; return res; }


	index2& operator+=( const index2& i ) { x += i.x; y += i.y; return *this;}
	index2& operator-=( const index2& i ) { x -= i.x; y -= i.y; return *this;}

	index2 operator*( int d ) const	{ index2 res(*this); res *= d; return res; }
	index2 operator/( int d ) const	{ index2 res(*this); res /= d; return res; }
	index2 operator%( int d ) const	{ index2 res(*this); res %= d; return res; }

	index2& operator*=( int d ) { x *= d; y *= d; return *this;}
	index2& operator/=( int d ) { x /= d; y /= d; return *this;}
	index2& operator%=( int d ) { x %= d; y %= d; return *this;}

	index2 operator*( const index2& i ) const	{ index2 res(*this); res *= i; return res; }
	index2 operator/( const index2& i ) const	{ index2 res(*this); res /= i; return res; }
	index2 operator%( const index2& i ) const	{ index2 res(*this); res %= i; return res; }
	index2& operator*=( const index2& i )		{ x *= i.x; y *= i.y; return *this;}
	index2& operator/=( const index2& i )		{ x /= i.x; y /= i.y; return *this;}
	index2& operator%=( const index2& i )		{ x %= i.x; y %= i.y; return *this;}
	
	bool IsCorrect() const { return operator!=( outIndex ); }
	inline int Square() const { return abs(x*y); }
	inline int magnitudeSq() const	{ return x*x + y*y; }

};

struct index2rect
{
	nm::index2 pos;
	nm::index2 size;
	nm::index2 denominator;

	int left() const { return pos.x / denominator.x; }
	int right() const { return (pos.x + size.x) / denominator.x; }
	int top() const { return pos.y / denominator.y; }
	int bottom() const { return (pos.y + size.y) / denominator.y; }

	bool is_inside(const nm::index2& p) const
	{
		return (p.x >= left()) && (p.x < right()) && (p.y >= top()) && (p.y < bottom());
	}

	float left(float denominatorFactor) const { return pos.x / (denominator.x * denominatorFactor); }
	float right(float denominatorFactor) const { return (pos.x + size.x) / (denominator.x * denominatorFactor); }
	float top(float denominatorFactor) const { return pos.y / (denominator.y * denominatorFactor); }
	float bottom(float denominatorFactor) const { return (pos.y + size.y) / (denominator.y * denominatorFactor); }

	bool collapsed() const 
	{
		return (size.x == 0) && (size.y == 0);
	}

	void collapse()
	{
		pos = index2::zeroIndex;
		size = index2::zeroIndex;
		denominator = index2::oneIndex;
	}
};


struct index2Iterator	
{
	index2Iterator() = default;
	index2Iterator(const index2& b, const index2& e) : val(b), begin(b), end(e) {}

	bool Forward()
	{ 
		if (val.y < end.y)
		{
			if (++val.x == end.x)
			{
				val.x = begin.x;
				++val.y;
			}
		}		

		return isCanForward();
	}

	auto flatten() const
	{
		auto size = end - begin;
		auto it = val - begin;
		return (size.x * it.y + it.x);
	}

	auto count() const
	{
		auto size = end - begin;
		return size.x * size.y;
	}

	bool isCanForward() const
	{
		return flatten() < count();
	}

	const index2& operator()() const { return val; }

	index2 val;
	index2 begin;
	index2 end;
};


inline Vector2 floatize(const index2& idx) { return Vector2((float) idx.x, (float) idx.y); }

}