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
//#include<glm/glm.hpp>

namespace mirror::Serialization
{
	template <typename T>
	constexpr void FillTypeInfoJson(TypeInfo& info);
}

namespace mirror::Serialization
{
	using RapidJsonAllocator = RAPIDJSON_DEFAULT_ALLOCATOR;

	//特化该模板来自定义序列化
	template <typename T>
	struct JsonSerializer;

	template <typename T>
	concept CustomJsonSerializer = requires(T*t, rapidjson::Value*jsonVal, RapidJsonAllocator*allocator)
	{
		JsonSerializer<T>::Serialize(jsonVal, t, allocator);
		JsonSerializer<T>::Deserialize(jsonVal, t);
	};
}

/*
Tips:
1、建议从以下两个函数开始了解整个序列化系统
	template <typename T>
	void SerializeJsonDefault(rapidjson::Value& jsonVal, const T* inVal, RapidJsonAllocator& allocator);
	template <typename T>
	void DeserializeJsonDefault(rapidjson::Value& jsonVal, T* outVal);
2、整个序列化系统呈树状结构一层层的向下调用特化结构体(JsonSerializer)中的序列化函数，
   所以如果想要自定义序列化只需实现JsonSerializer中的序列化函数即可
*/

namespace mirror::Serialization
{
	template<typename T>
	void SerializeJson(std::string_view outFilePath,const T*inVal);
	template <typename T>
	void SerializeJson(std::ostream* stream, const T* inVal);
	template <typename T>
	void SerializeJson(rapidjson::Value* jsonVal, const T* inVal, RapidJsonAllocator* allocator);

	/// <summary>
	/// 将值以json格式序列化到jsonVal。内部会根据虚表指针获取实际的类型然后遍历父类转发到其对应的序列化函数
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="jsonVal"></param>
	/// <param name="inVal"></param>
	/// <param name="allocator"></param>
	template <typename T>
	void SerializeJsonDefault(rapidjson::Value* jsonVal, const T* inVal, RapidJsonAllocator* allocator);
	/// <summary>
	/// 将值以json序列化到jsonVal。
	/// </summary>
	/// <param name="jsonVal"></param>
	/// <param name="inVal"></param>
	/// <param name="type"></param>
	/// <param name="allocator"></param>
	void SerializeJsonDefault(rapidjson::Value* jsonVal, const void* inVal, TypeId type, RapidJsonAllocator* allocator);


	template<typename T>
	void DeserializeJson(std::string_view inFilePath, T* outVal);
	template <typename T>
	void DeserializeJson(std::istream* stream, T* outVal);
	template <typename T>
	void DeserializeJson(rapidjson::Value* jsonVal, T* outVal);
	/// <summary>
	/// 将jsonVal反序列化到outVal。内部会根据虚表指针获取实际的类型然后遍历父类转发到其对应的反序列化函数
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="jsonVal"></param>
	/// <param name="outVal"></param>
	template <typename T>
	void DeserializeJsonDefault(rapidjson::Value* jsonVal, T* outVal);
	/// <summary>
	/// 将jsonVal反序列化到outVal。
	/// </summary>
	/// <param name="jsonVal"></param>
	/// <param name="outVal"></param>
	/// <param name="type"></param>
	void DeserializeJsonDefault(rapidjson::Value* jsonVal, void* outVal, TypeId type);










	///** TypeId */
	//template <>
	//struct JsonSerializer<TypeId>
	//{
	//	static void Serialize(rapidjson::Value* jsonVal, const TypeId* inVal, RapidJsonAllocator* allocator);
	//	static void Deserialize(rapidjson::Value* jsonVal, TypeId* outVal);
	//};

	///** VariableId */
	//template <>
	//struct JsonSerializer<VariableId>
	//{
	//	static void Serialize(rapidjson::Value* jsonVal, const VariableId* inVal, RapidJsonAllocator* allocator);
	//	static void Deserialize(rapidjson::Value* jsonVal, VariableId* outVal);
	//};

	///** FunctionId */
	//template <>
	//struct JsonSerializer<FunctionId>
	//{
	//	static void Serialize(rapidjson::Value* jsonVal, const FunctionId* inVal, RapidJsonAllocator* allocator);
	//	static void Deserialize(rapidjson::Value* jsonVal, FunctionId* outVal);
	//};

#ifdef  GLM_ENABLE
	template <>
	struct JsonSerializer<glm::vec1>
	{
		static void Serialize(rapidjson::Value* jsonVal, const glm::vec1* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, glm::vec1* outVal);
	};
	template <>
	struct JsonSerializer<glm::vec2>
	{
		static void Serialize(rapidjson::Value* jsonVal, const glm::vec2* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, glm::vec2* outVal);
	};
	template <>
	struct JsonSerializer<glm::vec3>
	{
		static void Serialize(rapidjson::Value* jsonVal, const glm::vec3* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, glm::vec3* outVal);
	};
	template <>
	struct JsonSerializer<glm::vec4>
	{
		static void Serialize(rapidjson::Value* jsonVal, const glm::vec4* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, glm::vec4* outVal);
	};

#endif //  GLM_HPP








