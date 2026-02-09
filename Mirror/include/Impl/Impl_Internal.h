#pragma once
#ifndef IMPL_INTERNAL
#define IMPL_INTERNAL
#include<iostream>
#include<algorithm>
#include"../Info/TypeInfo.h"
#include"../Info/MemberInfo.h"
#include"../Info/FunctionInfo.h"

#include"../Tool/TypeHash.h"
#include"../Tool/Util.h"

#include"../Storage/TypeTuple.h"

#include"../Impl/TypeConverter.h"

template <>
struct std::hash<mirror::TypeId>
{
	std::size_t operator()(const mirror::TypeId& id) const noexcept
	{
		return static_cast<size_t>(id.GetId());
	}
};
template <>
struct std::hash<mirror::VariableId>
{
	std::size_t operator()(const mirror::VariableId& id) const noexcept
	{
		return static_cast<size_t>(id.GetHash());
	}
};
template <>
struct std::hash<mirror::FunctionId>
{
	std::size_t operator()(const mirror::FunctionId& id) const noexcept
	{
		return static_cast<size_t>(id.GetId());
	}
};
namespace mirror
{
	inline std::ostream& operator<<(std::ostream& lhs, const TypeId& rhs)
	{
		lhs << rhs.GetId();
		return lhs;
	}
	inline std::istream& operator>>(std::istream& lhs, TypeId& rhs)
	{
		uint64_t idInt{};
		lhs >> idInt;
		rhs.SetTypeId(idInt);

		return lhs;
	}
	inline std::ostream& operator<<(std::ostream& lhs, const VariableId& rhs)
	{
		lhs << rhs.m_Type << ' '
			<< rhs.m_ArraySize << ' '
			<< rhs.m_PointerAmount << ' '
			<< rhs.m_TraitFlags;
		return lhs;
	}
	inline std::istream& operator>>(std::istream& lhs, VariableId& rhs)
	{
		lhs >> rhs.m_Type
			>> rhs.m_ArraySize
			>> rhs.m_PointerAmount
			>> rhs.m_TraitFlags;
		return lhs;
	}
	inline constexpr bool operator==(TypeId lhs, TypeId rhs)
	{
		return lhs.GetId() == rhs.GetId();
	}
	inline constexpr bool operator==(const VariableId& lhs, const VariableId& rhs)
	{
		return lhs.m_Type == rhs.m_Type &&
			lhs.m_ArraySize == rhs.m_ArraySize &&
			lhs.m_PointerAmount == rhs.m_PointerAmount &&
			lhs.m_TraitFlags == rhs.m_TraitFlags;
	}
	inline constexpr bool operator==(FunctionId lhs, FunctionId rhs)
	{
		return lhs.GetId() == rhs.GetId();
	}
	/*
	存储全局反射信息
	*/
	struct GlobalData
	{
		GlobalData() = default;
		~GlobalData() = default;
		GlobalData(const GlobalData&) = delete;
		GlobalData(GlobalData&&) noexcept = delete;
		GlobalData& operator=(const GlobalData&) = delete;
		GlobalData& operator=(GlobalData&&) noexcept = delete;

		std::unordered_map<TypeId, TypeInfo>			TypeInfoMap{ };

		std::unordered_map<std::string, TypeId>			NameToTypeIdMap{ };

		std::unordered_map<FunctionId, FunctionInfo>	FunctionInfoMap{ };

		std::unordered_map<std::string, FunctionId>		NameToFunctionIdMap{ };

		std::unordered_map<const void*, FunctionId>		FunctionAddressToIdMap{ };
	};
	inline GlobalData& GetGlobalData() { static GlobalData data; return data; }

	inline std::unordered_map<TypeId, TypeInfo>& GetTypeInfoMap()
	{
		return GetGlobalData().TypeInfoMap;
	}

	inline const std::unordered_map<TypeId, TypeInfo>& GetAllTypeInfo()
	{
		return GetTypeInfoMap();
	}

	inline const TypeInfo& GetTypeInfo(TypeId id)
	{
		assert(GetTypeInfoMap().contains(id));
		return GetTypeInfoMap()[id];
	}

	template <typename ... Types>
	constexpr std::array<VariableId, sizeof...(Types)> GetVariableArray()
	{
		return std::array<VariableId, sizeof...(Types)> {VariableId::Create<Types>()...};
	}

	template <typename... Types>
	constexpr uint64_t GetTypesHash()
	{
		constexpr auto variableIds = GetVariableArray<Types...>();

		std::array<uint64_t, sizeof...(Types)> variableHashes{};

		std::transform(variableIds.begin(), variableIds.end(), variableHashes.begin(), [](VariableId id)
			{
				return id.GetHash();
			});

		return util::hash({ variableHashes.data(), variableHashes.size() });
	}
	//fun() hash
	template <typename ReturnType, typename ... ParameterTypes>
	uint64_t GetFunctionHash(ReturnType(*)(ParameterTypes...), std::string_view name)
	{
		return util::hash(name) ^ GetTypesHash<ReturnType, ParameterTypes...>();
	}
	//class::fun() hash
	template <typename Class, typename ReturnType, typename ... ParameterTypes>
	uint64_t GetFunctionHash(ReturnType(Class::*)(ParameterTypes...), std::string_view name)
	{
		return util::hash(name) ^ GetTypesHash<Class, ReturnType, ParameterTypes...>();
	}
	//class::fun() const hash
	template <typename Class, typename ReturnType, typename ... ParameterTypes>
	uint64_t GetFunctionHash(ReturnType(Class::*)(ParameterTypes...) const, std::string_view name)
	{
		return util::hash(name) ^ GetTypesHash<Class, ReturnType, ParameterTypes...>();
	}

	

	template <typename Parent, typename Child>
	constexpr size_t GetClassOffset()
	{
		static_assert(std::is_base_of_v<Parent, Child>,"Child must be a derived class of Parent");
		Child child{};
		return reinterpret_cast<const char*>(static_cast<Parent*>(&child)) - reinterpret_cast<const char*>(&child);
	}

	template <typename T>
	const void* GetVTable(const T* instance) requires std::is_polymorphic_v<T>
	{
		return *std::bit_cast<void**>(instance);
	}
}
#endif // !IMPL_INTERNAL
