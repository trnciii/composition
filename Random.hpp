#pragma once

#include <cstdint>

struct XORSHIFT32State;
struct XORSHIFT128State;

typedef XORSHIFT128State RNG; //default

///////////////////////////////////////

struct XORSHIFT128State{
	uint32_t x1 = 123456789;
	uint32_t x2 = 362436069;
	uint32_t x3 = 521288629;
	uint32_t x4 = 88675123;
	
	inline XORSHIFT128State(){}
	inline XORSHIFT128State(uint32_t s){seed(s);}
	
	inline void seed(uint32_t s){
		x1 = s = (uint32_t)1812433253*(s^(s>>30))+1;
		x2 = s = (uint32_t)1812433253*(s^(s>>30))+2;
		x3 = s = (uint32_t)1812433253*(s^(s>>30))+3;
		x4 = s = (uint32_t)1812433253*(s^(s>>30))+4;
	}

	inline uint32_t next(){
		uint32_t t = x4^x4<<11;
		x4=x3; x3=x2; x2=x1;
		return x1 = (t^t>>8)^x1^(x1>>19);
	}
	inline double uniform(){return (double)(next()-1)/0xffffffff;}
};

struct XORSHIFT32State{
	uint32_t a;

	inline XORSHIFT32State():a(999){}
	inline XORSHIFT32State(uint32_t s):a(s){}
	inline void setSeed(uint32_t s){a=s;}

	inline uint32_t next(){
		a ^= a<<13;
		a ^= a>>17;
		a ^= a<<5;
		return a;
	}
	inline double uniform(){return (double)(next()-1)/0xffffffff;}
};