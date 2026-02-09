#pragma once
#include<type_traits>
#include <cstdint>       
namespace mirror
{
    struct TypeConverter
    {
    private:
        static bool IsNumericType(const VariableId& from)
        {
            return from.GetTypeId().IsOneOf<int, float, double,
                uint8_t, uint16_t, uint32_t, uint64_t,
                int8_t, int16_t, int32_t, int64_t>();
        }
    public:
        template<typename To>
        static To* Convert(const VariableId& fromId, void* fromData)
        {
            return nullptr;
        } 
    };

    template<typename To>
    inline auto& Convert(TypeTuple& typeTuple, uint32_t varIdIndex)
    {
        const VariableId& fromVarId = typeTuple.GetVariableIds()[varIdIndex];
        constexpr const VariableId& toVarId = VariableId::Create<To>();

        const bool SameType         = fromVarId.GetTypeId() == toVarId.GetTypeId();
        const bool SameArraySize    = fromVarId.GetArraySize() == toVarId.GetArraySize();
        const bool fromVarIdIsArray = fromVarId.IsArray();
        const bool ConstCorrect     = !(fromVarId.IsConst() && !toVarId.IsConst());

        const uint32_t fromVarIdPointAmount = fromVarId.GetPointerAmount();
        constexpr const uint32_t toVarIdPointAmount   = toVarId.GetPointerAmount();

        constexpr const bool toVarIdIsPoint = toVarId.IsPointer();
        void* data = typeTuple.GetVoid(varIdIndex);
        /*
        1、类型相同
        2、数组长度(非数组都为1)相同或者from是数组、to是指针
        3、相同的const限定
        4、指针级数相同
        满足以上4点直接转换返回
        */
        if (SameType && (SameArraySize || (fromVarIdIsArray && toVarIdIsPoint)) && ConstCorrect&&(fromVarIdPointAmount== toVarIdPointAmount))
        {
            return *static_cast<To*>(data);
        }
        
        if (fromVarIdPointAmount)
        {
            // 场景1：TypeTuple(from)存储的是指针Ty1*，(to)也是指针Ty2*
            if constexpr (toVarIdPointAmount)
            {
                // 返回To&(Ty2*&),Get内部会进行类型判断断言所以没有进行类型判断
                return typeTuple.Get<To>(varIdIndex);
            }
            // 场景2：TypeTuple(from)存储的是指针Ty1*，(to)不是指针 Ty2
            else
            {
                // 取出存储的指针，解引用后返回To&（Ty1&）
                using PtrType = std::remove_reference_t<To>*;
                PtrType* pStoragePtr = &typeTuple.Get<PtrType>(varIdIndex);
                assert(pStoragePtr && *pStoragePtr);
                if  (SameType)
                {
                    return **pStoragePtr; // 解引用为Ty1&
                }
                else
                {
                    //类型不相同尝试进行转换(int->float、char->string.....)返回的值是拷贝了原值的局部静态变量
                    auto* ret = TypeConverter::Convert<To>(fromVarId, data);
                    assert(ret && "Parameter convert failed!");
                    return *ret;
                }
            }
        }
        // 场景3：TypeTuple(from)存储的非指针Ty1，(to)是指针Ty2*
        else if constexpr (toVarIdPointAmount)
        {
            assert((fromVarIdIsArray && toVarIdIsPoint) && "A pointer value was expected for the target, but a non-pointer value was passed!");
            //在传递数组的情况下会将数组的第一个元素的地址返回
            using ValueType = std::remove_pointer_t<To>;
            static_assert(std::is_same_v<decltype(typeTuple.Get<ValueType>(varIdIndex)), ValueType&>,"Storage type must match the pointer type of To!");
            ValueType& storageVal = typeTuple.Get<ValueType>(varIdIndex);
            To pVal = &storageVal;
            return *std::addressof(pVal);
        }
        // 场景4：TypeTuple(from)存储的非指针Ty1，(to)也非指针Ty2
        else
        {
            if  (SameType)
            {
                return typeTuple.Get<To>(varIdIndex);
            }
            else
            {
                //类型不相同尝试进行转换(int->float、char->string.....)返回的值是拷贝了原值的局部静态变量
                auto* ret = TypeConverter::Convert<To>(fromVarId, data);
                assert(ret && "Parameter convert failed!");
                return *ret;
            }
        }
    }

    template<>
    inline std::string* TypeConverter::Convert<std::string>(const VariableId& from, void* fromData)
    {
        if (from.GetTypeId() != TypeId::Create<char>())return nullptr;
        static std::string ret = "";
        ret = std::string(static_cast<const char*>(fromData));
        return &ret;
    }

