#pragma once
#include"Impl_Internal.h"
namespace mirror
{
	template <typename ReturnType, typename ... ParameterTypes>
	inline void FunctionCallerHelper(const void* address, TypeTuple& tupleStorage, void* returnAddress)
	{
		ReturnType(*function)(ParameterTypes...);
		function = std::bit_cast<decltype(function)>(address);
		if constexpr (std::is_same_v<ReturnType, void>)
		{
			(void)returnAddress;

			TupleFunctionCall<std::tuple<ParameterTypes...>>(
				function,
				tupleStorage,
				std::make_index_sequence<sizeof...(ParameterTypes)>());
		}
		else
		{
			if (returnAddress)
				*static_cast<ReturnType*>(returnAddress) = TupleFunctionCall<std::tuple<ParameterTypes...>>(
					function,
					tupleStorage,
					std::make_index_sequence<sizeof...(ParameterTypes)>());
			else
				TupleFunctionCall<std::tuple<ParameterTypes...>>(
					function,
					tupleStorage,
					std::make_index_sequence<sizeof...(ParameterTypes)>());
		}
	}
	/// <summary>
	/// 成员函数调用助手、编译期保存类型信息，运行期通过函数指针调用
	/// </summary>
	/// <typeparam name="Class">实例类型</typeparam>
	/// <typeparam name="ReturnType">函数返回值</typeparam>
	/// <typeparam name="...ParameterTypes">参数</typeparam>
	/// <param name="address">函数地址</param>
	/// <param name="instance">实例对象</param>
	/// <param name="tupleStorage">存储参数的元组</param>
	/// <param name="returnAddress">通过指针的方式来接受返回值</param>
	template <typename Class, typename ReturnType, typename ... ParameterTypes>
	inline void MethodCallerHelper(const void* address, void* instance,TypeTuple& tupleStorage, void* returnAddress)
	{
		ReturnType(Class:: * function)(ParameterTypes...);
		function = reinterpret_cast<decltype(function)&>(address);
		if constexpr (std::is_same_v<ReturnType, void>)
		{
			(void)returnAddress;

			TupleMethodCall<std::tuple<ParameterTypes...>, Class>(
				function,
				instance,
				tupleStorage,
				std::make_index_sequence<sizeof...(ParameterTypes)>());
		}
		else
		{
			if (returnAddress)
				*static_cast<ReturnType*>(returnAddress) = TupleMethodCall<std::tuple<ParameterTypes...>, Class>(
					function,
					instance,
					tupleStorage,
					std::make_index_sequence<sizeof...(ParameterTypes)>());
			else
				TupleMethodCall<std::tuple<ParameterTypes...>, Class>(
					function,
					instance,
					tupleStorage,
					std::make_index_sequence<sizeof...(ParameterTypes)>());
		}
	}

	template <typename FunctionParameterTuple, typename Function, size_t... Index>
	inline auto TupleFunctionCall(Function function, TypeTuple& typeTuple, std::index_sequence<Index...>)
	{
		return function(ConvertParameter<FunctionParameterTuple, Index>(typeTuple)...);
	}
	/// <summary>
	/// 实际的函数调用，实现隐式类型转换
	/// </summary>
	/// <typeparam name="FunctionParameterTuple">std::tuple<....>存储参数类型信息</typeparam>
	/// <typeparam name="Class">类</typeparam>
	/// <typeparam name="Function">函数指针</typeparam>
	/// <typeparam name="...Index">参数索引</typeparam>
	/// <param name="function"></param>
	/// <param name="instance"></param>
	/// <param name="typeTuple">函数实参</param>
	/// <param name=""></param>
	/// <returns></returns>
	template <typename FunctionParameterTuple, typename Class, typename Function, size_t... Index>
	inline auto TupleMethodCall(Function function, void* instance, TypeTuple& typeTuple, std::index_sequence<Index...>)
	{
		return (static_cast<Class*>(instance)->*function)(ConvertParameter<FunctionParameterTuple, Index>(typeTuple)...);
	}

