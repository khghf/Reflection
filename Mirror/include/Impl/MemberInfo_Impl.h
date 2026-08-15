#pragma once
#include"Impl_Internal.h"

namespace mirror
{
	template <typename T>
	constexpr VariableId VariableId::Create()
	{
		using Type_RemovedExtents = std::remove_all_extents_t<T>;
		using Type_RemovedRefs = std::remove_reference_t<Type_RemovedExtents>;
		using Type_RemovedPtrs =trait::remove_all_pointers_t<Type_RemovedRefs>;

		using StrippedType = std::remove_cvref_t<Type_RemovedPtrs>;

		AutoRegisterTypeOnce<StrippedType> TypeRegister{};

		constexpr bool IsRef{ std::is_reference_v<T> };
		constexpr bool IsRValRef{ std::is_rvalue_reference_v<T> };
		constexpr bool IsConst{ std::is_const_v<Type_RemovedPtrs> };
		constexpr bool IsVolatile{ std::is_volatile_v<Type_RemovedPtrs> };

		constexpr uint32_t PointerAmount{ util::CountPointers<Type_RemovedRefs>() };

		auto variable = VariableId(TypeId::Create<StrippedType>());

		if constexpr (IsConst)		variable.SetConstFlag();
		if constexpr (IsVolatile)	variable.SetVolatileFlag();
		if constexpr (IsRef)		variable.SetReferenceFlag();
		if constexpr (IsRValRef)	variable.SetRValReferenceFlag();

		variable.SetPointerAmount(PointerAmount);

		if constexpr (!std::is_same_v<void, Type_RemovedExtents>)
		{
			constexpr uint32_t ArraySize{ sizeof(T) / sizeof(Type_RemovedExtents) };
			variable.SetArraySize(ArraySize);
		}
		else
		{
			variable.SetArraySize(1);
		}

		return variable;
	}

	
	/// <summary>
	/// 辅助函数，用于设置类实例的字段值。
	/// </summary>
	/// <typeparam name="Class">类的类型。</typeparam>
	/// <typeparam name="Field">字段的类型。</typeparam>
	/// <param name="instance">指向类实例的指针。</param>
	/// <param name="typeTuple">包含字段值的 TypeTuple 对象。</param>
	/// <param name="offset">字段在类实例中的偏移量（以字节为单位）。</param>
	template<typename Class,typename Field>
	void SetterHelper(void* instance, TypeTuple& typeTuple, uint32_t offset)
	{
		Field* field = static_cast<Field*>(util::VoidOffset(instance, offset)) ;
		if constexpr (trait::IsMoveAssignOnly<Field>)
		{
			*field =std::move(Convert<Field>(typeTuple, 0));
		}
		else
		{
			*field =Convert<Field>(typeTuple, 0);
		}
	}
	/// <summary>
	/// 获取类实例中指定偏移量处的字段指针。
	/// </summary>
	/// <typeparam name="Class">类的类型。</typeparam>
	/// <typeparam name="Field">字段的类型。</typeparam>
	/// <param name="instance">指向类实例的指针。</param>
	/// <param name="offset">字段在类实例中的偏移量（以字节为单位）。</param>
	/// <returns>指向指定偏移量处字段的指针。</returns>
	template<typename Class, typename Field>
	void* GetterHelper(void* instance, uint32_t offset)
	{
		void* field = util::VoidOffset(instance, offset);
		return field;
	}

	template<typename T>
	inline void MemberInfo::Set(void* instance, T&& data) const
	{
		assert(instance);
		TypeTuple tuple(std::forward<T>(data));
		Setter(instance, tuple, Offset);
	}

	template<typename T>
	inline T MemberInfo::Get(void* instance) const
	{
		assert(instance);
		return *static_cast<T*>(Getter(instance, Offset));
	}

	template<typename T>
	inline T& MemberInfo::GetRef(void* instance) const
	{
		assert(instance);
		return *static_cast<T*>(Getter(instance, Offset));
	}


	template <typename Class,typename Field>
	const MemberInfo& RegisterField(VariableId memberId, std::string_view fieldName, uint32_t offset, uint32_t size, uint32_t align, MemberProperties properties)
	{
		MemberInfo info;
		info.Name = fieldName;
		info.Variable = memberId;
		info.Offset = offset;
		info.Size = size;
		info.Align = align;
		info.Properties = properties;
		info.Setter = &SetterHelper<Class,Field>;
		info.Getter = &GetterHelper<Class,Field>;

		auto& owner = const_cast<TypeInfo&>(RegisterType<Class>());

		auto it = std::upper_bound(owner.Members.begin(), owner.Members.end(), info);

		bool exists = false;
		if (it == owner.Members.begin())
		{
			return *owner.Members.emplace(it, std::move(info));
			
		}
		else
		{
			auto prev = it - 1;
			if (prev->Offset == info.Offset)
			{
				return *prev;
			}
			else
			{
				return *owner.Members.emplace(it, std::move(info));
			}
		}
	}

	struct AutoRegisterMember
	{
		template <typename Class,typename Field>
		AutoRegisterMember(Class*, Field*, VariableId memberId, std::string_view fieldName, uint32_t offset, uint32_t size, uint32_t align, MemberProperties properties = DefaultMemberProperties)
		{
			RegisterField<Class, Field>(memberId, fieldName, offset, size, align, properties);
		}
	};
	/*
	以下是对私有成员的注册处理
	*/
	namespace MemberRegistration
	{
		inline auto& GetRegisteredMembers()
		{
			//key:Id
			//value:TypeId-Offset
			static std::unordered_map<size_t, std::pair<TypeId, uint32_t>> RegisteredMembers;
			return RegisteredMembers;
		}

		inline void RegisterMemberWithId(size_t id, std::pair<TypeId, uint32_t> memberAccess)
		{
			GetRegisteredMembers().emplace(id, memberAccess);
		}

		inline MemberInfo* GetMember(size_t id)
		{
			auto& member = GetRegisteredMembers()[id];
			return const_cast<MemberInfo*>(member.first.GetMemberInfo(member.second));
		}

		inline void SetRuntimeProperties(size_t id, std::string_view name, MemberProperties properties = DefaultMemberProperties)
		{
			auto member = GetMember(id);
			assert(member);
			member->Name = name;
			member->Properties = properties;
		}
	}
	//利用模板绕过编译器访问检查来收集并注册编译期信息
	template <auto Member, size_t ID>
	struct RegisterMemberType
	{
		template <typename Class, typename T>
		static void* RegisterCompileTimeData(T Class::* member)
		{
			const MemberInfo& memberInfo = RegisterField<Class,T>(
				VariableId::Create<T>(),
				"",
				static_cast<uint32_t>(reinterpret_cast<size_t>(&(*static_cast<Class*>(nullptr).*member))),
				sizeof(decltype(member)),
				alignof(decltype(member)),
				DefaultMemberProperties
			);
			MemberRegistration::RegisterMemberWithId(ID, std::pair<TypeId, uint32_t>(TypeId::Create<Class>(), memberInfo.Offset));
			return nullptr;
		}

		inline static const void* TypeAccessData = RegisterCompileTimeData(Member);
	};
	struct AutoMemberVariableDataSetter
	{
		AutoMemberVariableDataSetter(size_t id, std::string_view name, MemberProperties properties = DefaultMemberProperties)
		{
			MemberRegistration::SetRuntimeProperties(id, name, properties);
		}
	};
}