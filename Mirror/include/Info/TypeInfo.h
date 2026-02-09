#pragma once

#include<string>
#include<vector>
#include<unordered_map>
#include<assert.h>
#include<cstdint>
#include<algorithm>
#include"../Tool/TypeTrait.h"
#include"../Tool/TypeHash.h"
namespace mirror
{
	struct TypeInfo;
	struct MemberInfo;
	class FunctionId;
	class TypeId final
	{
	public:
		constexpr TypeId() = default;
		constexpr ~TypeId() = default;

		constexpr explicit TypeId(uint64_t id) : m_ID{ id } {}

		constexpr TypeId(const TypeId&) = default;
		constexpr TypeId(TypeId&&) noexcept = default;
		constexpr TypeId& operator=(const TypeId&) = default;
		constexpr TypeId& operator=(TypeId&&) noexcept = default;
		constexpr operator bool() { return IsValid();}
		constexpr bool operator==(TypeId& other) { return m_ID == other.m_ID; }
	public:
		template <typename T>
		static constexpr TypeId Create();
	public:
		const TypeInfo&			GetInfo()const;
		constexpr uint64_t		GetId()const						{ return m_ID; }
		constexpr uint64_t		GetHash()const						{ return m_ID; }
		const MemberInfo*		GetMemberInfo(size_t offset)const;
		const MemberInfo*		GetMemberInfo(std::string name)const;

		void					SetMemberData(void* instance, std::string name, void* data)const;
		void					SetMemberData(void*instance, const MemberInfo&memberInfo,void*data)const;
		template<typename T>
		T						GetMemberData(void* instance, std::string)const;
		template<typename T>
		T						GetMemberData(void* instance, const MemberInfo& memberInfo)const;

		constexpr void			SetTypeId(uint64_t typeId)			{ m_ID = typeId; }

		constexpr bool			IsValid()const						{ return m_ID; }

		template<typename...T>
		constexpr bool IsOneOf()const;
	private:
		uint64_t m_ID{};
	};
	//描述父类信息
	struct BaseClassInfo
	{
		TypeId BaseId{};
		/*
		父类相对与子类的偏移
		 */
		size_t ClassOffset{};

		template <typename Parent, typename Child>
		static constexpr BaseClassInfo Create();
	};
	/*
	类型信息
	*/
	struct TypeInfo final
	{
		std::string					Name{ };

		uint32_t					Size{ };

		uint32_t					Align{ };

		std::vector<MemberInfo>		Members{ };			//成员变量(可无)

		std::vector<FunctionId>		MemberFunctions{ };	//成员函数(可无)

		std::vector<BaseClassInfo>	BaseClasses{ };		//基类(可无)

		std::vector<TypeId>			ChildClasses{ };	//派生类(可无)

		const void* VTable{ };

		template <typename T>
		static TypeInfo Create();

		FunctionId GetFunctionId(std::string name) const;
		const MemberInfo* GetMemberInfo(std::string name) const;
		
		/*
		用于在给定地址就地构造类型的函数指针。
		@addressForConstruct 用于构造的地址
		 */
		void (*Constructor)			(void*addressForConstruct) {};

		/*
		 用于使用复制构造函数在给定地址就地构造类型的函数指针。
		 @to		用于构造的地址
		 @instance	可复制类型实例的地址
		 */
		void (*CopyConstructor)		(void*to, const void*instance) {};

		/*
		 用于使用移动构造函数在给定地址就地构造类型的函数指针。
		 @to		用于构造的地址
		 @instance  可移动类型实例的地址
		 */
		void (*MoveConstructor)		(void*to, void*instance) {};

		/*
		 用于销毁给定地址处类型的函数指针
		 @instance 用于销毁的地址
		 */
		void (*Destructor)			(void* instance) {};

		/*
		交换数据
		*/
		void (*Swap)				(void*, void*) {};

		using RapidJsonAllocator = RAPIDJSON_DEFAULT_ALLOCATOR;
		/*
		 用于将给定类型的实例以 json 格式序列化到流中的函数指针
		 */
		void (*JSonSerializer)		(rapidjson::Value&, const void*, RapidJsonAllocator&) {};
		/*
		使用 json 格式的流初始化类型实例的函数指针
		 */
		void (*JSonDeserializer)	(rapidjson::Value&, void*) {};
		/*
		  将给定类型的实例以二进制格式序列化到流中的函数指针
		 */
		void (*BinarySerializer)	(std::ostream&, const void*) {};
		/*
		 将给定类型的实例以二进制格式序列化到流中的函数指针
		 */
		void (*BinaryDeserializer)	(std::istream&, void*) {};
	};
}
