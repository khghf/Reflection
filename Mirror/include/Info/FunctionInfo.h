#pragma once
#include<string_view>
#include<bit>
#include"MemberInfo.h"
#include<assert.h>
namespace mirror
{
	class TypeTuple;
	struct FunctionInfo final
	{
		const void*					FunctionAddress{ };	//函数地址
		VariableId					ReturnType{ };		//返回值
		std::string					Name{ };			//函数名
		std::vector<VariableId>		ParameterTypes{ };	//参数 

		/*
		  返回值和参数的哈希组合
		 */
		uint64_t						TypesHash{ };
		/*
		函数所属的类型
		*/
		TypeId							OwningType{ };
		/*
		附加的额外属性
		 */
		FunctionProperties				Properties{ };
		//@see	Create and  Function_Impl.h:FunctionCallerHelper
		void(*FunctionCaller)(const void*funAddress, TypeTuple&parameters, void*returnVal);
		//@see 	Create and	Function_Impl.h:MethodCallerHelper
		void(*MethodCaller)(const void* funAddress, void* instance, TypeTuple& parameters, void* returnVal);
	public:
		/*
		 为普通函数创建FunctionInfo
		 */
		template <typename TReturnType, typename ... TParameterTypes>
		static FunctionInfo Create(TReturnType(*function)(TParameterTypes...), std::string_view name, FunctionProperties properties);
		/*
		 为成员函数(无const)创建FunctionInfo
		 */
		template <typename Class, typename TReturnType, typename ... TParameterTypes>
		static FunctionInfo Create(TReturnType(Class::* function)(TParameterTypes...), std::string_view name, FunctionProperties properties);
		/*
		 为成员函数(有const)创建FunctionInfo
		 */
		template <typename Class, typename TReturnType, typename ... TParameterTypes>
		static FunctionInfo Create(TReturnType(Class::* function)(TParameterTypes...) const, std::string_view name, FunctionProperties properties);
		/*
		将函数地址转换成函数指针返回，转换失败返回nullptr
		 */
		template <typename ReturnT, typename... ParameterTs>
		auto Cast() const -> ReturnT(*)(ParameterTs...);
		/*
		将函数地址转换成成员函数指针，转换失败返回nullptr
		 */
		template <typename Class, typename ReturnT, typename... ParameterTs>
		auto MethodCast() const -> ReturnT(Class::*)(ParameterTs...);
	
		void Call(TypeTuple& parameters, void* pReturnValue = nullptr) const;
		
		void MemberCall(void* instance, TypeTuple& parameters, void* pReturnValue = nullptr) const;
		/*
		检查函数是否设置了目标属性
		 */
		constexpr bool IsPropertySet(FunctionProperties property) const { return !!(Properties & property); }

		constexpr bool IsMethod() const { return MethodCaller; }

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