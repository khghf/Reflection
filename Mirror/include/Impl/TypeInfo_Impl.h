#pragma once
#include"Impl_Internal.h"
namespace mirror
{
	
	template <typename T>
	inline constexpr void FillTypeInfo(TypeInfo& info)
	{
		if constexpr (trait::EnableDefaultConstructor<T>)
			info.Constructor = [](void* location)
			{
				new (location) T();
			};

		if constexpr (trait::EnableCopyConstructor<T>)
			info.CopyConstructor = [](void* location, const void* other)
			{
				new (location) T(*static_cast<const T*>(other));
			};

		if constexpr (trait::EnableMoveConstructor<T>)
			info.MoveConstructor = [](void* location, void* other)
			{
				new (location) T(std::move(*static_cast<T*>(other)));
			};

		if constexpr (trait::EnableDestructor<T>)
			info.Destructor = [](void* data)
			{
				static_cast<T*>(data)->~T();
			};

		if constexpr (trait::EnableSwapping<T>)
			info.Swap = [](void* lhs, void* rhs)
			{
				std::swap(*static_cast<T*>(lhs), *static_cast<T*>(rhs));
			};
	}
	template <typename Parent, typename Child>
	inline constexpr BaseClassInfo BaseClassInfo::Create()
	{
		return { TypeId::Create<Parent>(), GetClassOffset<Parent, Child>() };
	}



	template <typename T>
	struct AddDependency {};

	template<typename T>
	inline TypeInfo TypeInfo::Create()
	{
		AddDependency<T> Dependency{};

		TypeInfo info{};

		info.Name = TypeName<T>();
		if constexpr (std::is_same_v<T, void>)
		{
			info.Size = 0;
			info.Align = 1;
		}
		else
		{
			info.Size = sizeof(T);
			info.Align = alignof(T);
		}
		if constexpr (std::is_polymorphic_v<T> && std::is_default_constructible_v<T>)
		{
			T instance{};
			info.VTable = GetVTable(&instance);
		}
		FillTypeInfo<T>(info);
		Serialization::FillTypeInfoJSon<T>(info);

#ifdef GLAS_SERIALIZATION_BINARY
		Serialization::FillTypeInfoBinary<T>(info);
#endif 
#ifdef GLAS_SERIALIZATION_YAML
		Serialization::FillTypeInfoYaml<T>(info);
#endif
		return info;
	}

	inline FunctionId TypeInfo::GetFunctionId(std::string name) const
	{
		FunctionId id{};
		if (auto result = std::ranges::find_if(MemberFunctions, [&name](auto in) { return in.GetInfo()->Name == name; }); result != MemberFunctions.end())
		{
			id = *result;
		}
		return id;
	}

	inline const MemberInfo* TypeInfo::GetMemberInfo(std::string name) const
	{
		if (auto result = std::ranges::find_if(Members, [&name](auto in) { return in.Name == name; }); result != Members.end())
		{
			return &*result;
		}
		return nullptr;
	}



	//--------------------TypeId--------------------//

	template <typename T>
	inline constexpr TypeId TypeId::Create()
	{
		AutoRegisterTypeOnce<T>();
		return TypeId(TypeHash<trait::strip_type_t<T>>());
	}
	
	
	inline const TypeInfo& TypeId::GetInfo() const
	{
		return GetTypeInfo(*this);
	}
	inline const  MemberInfo* TypeId::GetMemberInfo(size_t offset) const
	{
		auto& members = GetInfo().Members;

		MemberInfo info{};
		info.Offset = static_cast<uint32_t>(offset);

		const auto it = std::lower_bound(members.begin(), members.end(), info);
		if (it != members.end() && it->Offset == offset)
		{
			return &*it;
		}
		return nullptr;
	}
	inline  const MemberInfo* TypeId::GetMemberInfo(std::string name) const
	{
		return GetInfo().GetMemberInfo(name);
	}
	inline void TypeId::SetMemberData(void* instance, std::string name, void* data)const
	{
		GetMemberInfo(name)->Set(instance, data);
	}
	inline void TypeId::SetMemberData(void* instance, const MemberInfo& memberInfo, void* data)const
	{
		memberInfo.Set(instance, data);
	}
	template<typename T>
	inline T TypeId::GetMemberData(void*instance,std::string name)const
	{
		return GetMemberInfo(name)->Get(instance);
	}

	template<typename T>
	inline T TypeId::GetMemberData(void* instance, const MemberInfo& memberInfo)const
	{
		return memberInfo.Get(instance);
	}

	template<typename...T>
	inline constexpr bool TypeId::IsOneOf()const
	{
		return IsValid() && ((*this == Create<T>())||...);
	}

	//--------------------------------------------------------------------------------------------------//
	//---------------------------------------------Register---------------------------------------------//
	//--------------------------------------------------------------------------------------------------//

	template <typename T>
	const TypeInfo& RegisterType();
	template <typename T>
	struct AutoRegisterType
	{
		AutoRegisterType()
		{
			RegisterType<T>();
		}
	};
	template <typename T>
	struct AutoRegisterTypeOnce
	{
	private:
		struct AutoRegisterTypeOnce_Internal
		{
			AutoRegisterTypeOnce_Internal()
			{
				RegisterType<T>();
			}
		};
		inline static AutoRegisterTypeOnce_Internal StaticRegisterType{};
	};
	template <typename T>
	inline const TypeInfo& RegisterType()
	{
		auto& globalData = GetGlobalData();
		auto& typeInfoMap = globalData.TypeInfoMap;
		auto& nameToTypeIdMap = globalData.NameToTypeIdMap;

		constexpr TypeId hash = TypeId::Create<T>();

		const auto it = typeInfoMap.find(hash);
		if (it == typeInfoMap.end())
		{
			auto& createdTypeInfo = typeInfoMap.emplace(
				hash,
				TypeInfo::Create<T>()
			).first->second;

			nameToTypeIdMap.emplace(TypeName<T>(), hash);
			return createdTypeInfo;
		}
		return it->second;
	}

	template <typename Parent, typename Child>
	inline void RegisterChild()
	{
		auto& parentInfo = const_cast<TypeInfo&>(RegisterType<Parent>());
		auto& childInfo = const_cast<TypeInfo&>(RegisterType<Child>());

		if constexpr (std::is_default_constructible_v<Child>)
		{
			Child child{};
			Parent* parent = &child;
		}

		parentInfo.ChildClasses.emplace_back(TypeId::Create<Child>());
		childInfo.BaseClasses.emplace_back(BaseClassInfo::Create<Parent, Child>());
	}

	template <typename Parent, typename Child>
	struct AutoRegisterChildOnce
	{
	private:
		struct AutoRegisterChildOnce_Internal
		{
			AutoRegisterChildOnce_Internal()
			{
				RegisterChild<Parent, Child>();
			}
		};
		inline static AutoRegisterChildOnce_Internal StaticRegisterType{};
	};
}
