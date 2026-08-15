#pragma once
#include<string_view>
#include<bit>
#include"MemberInfo.h"
#include<assert.h>
namespace mirror
{
	class TypeTuple;

	/// <summary>
	/// 描述一个函数的信息，包括其地址、返回类型、名称、参数类型、所属类型以及调用方式。
	/// </summary>
	struct FunctionInfo final
	{
		const void*					FunctionAddress{ };	//函数地址
		VariableId					ReturnType{ };		//返回值
		std::string					Name{ };			//函数名
		std::vector<VariableId>		ParameterTypes{ };	//参数 

		
		/// <summary>
		/// 返回值和参数的哈希组合
		/// </summary>
		uint64_t						TypesHash{ };

		/// <summary>
		/// 函数所属的类型
		/// </summary>
		TypeId							OwningType{ };

		/// <summary>
		/// 附加的额外属性
		/// </summary>
		FunctionProperties				Properties{ };

		/// <summary>
		/// 调用指定函数地址并传递参数，返回结果。
		/// </summary>
		void(*FunctionCaller)(const void*funAddress, TypeTuple&parameters, void*returnVal);

		/// <summary>
		/// 调用指定实例的方法。
		/// </summary>
		void(*MethodCaller)(const void* funAddress, void* instance, TypeTuple& parameters, void* returnVal);
	public:
		
		/// <summary>
		/// 为普通函数创建FunctionInfo
		/// </summary>
		/// <typeparam name="TReturnType">返回值类型</typeparam>
		/// <typeparam name="...TParameterTypes">参数类型</typeparam>
		/// <param name="function">函数指针</param>
		/// <param name="name">函数名</param>
		/// <param name="properties">要附加的属性</param>
		/// <returns></returns>
		template <typename TReturnType, typename ... TParameterTypes>
		static FunctionInfo Create(TReturnType(*function)(TParameterTypes...), std::string_view name, FunctionProperties properties);

		/// <summary>
		/// 为成员函数创建FunctionInfo
		/// </summary>
		/// <typeparam name="Class"></typeparam>
		/// <typeparam name="TReturnType"></typeparam>
		/// <typeparam name="...TParameterTypes"></typeparam>
		/// <param name="function"></param>
		/// <param name="name"></param>
		/// <param name="properties"></param>
		/// <returns></returns>
		template <typename Class, typename TReturnType, typename ... TParameterTypes>
		static FunctionInfo Create(TReturnType(Class::* function)(TParameterTypes...), std::string_view name, FunctionProperties properties);
		template <typename Class, typename TReturnType, typename ... TParameterTypes>
		static FunctionInfo Create(TReturnType(Class::* function)(TParameterTypes...) const, std::string_view name, FunctionProperties properties);


		/// <summary>
		/// 将函数地址转换成函数指针返回，转换失败返回nullptr
		/// </summary>
		/// <typeparam name="ReturnT"></typeparam>
		/// <typeparam name="...ParameterTs"></typeparam>
		/// <returns></returns>
		template <typename ReturnT, typename... ParameterTs>
		auto Cast() const -> ReturnT(*)(ParameterTs...);

		/// <summary>
		/// 将函数地址转换成成员函数指针，转换失败返回nullptr
		/// </summary>
		/// <typeparam name="Class"></typeparam>
		/// <typeparam name="ReturnT"></typeparam>
		/// <typeparam name="...ParameterTs"></typeparam>
		/// <returns></returns>
		template <typename Class, typename ReturnT, typename... ParameterTs>
		auto MethodCast() const -> ReturnT(Class::*)(ParameterTs...);
	
		/// <summary>
		/// 调用函数，使用给定的参数元组，并可选地存储返回值。
		/// </summary>
		/// <param name="parameters">参数元组，包含调用函数所需的所有参数。</param>
		/// <param name="pReturnValue">一个可选的指针，用于存储函数的返回值。如果为 nullptr，则忽略返回值。</param>
		void Call(TypeTuple& parameters, void* pReturnValue = nullptr) const;
		
		/// <summary>
		/// 调用成员函数。
		/// </summary>
		/// <param name="instance">指向成员函数所属实例的指针。</param>
		/// <param name="parameters">包含调用成员函数所需参数的 TypeTuple 对象。</param>
		/// <param name="pReturnValue">可选参数，指向存储返回值的指针。如果成员函数没有返回值，可以为 nullptr。</param>
		void MemberCall(void* instance, TypeTuple& parameters, void* pReturnValue = nullptr) const;

