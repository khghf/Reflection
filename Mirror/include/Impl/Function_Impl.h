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
		return (TypesHash == typesHash) ?
			std::bit_cast<ReturnT(*)(ParameterTs...)>(FunctionAddress) :
			nullptr;
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
	
	inline bool FunctionInfo::IsCompatible(std::span<const VariableId> otherVariables) const
	{
		if (ParameterTypes.size() != otherVariables.size()) 
			return false;

		for (size_t i{}; i < ParameterTypes.size(); ++i)
		{
			const auto& parameter = ParameterTypes[i];
			const auto& otherParams = otherVariables[i];

			const bool SameType = (parameter.GetTypeId() == otherParams.GetTypeId());
			const bool SameArraySize = parameter.GetArraySize() == otherParams.GetArraySize();
			const bool otherParameterIsArray = otherParams.IsArray();
			const bool funParameterIsPoint = parameter.IsPointer();
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







	/*
	以下是对私有成员的注册处理
	*/
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

	//利用模板绕过编译器访问检查来收集并注册信息
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
	struct AutoMemberFunctionDataSetter
	{
		AutoMemberFunctionDataSetter(size_t id, std::string_view name, FunctionProperties properties = DefaultFunctionProperties)
		{
			MethodRegistration::SetRuntimeProperties(id, name, properties);
		}
	};
}