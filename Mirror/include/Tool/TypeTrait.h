#pragma once
#include<type_traits>
namespace trait
{
	/*
	移除指针
	*/
	template <typename T> struct remove_all_pointers
	{
		using Type = T;
	};

	template <typename T> struct remove_all_pointers<T*>
	{
		using Type = typename remove_all_pointers<T>::Type;
	};

	template <typename T>
	using remove_all_pointers_t = typename remove_all_pointers<T>::Type;

	/*
	移除所有修饰符
	*/
	template <typename T> struct strip_type
	{
		using Type = std::remove_cvref_t<remove_all_pointers_t<std::remove_reference_t<std::remove_all_extents_t<T>>>>;
	};

	template <typename T>
	using strip_type_t = typename strip_type<T>::Type;


	template <typename T>
	concept STDSwappable = requires(T & lhs, T & rhs) { std::swap(lhs, rhs); };

	template <typename T>
	concept AlternativeSwappable = requires(T & lhs, T & rhs) { swap(lhs, rhs); };

	template <typename T>
	concept Swappable = STDSwappable<T> || AlternativeSwappable<T>;

	template<typename T>
	inline constexpr bool HasDefaultConstructor = std::is_default_constructible_v<T>;

	template <typename T>
	inline constexpr bool HasDestructor = std::is_destructible_v<T>;

	template <typename T>
	inline constexpr bool HasCopyConstructor = std::is_copy_constructible_v<T>;

	template <typename T>
	inline constexpr bool HasMoveConstructor = std::is_move_constructible_v<T>;

	template<typename T>
	inline constexpr bool HasCopyAssign = std::is_copy_assignable_v<T>;

	template<typename T>
	inline constexpr bool HasMoveAssign = std::is_move_assignable_v<T>;

	template <typename T>
	inline constexpr bool HasSwapping = Swappable<T>;

	template<typename T>
	inline constexpr bool IsMoveAssignOnly =!HasCopyAssign<T>&& HasMoveAssign<T>;




	//启用以给T类型的TypeInfo的构造、移动构造的指针赋值@see FillTypeInfo
	template<typename T>
	inline constexpr bool EnableDefaultConstructor = std::is_default_constructible_v<T>;

	template <typename T>
	inline constexpr bool EnableDestructor = std::is_destructible_v<T>;

	template <typename T>
	inline constexpr bool EnableCopyConstructor = std::is_copy_constructible_v<T>;

	template <typename T>
	inline constexpr bool EnableMoveConstructor = std::is_move_constructible_v<T>;

	template <typename T>
	inline constexpr bool EnableSwapping = Swappable<T>;
}