	template <typename FunctionParameterTuple, size_t Index>
	inline auto& ConvertParameter(TypeTuple& typeTuple)
	{
		using ParameterType = std::tuple_element_t<Index, FunctionParameterTuple>;
		return Convert<ParameterType>(typeTuple, Index);
		//using ParameterType = std::tuple_element_t<Index, FunctionParameterTuple>;
		//using RetType = ParameterType&;
		//auto variable = typeTuple.GetVariable(Index);
		//constexpr bool IsParamPtr = std::is_pointer_v<ParameterType>;
		//const bool IsStoragePtr = variable.IsPointer();
		//if (IsStoragePtr)
		//{
		//	// 场景1：TypeTuple存储的是指针Ty1*，形参也是指针Ty2*
		//	if constexpr (IsParamPtr)
		//	{
		//		// 返回ParameterType&(Ty2*&)
		//		return typeTuple.Get<ParameterType>(Index);
		//	}
		//	// 场景2：TypeTuple存储的是指针Ty1*，形参不是指针 Ty2
		//	else
		//	{
		//		// 取出存储的指针，解引用后返回ParameterType&（Ty1&）
		//		using PtrType = std::remove_reference_t<ParameterType>*;
		//		PtrType* pStoragePtr = &typeTuple.Get<PtrType>(Index); 
		//		assert(pStoragePtr && *pStoragePtr);
		//		return **pStoragePtr; // 解引用为Ty1&，匹配RetType
		//	}
		//}
		//// 场景3：TypeTuple存储的非指针Ty1，形参是指针Ty2*
		//else if constexpr (IsParamPtr)
		//{
		//	using ValueType = std::remove_pointer_t<ParameterType>;
		//	static_assert(std::is_same_v<decltype(typeTuple.Get<ValueType>(Index)), ValueType&>,
		//		"Storage type must match the pointee type of parameter!");
		//	ValueType& storageVal = typeTuple.Get<ValueType>(Index);
		//	ParameterType pVal = &storageVal; 
		//	return *std::addressof(pVal); 
		//}
		//// 场景4：TypeTuple存储的非指针Ty1，形参也非指针Ty2
		//else
		//{
		//	//TypeConvert::Convert<ParameterType>()
		//	return typeTuple.Get<ParameterType>(Index);
		//}
	}


	template <typename ReturnType, typename ... ParameterTs>
	inline FunctionInfo FillFunctionInfo(const void* address, std::string_view name, FunctionProperties properties)
	{
		FunctionInfo info{};

		info.FunctionAddress = address;
		info.ReturnType = VariableId::Create<ReturnType>();
		info.Name = name;
		info.TypesHash = GetTypesHash<ReturnType, ParameterTs...>();
		info.Properties = properties;

		constexpr size_t parameterPackSize = sizeof...(ParameterTs);

		if constexpr (parameterPackSize != 0)
		{
			info.ParameterTypes.resize(parameterPackSize);
			auto parameterTypes = GetVariableArray<ParameterTs...>();

			std::copy(parameterTypes.begin(), parameterTypes.end(), info.ParameterTypes.begin());
		}
		return info;
	}
	template <typename TReturnType, typename ... TParameterTypes>
	inline FunctionInfo FunctionInfo::Create(TReturnType(*function)(TParameterTypes...), std::string_view name, FunctionProperties properties)
	{
		FunctionInfo info = FillFunctionInfo<TReturnType, TParameterTypes...>(std::bit_cast<const void*>(function), name, properties);
		info.FunctionCaller = &FunctionCallerHelper<TReturnType, TParameterTypes...>;
		return info;
	}
	template <typename Class, typename TReturnType, typename ... TParameterTypes>
	inline FunctionInfo FunctionInfo::Create(TReturnType(Class::* function)(TParameterTypes...), std::string_view name, FunctionProperties properties)
	{
		FunctionInfo info = FillFunctionInfo<TReturnType, TParameterTypes...>(std::bit_cast<const void*>(function), name, properties);
		info.OwningType = TypeId::Create<Class>();
		info.MethodCaller = &MethodCallerHelper<Class, TReturnType, TParameterTypes...>;
		return info;
	}
	template <typename Class, typename TReturnType, typename ... TParameterTypes>
	inline FunctionInfo FunctionInfo::Create(TReturnType(Class::* function)(TParameterTypes...) const, std::string_view name, FunctionProperties properties)
	{
		FunctionInfo info = FillFunctionInfo<TReturnType, TParameterTypes...>(std::bit_cast<const void*>(function), name, properties);
		info.OwningType = TypeId::Create<Class>();
		info.MethodCaller = &MethodCallerHelper<Class, TReturnType, TParameterTypes...>;
		return info;
	}

