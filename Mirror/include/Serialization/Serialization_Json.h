#pragma once
#include"../../ThirdPart/rapidjson/include/rapidjson/rapidjson.h"
#include"../../ThirdPart/rapidjson/include/rapidjson/document.h"
#include"../../ThirdPart/rapidjson/include/rapidjson/istreamwrapper.h"
#include"../../ThirdPart/rapidjson/include/rapidjson/ostreamwrapper.h"
#include"../../ThirdPart/rapidjson/include/rapidjson/prettywriter.h"
#include"../Impl/Impl_Internal.h"
#include <string>
#include <vector>
#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <memory>
#include <optional>
#include <utility>
namespace mirror::Serialization
{
	template <typename T>
	constexpr void FillTypeInfoJSon(TypeInfo& info);
}

namespace mirror::Serialization
{
	using RapidJsonAllocator = RAPIDJSON_DEFAULT_ALLOCATOR;

	//特化该模板来自定义序列化
	template <typename T>
	struct JSonSerializer;

	template <typename T>
	concept CustomJSonSerializer = requires(T t, rapidjson::Value jsonVal, RapidJsonAllocator allocator)
	{
		JSonSerializer<T>::Serialize(jsonVal, t, allocator);
		JSonSerializer<T>::Deserialize(jsonVal, t);
	};
}
/*
Tips:
1、建议从以下两个函数开始了解整个序列化系统
	template <typename T>
	void SerializeJSonDefault(rapidjson::Value& jsonVal, const T& value, RapidJsonAllocator& allocator);
	template <typename T>
	void DeserializeJSonDefault(rapidjson::Value& jsonVal, T& value);
2、整个序列化系统呈树状结构一层层的向下调用特化结构体(JSonSerializer)中的序列化函数，所以如果想要自定义序列化只需实现JSonSerializer中的序列化函数即可

*/

namespace mirror::Serialization
{
	/*
	以json格式序列化/反序列化data到I/O流,需要提供对应的TypeId
	*/
	void SerializeJSon(std::ostream& stream, const void* data, TypeId type);
	void DeserializeJSon(std::istream& stream, void* data, TypeId type);

	/*
	以json格式序列化/反序列化value到I/O流,内部调用
		void SerializeJSon(std::ostream& stream, const void* data, TypeId type);
		void DeserializeJSon(std::istream& stream, void* data, TypeId type);
	*/
	template <typename T>
	void SerializeJSon(std::ostream& stream, const T& value);
	template <typename T>
	void DeserializeJSon(std::istream& stream, T& value);
	/*
	序列化/反序列化为json格式，调用自定义序列化函数，没用则调用默认序列化函数(SerializeJSonDefault/DeserializeJSonDefault)
	*/
	template <typename T>
	void SerializeJSon(rapidjson::Value& jsonVal, const T& value, RapidJsonAllocator& allocator);
	template <typename T>
	void DeserializeJSon(rapidjson::Value& jsonVal, T& value);


	void SerializeJSonDefault(rapidjson::Value& jsonVal, const void* data, TypeId type, RapidJsonAllocator& allocator);
	void DeserializeJSonDefault(rapidjson::Value& jsonVal, void* data, TypeId type);
	/*
	序列化/反序列化data,内部调用
		void SerializeJSonDefault(rapidjson::Value& jsonVal, const void* data, TypeId type, RapidJsonAllocator& allocator);
		void DeserializeJSonDefault(rapidjson::Value& jsonVal, void* data, TypeId type);
	*/
	template <typename T>
	void SerializeJSonDefault(rapidjson::Value& jsonVal, const T& value, RapidJsonAllocator& allocator);
	template <typename T>
	void DeserializeJSonDefault(rapidjson::Value& jsonVal, T& value);


	/** FLOAT */
	template <>
	struct JSonSerializer<float>
	{
		static void Serialize(rapidjson::Value& jsonVal, const float& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, float& value);
	};

	/** DOUBLE */
	template <>
	struct JSonSerializer<double>
	{
		static void Serialize(rapidjson::Value& jsonVal, const double& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, double& value);
	};

	/** INT */
	template <>
	struct JSonSerializer<int>
	{
		static void Serialize(rapidjson::Value& jsonVal, const int& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, int& value);
	};

	/** INT 64 */
	template <>
	struct JSonSerializer<int64_t>
	{
		static void Serialize(rapidjson::Value& jsonVal, const int64_t& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, int64_t& value);
	};

	/** UINT */
	template <>
	struct JSonSerializer<unsigned int>
	{
		static void Serialize(rapidjson::Value& jsonVal, const unsigned int& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, unsigned int& value);
	};

	/** UINT 64 */
	template <>
	struct JSonSerializer<uint64_t>
	{
		static void Serialize(rapidjson::Value& jsonVal, const uint64_t& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, uint64_t& value);
	};

	/** BOOL */
	template <>
	struct JSonSerializer<bool>
	{
		static void Serialize(rapidjson::Value& jsonVal, const bool& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, bool& value);
	};

	/** STRING */
	template <>
	struct JSonSerializer<char*>
	{
		static void Serialize(rapidjson::Value& jsonVal, const char*& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, const char*& value);
	};