    template<>
    inline float* TypeConverter::Convert<float>(const VariableId& from, void* fromData)
    {
        if (!IsNumericType(from)) return nullptr;
        static float target_val = 0.0f; 

        if (from.GetTypeId().IsOneOf<int>()) target_val = static_cast<float>(*static_cast<int*>(fromData));
        else if (from.GetTypeId().IsOneOf<double>()) target_val = static_cast<float>(*static_cast<double*>(fromData));
        else if (from.GetTypeId().IsOneOf<int8_t>()) target_val = static_cast<float>(*static_cast<int8_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int16_t>()) target_val = static_cast<float>(*static_cast<int16_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int32_t>()) target_val = static_cast<float>(*static_cast<int32_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int64_t>()) target_val = static_cast<float>(*static_cast<int64_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint8_t>()) target_val = static_cast<float>(*static_cast<uint8_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint16_t>()) target_val = static_cast<float>(*static_cast<uint16_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint32_t>()) target_val = static_cast<float>(*static_cast<uint32_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint64_t>()) target_val = static_cast<float>(*static_cast<uint64_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<float>()) target_val = *static_cast<float*>(fromData);

        return &target_val;
    }
    template<>
    inline double* TypeConverter::Convert<double>(const VariableId& from, void* fromData)
    {
        if (!IsNumericType(from)) return nullptr;
        static double target_val = 0.0; 

        if (from.GetTypeId().IsOneOf<int>()) target_val = static_cast<double>(*static_cast<int*>(fromData));
        else if (from.GetTypeId().IsOneOf<float>()) target_val = static_cast<double>(*static_cast<float*>(fromData));
        else if (from.GetTypeId().IsOneOf<int8_t>()) target_val = static_cast<double>(*static_cast<int8_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int16_t>()) target_val = static_cast<double>(*static_cast<int16_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int32_t>()) target_val = static_cast<double>(*static_cast<int32_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int64_t>()) target_val = static_cast<double>(*static_cast<int64_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint8_t>()) target_val = static_cast<double>(*static_cast<uint8_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint16_t>()) target_val = static_cast<double>(*static_cast<uint16_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint32_t>()) target_val = static_cast<double>(*static_cast<uint32_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint64_t>()) target_val = static_cast<double>(*static_cast<uint64_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<double>()) target_val = *static_cast<double*>(fromData);

        return &target_val;
    }

    template<>
    inline int32_t* TypeConverter::Convert<int32_t>(const VariableId& from, void* fromData)
    {
        if (!IsNumericType(from)) return nullptr;
        static int32_t target_val = 0; 

        if (from.GetTypeId().IsOneOf<int>()) target_val = static_cast<int32_t>(*static_cast<int*>(fromData));
        else if (from.GetTypeId().IsOneOf<float>()) target_val = static_cast<int32_t>(*static_cast<float*>(fromData));
        else if (from.GetTypeId().IsOneOf<double>()) target_val = static_cast<int32_t>(*static_cast<double*>(fromData));
        else if (from.GetTypeId().IsOneOf<int8_t>()) target_val = static_cast<int32_t>(*static_cast<int8_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int16_t>()) target_val = static_cast<int32_t>(*static_cast<int16_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int64_t>()) target_val = static_cast<int32_t>(*static_cast<int64_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint8_t>()) target_val = static_cast<int32_t>(*static_cast<uint8_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint16_t>()) target_val = static_cast<int32_t>(*static_cast<uint16_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint32_t>()) target_val = static_cast<int32_t>(*static_cast<uint32_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint64_t>()) target_val = static_cast<int32_t>(*static_cast<uint64_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int32_t>()) target_val = *static_cast<int32_t*>(fromData);

        return &target_val; 
    }

