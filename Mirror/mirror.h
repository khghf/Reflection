#pragma once
#ifndef MIRROR_H
#define MIRROR_H

#include"include/Serialization/Serialization_Json_Impl.h"
#include"include/Serialization/Serialization_Binary_Impl.h"
#include"include/Impl/TypeInfo_Impl.h"
#include"include/Impl/MemberInfo_Impl.h"
#include"include/Impl/FunctionInfo_Impl.h"
#include"include/Impl/Dependency.h"

#define _CONCAT_(a,b) a ## b
/*创建一个内联静态变量以在main前注册给定		类型*/
#define _REGISTER_TYPE_INTERNAL(TYPE, ID)										inline static mirror::AutoRegisterType<TYPE> _CONCAT_(RegisterType_, ID) {};
/*创建一个内联静态变量以在main前注册给定		成员变量*/
#define _REGISTER_MEMBER_INTERNAL(TYPE, MEMBER, PROPERTIES, ID)					inline static mirror::AutoRegisterMember _CONCAT_(RegisterMember_, ID){ static_cast<TYPE*>(nullptr),static_cast<decltype(TYPE::MEMBER)*>(nullptr),mirror::VariableId::Create<decltype(TYPE::MEMBER)>(), #MEMBER, offsetof(TYPE, MEMBER), sizeof(decltype(TYPE::MEMBER)), alignof(decltype(TYPE::MEMBER)), PROPERTIES};
/*创建一个内联静态变量以在main前注册给定		私有成员变量*/
#define _REGISTER_PRIVATE_MEMBER_INTERNAL(TYPE, MEMBER, PROPERTIES, ID)			template struct mirror::RegisterMemberType<&TYPE::MEMBER, ID>; \
																				inline static mirror::AutoMemberVariableDataSetter _CONCAT_(RegisterMember_, ID){ID, #MEMBER, PROPERTIES};
/*创建一个内联静态变量以在main前注册给定		函数.*/
#define _REGISTER_FUNCTION_INTERNAL(FUNCTION, PROPS,ID)						    inline static mirror::AutoRegisterFunction _CONCAT_(RegisterFunction_, ID) {FUNCTION, #FUNCTION, PROPS};
/*创建一个内联静态变量以在main前注册给定		成员函数*/
#define _REGISTER_MEMBER_FUNCTION_INTERNAL(CLASS, FUNCTION, PROPS, ID)			inline static mirror::AutoRegisterMemberFunction _CONCAT_(RegisterMemberFunction_, ID) {&CLASS::FUNCTION, #FUNCTION, PROPS};
/*创建一个内联静态变量以在main前注册给定		私有成员函数*/
#define _REGISTER_PRIVATE_MEMBER_FUNCTION_INTERNAL(CLASS, FUNCTION, PROPS, ID)	template struct mirror::RegisterMethodType<&CLASS::FUNCTION, ID>; \
																				inline static mirror::AutoMemberFunctionDataSetter _CONCAT_(RegisterMember_, ID){ID, #FUNCTION, PROPS};
/*创建一个内联静态变量以在main前为给定类型填充派生类信息*/
#define _REGISTER_CHILD_INTERNAL(BASE, CHILD, ID)								inline static mirror::AutoRegisterChildOnce<BASE, CHILD> _CONCAT_(RegisterChild_, ID) {};

#define _REGISTER_ENUM_ITEM_INTERNAL(TYPE,NAME,VALUE,ID)                        inline static mirror::AutoRegisterEnumItem<TYPE> _CONCAT(RegisterEnumItem,ID) {NAME,VALUE};


#define _EXTRACT_ENUM_TYPE(ENUM_VALUE) \
    std::decay_t<decltype(ENUM_VALUE)>

#define _EXTRACT_ENUM_ITEM_NAME(ENUM_VALUE) \
    []() -> std::string_view \
    { \
        std::string_view full = #ENUM_VALUE; \
        size_t pos = full.find_last_of(':'); \
        if (pos != std::string_view::npos) { \
            return full.substr(pos + 1); \
        } \
        return full; \
    }()

#define _EXTRACT_ENUM_VALUE(ENUM_VALUE) \
    static_cast<uint64_t>(ENUM_VALUE)


#define REGISTER_TYPE(TYPE)										_REGISTER_TYPE_INTERNAL(TYPE, __COUNTER__)

#define REGISTER_CHILD(BASE, CHILD)								_REGISTER_CHILD_INTERNAL(BASE, CHILD, __COUNTER__)

#define REGISTER_MEMBER(TYPE, MEMBER)							_REGISTER_MEMBER_INTERNAL(TYPE, MEMBER,mirror::DefaultMemberProperties, __COUNTER__)

#define REGISTER_PRIVATE_MEMBER(TYPE, MEMBER)					_REGISTER_PRIVATE_MEMBER_INTERNAL(TYPE, MEMBER,mirror::DefaultMemberProperties, __COUNTER__)

#define REGISTER_MEMBER_PROP(TYPE, MEMBER, PROPERTIES)			_REGISTER_MEMBER_INTERNAL(TYPE, MEMBER, PROPERTIES, __COUNTER__)

#define REGISTER_PRIVATE_MEMBER_PROP(TYPE, MEMBER, PROPERTIES)	_REGISTER_PRIVATE_MEMBER_INTERNAL(TYPE, MEMBER, PROPERTIES, __COUNTER__)

#define REGISTER_MEMBER_FUNCTION(CLASS, FUNCTION)				_REGISTER_MEMBER_FUNCTION_INTERNAL(CLASS, FUNCTION,mirror::DefaultFunctionProperties, __COUNTER__);

#define REGISTER_PRIVATE_MEMBER_FUNCTION(CLASS, FUNCTION)		_REGISTER_PRIVATE_MEMBER_FUNCTION_INTERNAL(CLASS, FUNCTION,mirror::DefaultFunctionProperties, __COUNTER__);

#define REGISTER_MEMBER_FUNCTION_PROP(CLASS, FUNCTION, PROPS)	_REGISTER_MEMBER_FUNCTION_INTERNAL(CLASS, FUNCTION, PROPS, __COUNTER__);

#define REGISTER_FUNCTION(FUNCTION)								_REGISTER_FUNCTION_INTERNAL(FUNCTION, mirror::DefaultFunctionProperties,__COUNTER__);

#define REGISTER_FUNCTION_PROP(FUNCTION, PROPS)					_REGISTER_FUNCTION_INTERNAL(FUNCTION,PROPS, __COUNTER__ );

#define REGISTER_ENUM(TYPE)                                     REGISTER_TYPE(TYPE);

#define REGISTER_ENUM_ITEM(TYPE_NAME)                           _REGISTER_ENUM_ITEM_INTERNAL(_EXTRACT_ENUM_TYPE(TYPE_NAME),_EXTRACT_ENUM_ITEM_NAME(TYPE_NAME),_EXTRACT_ENUM_VALUE(TYPE_NAME),__COUNTER__);


namespace mirror
{
	template<typename Type>
	TypeId GetTypeId()
	{
		return TypeId::Create<Type>();
	}
	template<typename Class>
	FunctionId GetMethod(std::string methodName)
	{
		return GetTypeId<Class>().GetInfo().GetFunctionId(methodName);
	}
	template<typename Class>
	const MemberInfo* GetVariable(std::string memberName)
	{
		return GetTypeId<Class>().GetInfo().GetMemberInfo(memberName);
	}

    template<typename T>
    const EnumInfo& GetEnumInfo()
    {
        return TypeId::Create<T>().GetEnumInfo();
    }

    template <typename Class>
    void Serialize(std::string_view outPath,const Class* val)
    {
        Serialization::SerializeJson(outPath, val);
    }

    template <typename Class>
    void Deserialize(const std::string_view path,Class* outVal)
    {
        Serialization::DeserializeJson(path,outVal);
    }


    template <typename Class>
    void SerializeBinary( std::string_view outPath,const Class* val)
    {
        Serialization::SerializeBinary(outPath, val);
    }

    template <typename Class>
    void DeserializeBinary(std::string_view path,Class* outVal )
    {
        Serialization::DeserializeBinary(path, outVal);
    }
}
#endif // !MIRROR