	/** ARRAY */
	template <typename T>
	struct JSonSerializer<T[]>
	{
		static void Serialize(rapidjson::Value& jsonVal, const T value[], RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, T value[]);
	};

	template <>
	struct JSonSerializer<TypeId>
	{
		static void Serialize(rapidjson::Value& jsonVal, const TypeId& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, TypeId& value);
	};

	template <>
	struct JSonSerializer<VariableId>
	{
		static void Serialize(rapidjson::Value& jsonVal, const VariableId& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, VariableId& value);
	};

	template <>
	struct JSonSerializer<FunctionId>
	{
		static void Serialize(rapidjson::Value& jsonVal, const FunctionId& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, FunctionId& value);
	};

#ifdef _STRING_
	/** STRING */
	template <typename Elem, typename Traits, typename Alloc>
	struct JSonSerializer<std::basic_string<Elem, Traits, Alloc>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::basic_string<Elem, Traits, Alloc>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::basic_string<Elem, Traits, Alloc>& value);
	};
#endif

#ifdef _VECTOR_
	/** VECTOR */
	template <typename T, typename Alloc>
	struct JSonSerializer<std::vector<T, Alloc>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::vector<T, Alloc>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::vector<T, Alloc>& value);
	};
#endif

#ifdef _ARRAY_
	/** ARRAY */
	template <typename T, size_t Size>
	struct JSonSerializer<std::array<T, Size>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::array<T, Size>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::array<T, Size>& value);
	};
#endif

#ifdef _DEQUE_
	/** DEQUE*/
	template <typename T, typename Alloc>
	struct JSonSerializer<std::deque<T, Alloc>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::deque<T, Alloc>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::deque<T, Alloc>& value);
	};
#endif

#ifdef _FORWARD_LIST_
	/** FORWARD LIST */
	template <typename T, typename Alloc>
	struct JSonSerializer<std::forward_list<T, Alloc>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::forward_list<T, Alloc>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::forward_list<T, Alloc>& value);
	};
#endif

#ifdef _LIST_
	/** LIST */
	template <typename T, typename Alloc>
	struct JSonSerializer<std::list<T, Alloc>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::list<T, Alloc>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::list<T, Alloc>& value);
	};
#endif

#ifdef _SET_
	/** UNORDERED SET */
	template <typename T, typename Hasher, typename Keyeq, typename Alloc>
	struct JSonSerializer<std::unordered_set<T, Hasher, Keyeq, Alloc>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::unordered_set<T, Hasher, Keyeq, Alloc>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::unordered_set<T, Hasher, Keyeq, Alloc>& value);
	};

	/** UNORDERED MULTI SET */
	template <typename T, typename Hasher, typename Keyeq, typename Alloc>
	struct JSonSerializer<std::unordered_multiset<T, Hasher, Keyeq, Alloc>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::unordered_multiset<T, Hasher, Keyeq, Alloc>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::unordered_multiset<T, Hasher, Keyeq, Alloc>& value);
	};
#endif

#ifdef _MAP_
	/** MAP */
	template <typename Key, typename Value, typename P, typename Alloc>
	struct JSonSerializer<std::map<Key, Value, P, Alloc>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::map<Key, Value, P, Alloc>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::map<Key, Value, P, Alloc>& value);
	};

	/** MULTI MAP */
	template <typename Key, typename Value, typename P, typename Alloc>
	struct JSonSerializer<std::multimap<Key, Value, P, Alloc>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::multimap<Key, Value, P, Alloc>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::multimap<Key, Value, P, Alloc>& value);
	};
#endif

#ifdef _UNORDERED_MAP_
	/** UNORDERED MAP */
	template <typename Key, typename Value, typename Hasher, typename Keyeq, typename Alloc>
	struct JSonSerializer<std::unordered_map<Key, Value, Hasher, Keyeq, Alloc>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::unordered_map<Key, Value, Hasher, Keyeq, Alloc>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::unordered_map<Key, Value, Hasher, Keyeq, Alloc>& value);
	};

	/** UNORDERED MULTI MAP */
	template <typename Key, typename Value, typename Hasher, typename Keyeq, typename Alloc>
	struct JSonSerializer<std::unordered_multimap<Key, Value, Hasher, Keyeq, Alloc>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::unordered_multimap<Key, Value, Hasher, Keyeq, Alloc>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::unordered_multimap<Key, Value, Hasher, Keyeq, Alloc>& value);
	};
#endif

#ifdef _MEMORY_
	/** UNIQUE PTR */
	template <typename T, typename Delete>
	struct JSonSerializer<std::unique_ptr<T, Delete>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::unique_ptr<T, Delete>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::unique_ptr<T, Delete>& value);
	};
#endif

#ifdef _OPTIONAL_
	/** OPTIONAL */
	template <typename T>
	struct JSonSerializer<std::optional<T>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::optional<T>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::optional<T>& value);
	};
#endif

#ifdef _UTILITY_
	/** PAIR */
	template <typename T1, typename T2>
	struct JSonSerializer<std::pair<T1, T2>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::pair<T1, T2>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::pair<T1, T2>& value);
	};

#endif 
#ifdef _TUPLE_
	/** TUPLE */
	template <typename ... Ts>
	struct JSonSerializer<std::tuple<Ts...>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::tuple<Ts...>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::tuple<Ts...>& value);
	};
#endif
}