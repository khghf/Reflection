#pragma once
#include<string_view>
#include<span>
#include<type_traits>
namespace util
{
	constexpr uint64_t hash(std::string_view str)
	{
		std::uint64_t hash_value = 0xcbf29ce484222325ULL;
		constexpr std::uint64_t prime = 0x100000001b3ULL;
		for (const char c : str)
		{
			hash_value ^= static_cast<uint64_t>(c);
			hash_value *= prime;
		}
		return hash_value;
	}

	constexpr uint64_t hash(std::span<const uint64_t> span)
	{
		std::uint64_t hash_value = 0xcbf29ce484222325ULL;
		constexpr std::uint64_t prime = 0x100000001b3ULL;
		for (const uint64_t c : span)
		{
			hash_value ^= static_cast<const uint64_t>(c);
			hash_value *= prime;
		}
		return hash_value;
	}
	template <typename T>
	inline constexpr uint32_t CountPointers(uint32_t counter = 0)
	{
		if constexpr (std::is_pointer_v<T>)
			return CountPointers<std::remove_pointer_t<T>>(++counter);
		else
			return counter;
	}
	inline const void* VoidOffset(const void* data, size_t offset)
	{
		return static_cast<const uint8_t*>(data) + offset;
	}

	inline void* VoidOffset(void* data, size_t offset)
	{
		return static_cast<uint8_t*>(data) + offset;
	}

	

}

/*
	枚举位运算模板，使用ENABLE_ENUM_BIT_OPERATION可快速定义位运算
*/
template <typename Enum>
inline constexpr bool EnumEnableBitOperations = false;

template <typename Enum>
concept BitFieldOperators = EnumEnableBitOperations<Enum> && std::is_enum_v<Enum>;

template <typename Enum>
constexpr Enum operator|(Enum lhs, Enum rhs) requires BitFieldOperators<Enum>
{
	using T = std::underlying_type_t<Enum>;
	return static_cast<Enum>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

template <typename Enum>
constexpr Enum operator&(Enum lhs, Enum rhs) requires BitFieldOperators<Enum>
{
	using T = std::underlying_type_t<Enum>;
	return static_cast<Enum>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

template <typename Enum>
constexpr Enum operator^(Enum lhs, Enum rhs) requires BitFieldOperators<Enum>
{
	using T = std::underlying_type_t<Enum>;
	return static_cast<Enum>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

template <typename Enum>
constexpr Enum operator~(Enum val) requires BitFieldOperators<Enum>
{
	using T = std::underlying_type_t<Enum>;
	return static_cast<Enum>(~static_cast<T>(val));
}

template <typename Enum>
constexpr bool operator!(Enum val) requires BitFieldOperators<Enum>
{
	return !static_cast<bool>(val);
}

#define ENABLE_ENUM_BIT_OPERATION(ENUM) template <> inline constexpr bool EnumEnableBitOperations<ENUM> = true;