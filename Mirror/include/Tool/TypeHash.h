#pragma once
#include<string_view>
#include"Util.h"
template <typename T> constexpr std::string_view TypeName();

template <> constexpr std::string_view TypeName<void>() { return "void"; }

namespace mirror
{
	namespace detail
	{
		//从模板中提取类型字符串
		using type_name_prober = void;
		template <typename T>
		constexpr std::string_view wrapped_type_name()
		{
#ifdef __clang__
			return __PRETTY_FUNCTION__;
#elif defined(__GNUC__)
			return __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
			return __FUNCSIG__;
#else
	#error "Unsupported compiler"
#endif
		}
		constexpr std::size_t wrapped_type_name_prefix_length()
		{
			return wrapped_type_name<type_name_prober>().find(TypeName<type_name_prober>());
		}
		constexpr std::size_t wrapped_type_name_suffix_length()
		{
			return	wrapped_type_name<type_name_prober>().length()
					- wrapped_type_name_prefix_length()
					- TypeName<type_name_prober>().length();
		}
		
	}
	template <typename T>
	constexpr std::string_view TypeName()
	{
		constexpr auto wrapped_name = detail::wrapped_type_name<T>();
		constexpr auto prefix_length = detail::wrapped_type_name_prefix_length();
		constexpr auto suffix_length = detail::wrapped_type_name_suffix_length();
		constexpr auto type_name_length = wrapped_name.length() - prefix_length - suffix_length;
		return wrapped_name.substr(prefix_length, type_name_length);
	}
	/*
	为给定类型创建哈希码
	*/
	template <typename Type>
	constexpr uint64_t TypeHash()
	{
		return util::hash(TypeName<Type>());
	}
}