	/** FLOAT */
	template <>
	struct JsonSerializer<float>
	{
		static void Serialize(rapidjson::Value* jsonVal, const float* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, float* outVal);
	};

	/** DOUBLE */
	template <>
	struct JsonSerializer<double>
	{
		static void Serialize(rapidjson::Value* jsonVal, const double* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, double* outVal);
	};

	/** INT 8 */
	template <>
	struct JsonSerializer<int8_t>
	{
		static void Serialize(rapidjson::Value* jsonVal, const int8_t* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, int8_t* outVal);
	};

	/** INT 16 */
	template <>
	struct JsonSerializer<int16_t>
	{
		static void Serialize(rapidjson::Value* jsonVal, const int16_t* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, int16_t* outVal);
	};

	/** INT 32 */
	template <>
	struct JsonSerializer<int32_t>
	{
		static void Serialize(rapidjson::Value* jsonVal, const int32_t* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, int32_t* outVal);
	};

	/** INT 64 */
	template <>
	struct JsonSerializer<int64_t>
	{
		static void Serialize(rapidjson::Value* jsonVal, const int64_t* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, int64_t* outVal);
	};

	/** UINT 8 */
	template <>
	struct JsonSerializer<uint8_t>
	{
		static void Serialize(rapidjson::Value* jsonVal, const uint8_t* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, uint8_t* outVal);
	};

	/** UINT 16 */
	template <>
	struct JsonSerializer<uint16_t>
	{
		static void Serialize(rapidjson::Value* jsonVal, const uint16_t* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, uint16_t* outVal);
	};

	/** UINT 32 */
	template <>
	struct JsonSerializer<uint32_t>
	{
		static void Serialize(rapidjson::Value* jsonVal, const uint32_t* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, uint32_t* outVal);
	};

	/** UINT 64 */
	template <>
	struct JsonSerializer<uint64_t>
	{
		static void Serialize(rapidjson::Value* jsonVal, const uint64_t* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, uint64_t* outVal);
	};

	/** BOOL */
	template <>
	struct JsonSerializer<bool>
	{
		static void Serialize(rapidjson::Value* jsonVal, const bool* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, bool* outVal);
	};
	/** CHAR PTR (char*) */
	template <>
	struct JsonSerializer<char*>
	{
		static void Serialize(rapidjson::Value* jsonVal, const char** inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, const char** outVal);
	};

	// ============ 复合类型特化 ============

	/** C-ARRAY */
	template <typename T>
	struct JsonSerializer<T[]>
	{
		static void Serialize(rapidjson::Value* jsonVal, const T* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, T* outVal);
	};





#ifdef _STRING_
	/** STRING */
	template <typename Elem, typename Traits, typename Alloc>
	struct JsonSerializer<std::basic_string<Elem, Traits, Alloc>>
	{
		static void Serialize(rapidjson::Value* jsonVal, const std::basic_string<Elem, Traits, Alloc>* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, std::basic_string<Elem, Traits, Alloc>* outVal);
	};
#endif

#ifdef _VECTOR_
	/** VECTOR */
	template <typename T, typename Alloc>
	struct JsonSerializer<std::vector<T, Alloc>>
	{
		static void Serialize(rapidjson::Value* jsonVal, const std::vector<T, Alloc>* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, std::vector<T, Alloc>* outVal);
	};
#endif

#ifdef _ARRAY_
	/** ARRAY */
	template <typename T, size_t Size>
	struct JsonSerializer<std::array<T, Size>>
	{
		static void Serialize(rapidjson::Value* jsonVal, const std::array<T, Size>* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, std::array<T, Size>* outVal);
	};
#endif

#ifdef _DEQUE_
	/** DEQUE */
	template <typename T, typename Alloc>
	struct JsonSerializer<std::deque<T, Alloc>>
	{
		static void Serialize(rapidjson::Value* jsonVal, const std::deque<T, Alloc>* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, std::deque<T, Alloc>* outVal);
	};
#endif

#ifdef _FORWARD_LIST_
	/** FORWARD LIST */
	template <typename T, typename Alloc>
	struct JsonSerializer<std::forward_list<T, Alloc>>
	{
		static void Serialize(rapidjson::Value* jsonVal, const std::forward_list<T, Alloc>* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, std::forward_list<T, Alloc>* outVal);
	};
#endif

#ifdef _LIST_
	/** LIST */
	template <typename T, typename Alloc>
	struct JsonSerializer<std::list<T, Alloc>>
	{
		static void Serialize(rapidjson::Value* jsonVal, const std::list<T, Alloc>* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, std::list<T, Alloc>* outVal);
	};
#endif

#ifdef _SET_
	/** UNORDERED SET */
	template <typename T, typename Hasher, typename Keyeq, typename Alloc>
	struct JsonSerializer<std::unordered_set<T, Hasher, Keyeq, Alloc>>
	{
		static void Serialize(rapidjson::Value* jsonVal, const std::unordered_set<T, Hasher, Keyeq, Alloc>* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, std::unordered_set<T, Hasher, Keyeq, Alloc>* outVal);
	};

