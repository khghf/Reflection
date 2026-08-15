#pragma once
#include"Impl_Internal.h"
//#include "TypeInfo.h"
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

		/*if constexpr (std::is_enum_v<T>)
		{
			EnumInfo*enumInfo = info.GetEnumInfo_Internal();
			*enumInfo = EnumInfo::Create<T>();
		}*/

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
		Serialization::FillTypeInfoJson<T>(info);
		Serialization::FillTypeInfoBinary<T>(info);
		//Serialization::FillTypeInfoYaml<T>(info);
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

	//inline const EnumInfo*TypeInfo::GetEnumInfo() const
	//{
	//	return GetEnumInfo_Internal();
	//}

	//inline EnumInfo*TypeInfo::GetEnumInfo_Internal() const
	//{
	//	//@todo 并非所有
	//	static EnumInfo info{};
	//	return &info;
	//}


	template <typename T>
	struct AutoRegisterTypeOnce;

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
	inline const EnumInfo& TypeId::GetEnumInfo() const
	{
		return mirror::GetEnumInfo(*this);
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

	
	
	/// <summary>
	/// 注册一个类型并返回其类型信息。
	/// </summary>
	/// <typeparam name="T">要注册的类型。</typeparam>
	/// <returns>与注册类型关联的类型信息对象的引用。</returns>
	template <typename T>
	inline const auto& RegisterType()
	{
		auto& globalData = GetGlobalData();
		auto& nameToTypeIdMap = globalData.NameToTypeIdMap;
		constexpr TypeId typeId = TypeId::Create<T>();


		auto& typeInfoMap = globalData.TypeInfoMap;
		auto& vTableToTypeIDMap = globalData.VTableToTypeIdMap;
		const auto it = typeInfoMap.find(typeId);
		if (it == typeInfoMap.end())
		{
			auto& createdTypeInfo = typeInfoMap.emplace(
				typeId,
				TypeInfo::Create<T>()
			).first->second;
			nameToTypeIdMap.emplace(TypeName<T>(), typeId);
			if (createdTypeInfo.VTable)vTableToTypeIDMap.emplace(createdTypeInfo.VTable, typeId);


			if constexpr (std::is_enum_v<T>)
			{
				auto& enumInfoMap = globalData.EnumInfoMap;
				auto it = enumInfoMap.find(typeId);
				if (it == enumInfoMap.end())
				{
					it = enumInfoMap.emplace(typeId, EnumInfo::Create<T>()).first;
					nameToTypeIdMap.emplace(TypeName<T>(), typeId);
				}
				createdTypeInfo.EnumInfoPtr = &(it->second);
			}
			//std::cout << "注册" << TypeName<T>() << std::endl;

			std::cout << "GlobalData@" << (void*)&globalData
				<< " typeId=" << typeId.GetId()
				<< " TypeName:" << TypeName<T>() << std::endl;
			return createdTypeInfo;
		}
		return it->second;
	}
	
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
		inline static AutoRegisterType<T> StaticRegister{};
	};


	/// <summary>
	/// 注册一个子类到其父类的类型信息中。
	/// </summary>
	/// <typeparam name="Parent">父类的类型。</typeparam>
	/// <typeparam name="Child">子类的类型。</typeparam>
	template <typename Parent, typename Child>
	inline void RegisterChild()
	{
		auto& parentInfo = const_cast<TypeInfo&>(RegisterType<Parent>());
		auto& childInfo = const_cast<TypeInfo&>(RegisterType<Child>());

		assert(parentInfo.ChildClasses.end() == std::ranges::find(parentInfo.ChildClasses, TypeId::Create<Child>()));
		assert(childInfo.BaseClasses.end() == std::ranges::find_if(childInfo.BaseClasses, [](BaseClassInfo info) { return info.BaseId == TypeId::Create<Parent>(); }));

		if constexpr (std::is_default_constructible_v<Child>)
		{
			Child child{};
			Parent* parent = &child;

			// Register VTable
			//GetGlobalData().VTableMap.emplace(GetVTable(parent), TypeId::Create<Child>());
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
		inline static AutoRegisterChildOnce_Internal StaticRegisterChild{};
	};

	template<typename T>
	inline EnumInfo EnumInfo::Create()
	{
		EnumInfo info{};
		info.Name = TypeName<T>();
		return info;
	}

	template<typename T>
	struct AutoRegisterEnumItem
	{
		AutoRegisterEnumItem(std::string_view name, uint64_t value)
		{
			if constexpr (std::is_enum_v<T>)
			{
				EnumItem newItem{};
				newItem.Name = name;
				newItem.Value = value;

				TypeId id = TypeId::Create<T>();

				auto& map = GetGlobalData().EnumInfoMap;
				auto info = map.find(id);
				if (info!= map.end()&&!info->second.GetItem(name.data()))
				{
					info->second.Items.emplace_back(std::move(newItem));
				}
			}
		}
	};
}