    template<>
    inline  int8_t* TypeConverter::Convert<int8_t>(const VariableId& from, void* fromData)
    {
        if (!IsNumericType(from)) return nullptr;
        static int8_t target_val = 0; 

        if (from.GetTypeId().IsOneOf<int>()) target_val = static_cast<int8_t>(*static_cast<int*>(fromData));
        else if (from.GetTypeId().IsOneOf<float>()) target_val = static_cast<int8_t>(*static_cast<float*>(fromData));
        else if (from.GetTypeId().IsOneOf<double>()) target_val = static_cast<int8_t>(*static_cast<double*>(fromData));
        else if (from.GetTypeId().IsOneOf<int16_t>()) target_val = static_cast<int8_t>(*static_cast<int16_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int32_t>()) target_val = static_cast<int8_t>(*static_cast<int32_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int64_t>()) target_val = static_cast<int8_t>(*static_cast<int64_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint8_t>()) target_val = static_cast<int8_t>(*static_cast<uint8_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint16_t>()) target_val = static_cast<int8_t>(*static_cast<uint16_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint32_t>()) target_val = static_cast<int8_t>(*static_cast<uint32_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint64_t>()) target_val = static_cast<int8_t>(*static_cast<uint64_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int8_t>()) target_val = *static_cast<int8_t*>(fromData);

        return &target_val; 
    }

    template<>
    inline  int16_t* TypeConverter::Convert<int16_t>(const VariableId& from, void* fromData)
    {
        if (!IsNumericType(from)) return nullptr;
        static int16_t target_val = 0; 

        if (from.GetTypeId().IsOneOf<int>()) target_val = static_cast<int16_t>(*static_cast<int*>(fromData));
        else if (from.GetTypeId().IsOneOf<float>()) target_val = static_cast<int16_t>(*static_cast<float*>(fromData));
        else if (from.GetTypeId().IsOneOf<double>()) target_val = static_cast<int16_t>(*static_cast<double*>(fromData));
        else if (from.GetTypeId().IsOneOf<int8_t>()) target_val = static_cast<int16_t>(*static_cast<int8_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int32_t>()) target_val = static_cast<int16_t>(*static_cast<int32_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int64_t>()) target_val = static_cast<int16_t>(*static_cast<int64_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint8_t>()) target_val = static_cast<int16_t>(*static_cast<uint8_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint16_t>()) target_val = static_cast<int16_t>(*static_cast<uint16_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint32_t>()) target_val = static_cast<int16_t>(*static_cast<uint32_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint64_t>()) target_val = static_cast<int16_t>(*static_cast<uint64_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int16_t>()) target_val = *static_cast<int16_t*>(fromData);

        return &target_val; 
    }

    template<>
    inline   int64_t* TypeConverter::Convert<int64_t>(const VariableId& from, void* fromData)
    {
        if (!IsNumericType(from)) return nullptr;
        static int64_t target_val = 0; 

        if (from.GetTypeId().IsOneOf<int>()) target_val = static_cast<int64_t>(*static_cast<int*>(fromData));
        else if (from.GetTypeId().IsOneOf<float>()) target_val = static_cast<int64_t>(*static_cast<float*>(fromData));
        else if (from.GetTypeId().IsOneOf<double>()) target_val = static_cast<int64_t>(*static_cast<double*>(fromData));
        else if (from.GetTypeId().IsOneOf<int8_t>()) target_val = static_cast<int64_t>(*static_cast<int8_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int16_t>()) target_val = static_cast<int64_t>(*static_cast<int16_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int32_t>()) target_val = static_cast<int64_t>(*static_cast<int32_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint8_t>()) target_val = static_cast<int64_t>(*static_cast<uint8_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint16_t>()) target_val = static_cast<int64_t>(*static_cast<uint16_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint32_t>()) target_val = static_cast<int64_t>(*static_cast<uint32_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint64_t>()) target_val = static_cast<int64_t>(*static_cast<uint64_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int64_t>()) target_val = *static_cast<int64_t*>(fromData);

        return &target_val; 
    }

    template<>
    inline    uint8_t* TypeConverter::Convert<uint8_t>(const VariableId& from, void* fromData)
    {
        if (!IsNumericType(from)) return nullptr;
        static uint8_t target_val = 0;

        if (from.GetTypeId().IsOneOf<int>()) target_val = static_cast<uint8_t>(*static_cast<int*>(fromData));
        else if (from.GetTypeId().IsOneOf<float>()) target_val = static_cast<uint8_t>(*static_cast<float*>(fromData));
        else if (from.GetTypeId().IsOneOf<double>()) target_val = static_cast<uint8_t>(*static_cast<double*>(fromData));
        else if (from.GetTypeId().IsOneOf<int8_t>()) target_val = static_cast<uint8_t>(*static_cast<int8_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int16_t>()) target_val = static_cast<uint8_t>(*static_cast<int16_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int32_t>()) target_val = static_cast<uint8_t>(*static_cast<int32_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int64_t>()) target_val = static_cast<uint8_t>(*static_cast<int64_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint16_t>()) target_val = static_cast<uint8_t>(*static_cast<uint16_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint32_t>()) target_val = static_cast<uint8_t>(*static_cast<uint32_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint64_t>()) target_val = static_cast<uint8_t>(*static_cast<uint64_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint8_t>()) target_val = *static_cast<uint8_t*>(fromData);

        return &target_val; 
    }