	template <typename ReturnT, typename... ParameterTs>
	inline auto FunctionInfo::Cast() const->ReturnT(*)(ParameterTs...)
	{
		constexpr uint64_t typesHash = GetTypesHash<ReturnT, ParameterTs...>();
		return (TypesHash == typesHash) ?std::bit_cast<ReturnT(*)(ParameterTs...)>(FunctionAddress) :nullptr;
	}

	template <typename Class, typename ReturnT, typename... ParameterTs>
	inline auto FunctionInfo::MethodCast() const->ReturnT(Class::*)(ParameterTs...)
	{
		constexpr uint64_t typesHash = GetTypesHash<ReturnT, ParameterTs...>();

		ReturnT(Class:: * function)(ParameterTs...);
		function = reinterpret_cast<const decltype(function)&>(FunctionAddress);

		return (TypesHash == typesHash && OwningType == TypeId::Create<Class>()) ?
			function : nullptr;
	}

	inline void FunctionInfo::Call(TypeTuple& parameters, void* pReturnValue ) const
	{
		assert(FunctionCaller);
		//assert(IsCompatible(parameters.GetVariableIds()));
		FunctionCaller(FunctionAddress, parameters, pReturnValue);
	}

	inline void FunctionInfo::MemberCall(void* instance, TypeTuple& parameters, void* pReturnValue ) const
	{
		assert(MethodCaller && instance);
		//assert(IsCompatible(parameters.GetVariableIds()));
		MethodCaller(FunctionAddress, instance, parameters, pReturnValue);
	}

	/*
	判断给定的VariableIds是否与该函数参数兼容,若满足以下条件则返回true
	1、数量相同
	2、每个元素类型(原始类型,int int* int&都视为相同)相同
	3、每个元素数组长度相同(非数组为1)或者实参为数组、形参为指针
	4、每个元素const限定符相同
	*/
	inline bool FunctionInfo::IsCompatible(std::span<const VariableId> otherVariables) const
	{
		//1、数量相同
		if (ParameterTypes.size() != otherVariables.size()) 
			return false;

		for (size_t i{}; i < ParameterTypes.size(); ++i)
		{
			const auto& parameter = ParameterTypes[i];
			const auto& otherParams = otherVariables[i];
			//2、每个元素类型(原始类型,int int* int&都视为相同)相同
			const bool SameType = (parameter.GetTypeId() == otherParams.GetTypeId());
			
			//3、每个元素数组长度相同(非数组为1)或者实参为数组、形参为指针
			const bool SameArraySize = parameter.GetArraySize() == otherParams.GetArraySize();
			const bool otherParameterIsArray = otherParams.IsArray();
			const bool funParameterIsPoint = parameter.IsPointer();

			//4、每个元素const限定符相同
			const bool ConstCorrect = !(otherParams.IsConst() && !parameter.IsConst());

			if (!SameType || !SameArraySize&&!(funParameterIsPoint&&otherParameterIsArray)|| !ConstCorrect)
				return false;
		}
		return true;
	}

	inline const FunctionInfo* FunctionId::GetInfo() const
	{
		auto& functionMap = GetGlobalData().FunctionInfoMap;
		const auto it = functionMap.find(*this);
		if (it != functionMap.end())
		{
			return &it->second;
		}
		return nullptr;
	}

	template <typename ReturnType, typename... ParameterTypes>
	inline auto FunctionId::Cast() const->ReturnType(*)(ParameterTypes...)
	{
		return GetInfo()->Cast<ReturnType, ParameterTypes...>();
	}

	template <typename Class, typename ReturnType, typename... ParameterTypes>
	inline auto FunctionId::MethodCast() const->ReturnType(Class::*)(ParameterTypes...)
	{
		if (const auto info = GetInfo()) return info->MethodCast<Class, ReturnType, ParameterTypes...>();
		return nullptr;
	}

	template <typename ReturnType, typename ... ParameterTypes>
	inline FunctionId FunctionId::Create(ReturnType(*function)(ParameterTypes...), std::string_view name)
	{
		return FunctionId{ GetFunctionHash(function, name) };
	}

	template <typename Class, typename ReturnType, typename ... ParameterTypes>
	inline FunctionId FunctionId::Create(ReturnType(Class::* function)(ParameterTypes...), std::string_view name)
	{
		return FunctionId{ GetFunctionHash(function, name) };
	}

