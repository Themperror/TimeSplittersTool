#pragma once
#include <cstdint>
class TSMaterial
{
public:
	enum class WrapMode : uint32_t
	{
		Repeat,
		NoRepeat
	};
	enum Flag : uint32_t
	{
		None,
		TexturesIncInModel = 268435456
	};
	uint32_t ID;
	WrapMode wrapX;
	WrapMode wrapY;
	Flag flag;
};