    template<>
    inline   uint16_t* TypeConverter::Convert<uint16_t>(const VariableId& from, void* fromData)
    {
        if (!IsNumericType(from)) return nullptr;
        static uint16_t target_val = 0; 

        if (from.GetTypeId().IsOneOf<int>()) target_val = static_cast<uint16_t>(*static_cast<int*>(fromData));
        else if (from.GetTypeId().IsOneOf<float>()) target_val = static_cast<uint16_t>(*static_cast<float*>(fromData));
        else if (from.GetTypeId().IsOneOf<double>()) target_val = static_cast<uint16_t>(*static_cast<double*>(fromData));
        else if (from.GetTypeId().IsOneOf<int8_t>()) target_val = static_cast<uint16_t>(*static_cast<int8_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int16_t>()) target_val = static_cast<uint16_t>(*static_cast<int16_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int32_t>()) target_val = static_cast<uint16_t>(*static_cast<int32_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int64_t>()) target_val = static_cast<uint16_t>(*static_cast<int64_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint8_t>()) target_val = static_cast<uint16_t>(*static_cast<uint8_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint32_t>()) target_val = static_cast<uint16_t>(*static_cast<uint32_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint64_t>()) target_val = static_cast<uint16_t>(*static_cast<uint64_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint16_t>()) target_val = *static_cast<uint16_t*>(fromData);

        return &target_val;
    }

    template<>
    inline  uint32_t* TypeConverter::Convert<uint32_t>(const VariableId& from, void* fromData)
    {
        if (!IsNumericType(from)) return nullptr;
        static uint32_t target_val = 0; 

        if (from.GetTypeId().IsOneOf<int>()) target_val = static_cast<uint32_t>(*static_cast<int*>(fromData));
        else if (from.GetTypeId().IsOneOf<float>()) target_val = static_cast<uint32_t>(*static_cast<float*>(fromData));
        else if (from.GetTypeId().IsOneOf<double>()) target_val = static_cast<uint32_t>(*static_cast<double*>(fromData));
        else if (from.GetTypeId().IsOneOf<int8_t>()) target_val = static_cast<uint32_t>(*static_cast<int8_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int16_t>()) target_val = static_cast<uint32_t>(*static_cast<int16_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int32_t>()) target_val = static_cast<uint32_t>(*static_cast<int32_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int64_t>()) target_val = static_cast<uint32_t>(*static_cast<int64_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint8_t>()) target_val = static_cast<uint32_t>(*static_cast<uint8_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint16_t>()) target_val = static_cast<uint32_t>(*static_cast<uint16_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint64_t>()) target_val = static_cast<uint32_t>(*static_cast<uint64_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint32_t>()) target_val = *static_cast<uint32_t*>(fromData);

        return &target_val; 
    }

    template<>
    inline   uint64_t* TypeConverter::Convert<uint64_t>(const VariableId& from, void* fromData)
    {
        if (!IsNumericType(from)) return nullptr;
        static uint64_t target_val = 0;

        if (from.GetTypeId().IsOneOf<int>()) target_val = static_cast<uint64_t>(*static_cast<int*>(fromData));
        else if (from.GetTypeId().IsOneOf<float>()) target_val = static_cast<uint64_t>(*static_cast<float*>(fromData));
        else if (from.GetTypeId().IsOneOf<double>()) target_val = static_cast<uint64_t>(*static_cast<double*>(fromData));
        else if (from.GetTypeId().IsOneOf<int8_t>()) target_val = static_cast<uint64_t>(*static_cast<int8_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int16_t>()) target_val = static_cast<uint64_t>(*static_cast<int16_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int32_t>()) target_val = static_cast<uint64_t>(*static_cast<int32_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<int64_t>()) target_val = static_cast<uint64_t>(*static_cast<int64_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint8_t>()) target_val = static_cast<uint64_t>(*static_cast<uint8_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint16_t>()) target_val = static_cast<uint64_t>(*static_cast<uint16_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint32_t>()) target_val = static_cast<uint64_t>(*static_cast<uint32_t*>(fromData));
        else if (from.GetTypeId().IsOneOf<uint64_t>()) target_val = *static_cast<uint64_t*>(fromData);

        return &target_val; 
    }

} // namespace mirror
