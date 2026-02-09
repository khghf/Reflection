#pragma once
#include<cstdint>
#include"../Tool/Util.h"
namespace mirror
{
	using EnumBase = uint32_t;
	using MemberEnumBase = EnumBase;
	using FunctionEnumBase = EnumBase;
	enum class MemberProperties : EnumBase
	{
		None = 0,
		Serializable = 1 << 0,
	};

	inline constexpr MemberProperties DefaultMemberProperties{
		MemberProperties::Serializable
	};

	enum class FunctionProperties : EnumBase
	{
		None = 0,
		Method = 1 << 0,
		ConstMethod = 1 << 1,
	};

	inline constexpr FunctionProperties DefaultFunctionProperties{
		FunctionProperties::None
	};
}
ENABLE_ENUM_BIT_OPERATION(mirror::MemberProperties)
ENABLE_ENUM_BIT_OPERATION(mirror::FunctionProperties)