		/// <summary>
		/// 检查是否设置了指定的函数属性。
		/// </summary>
		/// <param name="property">要检查的函数属性。</param>
		/// <returns>如果指定的属性已设置，则返回 true；否则返回 false。</returns>
		constexpr bool IsPropertySet(FunctionProperties property) const { return !!(Properties & property); }

		/// <summary>
		/// 检查是否为成员函数
		/// </summary>
		/// <returns></returns>
		constexpr bool IsMethod() const { return MethodCaller; }

		/// <summary>
		/// 检查给定的变量集合是否与函数的参数兼容。
		/// </summary>
		/// <param name="otherVariables">一个常量变量 ID 的 span，表示要检查兼容性的变量集合。</param>
		/// <returns>如果变量集合兼容，则返回 true；否则返回 false。</returns>
		bool IsCompatible(std::span<const VariableId> otherVariables) const;
	};


	class FunctionId final
	{
	public:
		constexpr FunctionId() = default;
		constexpr explicit FunctionId(uint64_t functionHash) : m_FunctionHash{ functionHash } {}
	public:
		constexpr uint64_t GetId() const { return m_FunctionHash; }
		void SetId(uint64_t id) { m_FunctionHash = id; }

		/**
		 * Get the FunctionInfo associated with this function
		 * @see FunctionInfo
		 */
		const FunctionInfo* GetInfo() const;
		/**
		将函数地址转换成函数指针
		 */
		template <typename ReturnType, typename... ParameterTypes>
		auto Cast() const -> ReturnType(*)(ParameterTypes...);
		/**
		将函数地址转换成成员函数指针
		 */
		template <typename Class, typename ReturnType, typename... ParameterTypes>
		auto MethodCast() const -> ReturnType(Class::*)(ParameterTypes...);
		/**
		为给定函数创建一个FunctionId。
		 */
		template <typename ReturnType, typename ... ParameterTypes>
		static FunctionId Create(ReturnType(*function)(ParameterTypes...), std::string_view name);
		/**
		为给定成员函数(无const)创建一个FunctionId。
		 */
		template <typename Class, typename ReturnType, typename ... ParameterTypes>
		static FunctionId Create(ReturnType(Class::* function)(ParameterTypes...), std::string_view name);
		/**
		为给定成员函数(const)创建一个FunctionId。
		 */
		template <typename Class, typename ReturnType, typename ... ParameterTypes>
		static FunctionId Create(ReturnType(Class::* function)(ParameterTypes...) const, std::string_view name);
		/*
		获取与给定函数相关联的FunctionId。
		使用此函数前，必须先注册该函数。
		 */
		template <typename ReturnType, typename ... ParameterTypes>
		static FunctionId GetFunctionId(ReturnType(*function)(ParameterTypes...));
		/*
		获取与给定成员函数(无const)相关联的FunctionId。
		使用此函数前，必须先注册该函数。
		 */
		template <typename Class, typename ReturnType, typename ... ParameterTypes>
		static FunctionId GetFunctionId(ReturnType(Class::* function)(ParameterTypes...));
		/**
		获取与给定成员函数(有const)相关联的FunctionId。
		使用此函数前，必须先注册该函数。
		 */
		template <typename Class, typename ReturnType, typename ... ParameterTypes>
		static FunctionId GetFunctionId(ReturnType(Class::* function)(ParameterTypes...) const);
		/**
		 获取与给函数地址相关联的FunctionId。
		使用此函数前，必须先注册该函数。
		 */
		static FunctionId GetFunctionId(const void* functionAddress);

		void Call(TypeTuple& parameters, void* pReturnValue = nullptr) const;

		void MemberCall(void* instance, TypeTuple& parameters, void* pReturnValue = nullptr) const;
		void MemberCall(void* instance, TypeTuple&& parameters, void* pReturnValue = nullptr) const;
		constexpr bool IsValid()const { return m_FunctionHash; }
		constexpr operator bool()const { return IsValid(); }
	private:
		uint64_t m_FunctionHash{};
	};





}