	template <typename Class, typename ReturnType, typename ... ParameterTypes>
	inline FunctionId FunctionId::Create(ReturnType(Class::* function)(ParameterTypes...) const, std::string_view name)
	{
		return FunctionId{ GetFunctionHash(function, name) };
	}

	template <typename ReturnType, typename ... ParameterTypes>
	inline FunctionId FunctionId::GetFunctionId(ReturnType(*function)(ParameterTypes...))
	{
		return GetFunctionId(std::bit_cast<const void*>(function));
	}

	template <typename Class, typename ReturnType, typename ... ParameterTypes>
	inline FunctionId FunctionId::GetFunctionId(ReturnType(Class::* function)(ParameterTypes...))
	{
		return GetFunctionId(std::bit_cast<const void*>(function));
	}

	template <typename Class, typename ReturnType, typename ... ParameterTypes>
	inline FunctionId FunctionId::GetFunctionId(ReturnType(Class::* function)(ParameterTypes...) const)
	{
		return GetFunctionId(std::bit_cast<const void*>(function));
	}

	inline FunctionId FunctionId::GetFunctionId(const void* functionAddress)
	{
		return GetGlobalData().FunctionAddressToIdMap[functionAddress];
	}

	inline void FunctionId::Call(TypeTuple& parameters, void* pReturnValue ) const
	{
		GetInfo()->Call(parameters, pReturnValue);
	}

	inline void FunctionId::MemberCall(void* instance, TypeTuple& parameters, void* pReturnValue) const
	{
		GetInfo()->MemberCall(instance,parameters, pReturnValue);
	}

	inline void FunctionId::MemberCall(void* instance, TypeTuple&& parameters, void* pReturnValue) const
	{
		TypeTuple tuple(std::move(parameters));
		MemberCall(instance, tuple, pReturnValue);
	}
	//--------------------------------------------------------------------------------------------------//
	//---------------------------------------------Register---------------------------------------------//
	//--------------------------------------------------------------------------------------------------//
	

	template <typename Class, typename ReturnType, typename ... ParameterTypes>
	inline const FunctionInfo& RegisterMethodFunctionHelper(FunctionId functionId, FunctionInfo&functionInfo ,const void* function, std::string_view name, FunctionProperties properties)
	{
		if (const FunctionInfo* functionInfo = functionId.GetInfo())
			return *functionInfo;

		TypeId::Create<Class>();
		FunctionInfo& info = functionInfo;

		auto& classInfo = const_cast<TypeInfo&>(RegisterType<Class>());
		classInfo.MemberFunctions.emplace_back(functionId);

		auto& globalFunctionData = GetGlobalData();

		globalFunctionData.NameToFunctionIdMap.emplace(name, functionId);
		globalFunctionData.FunctionAddressToIdMap.emplace(info.FunctionAddress, functionId);
		return globalFunctionData.FunctionInfoMap.emplace(functionId, std::move(info)).first->second;
	}
	//class::fun()
	template <typename Class, typename ReturnType, typename ... ParameterTypes>
	inline const FunctionInfo& RegisterMethodFunction(ReturnType(Class::* function)(ParameterTypes...), std::string_view name, FunctionProperties properties)
	{
		FunctionInfo info = FunctionInfo::Create(function, name, properties);
		return RegisterMethodFunctionHelper<Class, ReturnType, ParameterTypes...>(
			FunctionId::Create(function, name),
			info,
			reinterpret_cast<const void*&>(function),
			name,
			(properties | FunctionProperties::Method) & ~(FunctionProperties::ConstMethod)
		);
	}
	//class::fun() const
	template <typename Class, typename ReturnType, typename ... ParameterTypes>
	inline const FunctionInfo& RegisterConstMethodFunction(ReturnType(Class::* function)(ParameterTypes...) const, std::string_view name, FunctionProperties properties)
	{
		FunctionInfo info = FunctionInfo::Create(function, name, properties);
		return RegisterMethodFunctionHelper<Class, ReturnType, ParameterTypes...>(
			FunctionId::Create(function, name),
			info,
			reinterpret_cast<const void*&>(function),
			name,
			(properties | FunctionProperties::ConstMethod) & ~(FunctionProperties::Method));
	}