	/** UNORDERED MULTI SET */
	template <typename T, typename Hasher, typename Keyeq, typename Alloc>
	struct JsonSerializer<std::unordered_multiset<T, Hasher, Keyeq, Alloc>>
	{
		static void Serialize(rapidjson::Value* jsonVal, const std::unordered_multiset<T, Hasher, Keyeq, Alloc>* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, std::unordered_multiset<T, Hasher, Keyeq, Alloc>* outVal);
	};
#endif

#ifdef _MAP_
	/** MAP */
	template <typename KeyEvent, typename Value, typename P, typename Alloc>
	struct JsonSerializer<std::map<KeyEvent, Value, P, Alloc>>
	{
		static void Serialize(rapidjson::Value* jsonVal, const std::map<KeyEvent, Value, P, Alloc>* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, std::map<KeyEvent, Value, P, Alloc>* outVal);
	};

	/** MULTI MAP */
	template <typename KeyEvent, typename Value, typename P, typename Alloc>
	struct JsonSerializer<std::multimap<KeyEvent, Value, P, Alloc>>
	{
		static void Serialize(rapidjson::Value* jsonVal, const std::multimap<KeyEvent, Value, P, Alloc>* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, std::multimap<KeyEvent, Value, P, Alloc>* outVal);
	};
#endif

#ifdef _UNORDERED_MAP_
	/** UNORDERED MAP */
	template <typename KeyEvent, typename Value, typename Hasher, typename Keyeq, typename Alloc>
	struct JsonSerializer<std::unordered_map<KeyEvent, Value, Hasher, Keyeq, Alloc>>
	{
		static void Serialize(rapidjson::Value* jsonVal, const std::unordered_map<KeyEvent, Value, Hasher, Keyeq, Alloc>* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, std::unordered_map<KeyEvent, Value, Hasher, Keyeq, Alloc>* outVal);
	};

	/** UNORDERED MULTI MAP */
	template <typename KeyEvent, typename Value, typename Hasher, typename Keyeq, typename Alloc>
	struct JsonSerializer<std::unordered_multimap<KeyEvent, Value, Hasher, Keyeq, Alloc>>
	{
		static void Serialize(rapidjson::Value* jsonVal, const std::unordered_multimap<KeyEvent, Value, Hasher, Keyeq, Alloc>* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, std::unordered_multimap<KeyEvent, Value, Hasher, Keyeq, Alloc>* outVal);
	};
#endif

#ifdef _MEMORY_
	/** UNIQUE PTR */
	template <typename T, typename Delete>
	struct JsonSerializer<std::unique_ptr<T, Delete>>
	{
		static void Serialize(rapidjson::Value* jsonVal, const std::unique_ptr<T, Delete>* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, std::unique_ptr<T, Delete>* outVal);
	};
#endif

#ifdef _OPTIONAL_
	/** OPTIONAL */
	template <typename T>
	struct JsonSerializer<std::optional<T>>
	{
		static void Serialize(rapidjson::Value* jsonVal, const std::optional<T>* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, std::optional<T>* outVal);
	};
#endif

#ifdef _UTILITY_
	/** PAIR */
	template <typename T1, typename T2>
	struct JsonSerializer<std::pair<T1, T2>>
	{
		static void Serialize(rapidjson::Value* jsonVal, const std::pair<T1, T2>* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, std::pair<T1, T2>* outVal);
	};
#endif

#ifdef _TUPLE_
	/** TUPLE */
	template <typename ... Ts>
	struct JsonSerializer<std::tuple<Ts...>>
	{
		static void Serialize(rapidjson::Value* jsonVal, const std::tuple<Ts...>* inVal, RapidJsonAllocator* allocator);
		static void Deserialize(rapidjson::Value* jsonVal, std::tuple<Ts...>* outVal);
	};
#endif
	
}