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
		/// <summary>
		/// 计算 `wrapped_type_name` 中类型名称前缀的长度。
		/// </summary>
		/// <returns></returns>
		constexpr std::size_t wrapped_type_name_prefix_length()
		{
			return wrapped_type_name<type_name_prober>().find(TypeName<type_name_prober>());
		}
		/// <summary>
		/// 计算类型名称包装的后缀长度。
		/// </summary>
		/// <returns></returns>
		constexpr std::size_t wrapped_type_name_suffix_length()
		{
			return	wrapped_type_name<type_name_prober>().length()
					- wrapped_type_name_prefix_length()
					- TypeName<type_name_prober>().length();
		}
		
	}
	/// <summary>
	/// 获取类型 T 的名称。
	/// </summary>
	/// <typeparam name="T">要获取名称的类型。</typeparam>
	/// <returns>类型 T 的名称，作为一个 constexpr std::string_view 返回。</returns>
	template <typename T>
	constexpr std::string_view TypeName()
	{
		constexpr auto wrapped_name = detail::wrapped_type_name<T>();
		constexpr auto prefix_length = detail::wrapped_type_name_prefix_length();
		constexpr auto suffix_length = detail::wrapped_type_name_suffix_length();
		constexpr auto type_name_length = wrapped_name.length() - prefix_length - suffix_length;
		return wrapped_name.substr(prefix_length, type_name_length);
	}
	/// <summary>
	/// 计算类型的哈希值(哈希名字)。
	/// </summary>
	/// <typeparam name="Type">要计算哈希值的类型。</typeparam>
	/// <returns>指定类型的哈希值，返回值为一个无符号64位整数。</returns>
	template <typename Type>
	constexpr uint64_t TypeHash()
	{
		return util::hash(TypeName<Type>());
	}
}