	//fun()
	template <typename ReturnType, typename ... ParameterTypes>
	inline const FunctionInfo& RegisterFunction(ReturnType(*function)(ParameterTypes...), std::string_view name, FunctionProperties properties)
	{
		FunctionId functionId = FunctionId::Create(function, name);

		if (const FunctionInfo* functionInfo = functionId.GetInfo())
			return *functionInfo;

		FunctionInfo info = FunctionInfo::Create(function, name, properties & ~(FunctionProperties::ConstMethod | FunctionProperties::Method));

		auto& globalFunctionData = GetGlobalData();

		globalFunctionData.NameToFunctionIdMap.emplace(name, functionId);
		globalFunctionData.FunctionAddressToIdMap.emplace(info.FunctionAddress, functionId);
		return globalFunctionData.FunctionInfoMap.emplace(functionId, std::move(info)).first->second;
	}

	struct AutoRegisterMemberFunction
	{
		template <typename Class, typename ReturnType, typename ... ParameterTypes>
		AutoRegisterMemberFunction(ReturnType(Class::* function)(ParameterTypes...), std::string_view name, FunctionProperties properties = DefaultFunctionProperties)
		{
			RegisterMethodFunction(function, name, properties);
		}
		template <typename Class, typename ReturnType, typename ... ParameterTypes>
		AutoRegisterMemberFunction(ReturnType(Class::* function)(ParameterTypes...) const, std::string_view name, FunctionProperties properties = DefaultFunctionProperties)
		{
			RegisterConstMethodFunction(function, name, properties);
		}
	};

	struct AutoRegisterFunction
	{
		template <typename ReturnType, typename ... ParameterTypes>
		AutoRegisterFunction(ReturnType(*function)(ParameterTypes...), std::string_view name, FunctionProperties properties = DefaultFunctionProperties)
		{
			RegisterFunction(function, name, properties);
		}
	};







	/// <summary>
	/// 函数注册表用于辅助私有函数的反射生成
	/// </summary>
	namespace MethodRegistration
	{
		inline auto& GetRegisteredMethods()
		{
			static std::unordered_map<size_t,FunctionId> RegisteredMethodss;
			return RegisteredMethodss;
		}

		inline void RegisterMethodWithId(size_t id, FunctionId funAccess)
		{
			GetRegisteredMethods().emplace(id, funAccess);
		}

		inline FunctionInfo* GetMethod(size_t id)
		{
			auto& function = GetRegisteredMethods()[id];
			return const_cast<FunctionInfo*>(function.GetInfo());
		}

		inline void SetRuntimeProperties(size_t id, std::string_view name, FunctionProperties properties = DefaultFunctionProperties)
		{
			auto function = GetMethod(id);
			assert(function);
			function->Name = name;
			function->Properties = properties;
		}
	}

	/// <summary>
	/// 利用编译器实例化模板绕过编译器检查来获取函数指针等主要信息
	/// </summary>
	/// <typeparam name="Method"></typeparam>
	/// <typeparam name="ID"></typeparam>
	template <auto Method, size_t ID>
	struct RegisterMethodType
	{
		template <typename Class, typename ReturnType, typename...ParameterTypes>
		static void* RegisterCompileTimeData(ReturnType(Class::*fun)(ParameterTypes...))
		{
			const FunctionInfo& funInfo = RegisterMethodFunction(fun, "", DefaultFunctionProperties);
			MethodRegistration::RegisterMethodWithId(ID, FunctionId::Create(fun,""));
			return nullptr;
		}
		template <typename Class, typename ReturnType, typename...ParameterTypes>
		static void* RegisterCompileTimeData(ReturnType(Class::* fun)(ParameterTypes...)const)
		{
			const FunctionInfo& funInfo = RegisterConstMethodFunction(fun, "", DefaultFunctionProperties);
			MethodRegistration::RegisterMethodWithId(ID, FunctionId::Create(fun, ""));
			return nullptr;
		}
		inline static const void* TypeAccessData = RegisterCompileTimeData(Method);
	};
	/// <summary>
	/// 用于天聪私有函数运行时未收集到的数据
	/// </summary>
	struct AutoMemberFunctionDataSetter
	{
		AutoMemberFunctionDataSetter(size_t id, std::string_view name, FunctionProperties properties = DefaultFunctionProperties)
		{
			MethodRegistration::SetRuntimeProperties(id, name, properties);
		}
	};
}