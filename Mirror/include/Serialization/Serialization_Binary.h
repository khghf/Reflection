#pragma once
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
	constexpr void FillTypeInfoBinary(TypeInfo& info);

	template <typename T>
	struct BinarySerializer;

	template <typename T>
	concept CustomBinarySerializer = requires(std::istream*istream, std::ostream*ostream, T*t)
	{
		BinarySerializer<T>::Serialize(ostream, t);
		BinarySerializer<T>::Deserialize(istream, t);
	};
}

namespace mirror::Serialization
{
	template <typename T>
	void SerializeBinary(std::string_view outFilePath, const T* inVal);
	template <typename T>
	void SerializeBinary(std::ostream* stream, const T* inVal);

	/// <summary>
	/// 将给定值以二进制格式序列化到输出流。内部会根据虚表指针获取实际的类型然后遍历父类转发到其对应的序列化函数
	/// </summary>
	/// <typeparam name="T">要序列化的值的类型。</typeparam>
	/// <param name="stream">指向输出流的指针，用于写入序列化数据。</param>
	/// <param name="inVal">指向要序列化的值的指针。</param>
	template <typename T>
	void SerializeBinaryDefault(std::ostream* stream, const T* inVal);

	/// <summary>
	/// 将值以二进制格式序列化到输出流中。
	/// </summary>
	/// <param name="stream">指向输出流的指针，用于写入序列化的二进制数据。</param>
	/// <param name="inVal">指向要序列化的值的指针。</param>
	/// <param name="type">表示要序列化值的类型的标识符。</param>
	void SerializeBinaryDefault(std::ostream* stream, const void* inVal, mirror::TypeId type);
	

	template <typename T>
	void DeserializeBinary(std::string_view inFilePath, T* outVal);
	template <typename T>
	void DeserializeBinary(std::istream* stream, T* outVal);

	/// <summary>
	/// 从二进制流中反序列化数据到指定的对象。内部会根据虚表指针获取实际的类型然后遍历父类转发到其对应的反序列化函数
	/// </summary>
	/// <typeparam name="T">目标对象的类型，用于存储反序列化后的数据。</typeparam>
	/// <param name="stream">输入的二进制流，用于读取数据。</param>
	/// <param name="outVal">指向目标对象的指针，用于存储反序列化后的数据。</param>
	template <typename T>
	void DeserializeBinaryDefault(std::istream* stream, T* outVal);

	/// <summary>
	/// 从二进制流中反序列化数据到指定类型的值。
	/// </summary>
	/// <param name="stream">指向输入流的指针，用于读取二进制数据。</param>
	/// <param name="outVal">指向输出值的指针，用于存储反序列化后的数据。</param>
	/// <param name="type">指定反序列化数据的类型标识符。</param>
	void DeserializeBinaryDefault(std::istream* stream, void* outVal, mirror::TypeId type);


	///** TypeId */
	//template <>
	//struct BinarySerializer<TypeId>
	//{
	//	static void Serialize(std::ostream* stream, const TypeId* inVal);
	//	static void Deserialize(std::istream* stream, TypeId* outVal);
	//};

	///** VariableId */
	//template <>
	//struct BinarySerializer<VariableId>
	//{
	//	static void Serialize(std::ostream* stream, const VariableId* inVal);
	//	static void Deserialize(std::istream* stream, VariableId* outVal);
	//};

	///** FunctionId */
	//template <>
	//struct BinarySerializer<FunctionId>
	//{
	//	static void Serialize(std::ostream* stream, const FunctionId* inVal);
	//	static void Deserialize(std::istream* stream, FunctionId* outVal);
	//};

#ifdef  GLM_ENABLE
	template <>
	struct BinarySerializer<glm::vec1>
	{
		static void Serialize(std::ostream* stream, const glm::vec1* inVal);
		static void Deserialize(std::istream* stream, glm::vec1* outVal);
	};
	template <>
	struct BinarySerializer<glm::vec2>
	{
		static void Serialize(std::ostream* stream, const glm::vec2* inVal);
		static void Deserialize(std::istream* stream, glm::vec2* outVal);
	};
	template <>
	struct BinarySerializer<glm::vec3>
	{
		static void Serialize(std::ostream* stream, const glm::vec3* inVal);
		static void Deserialize(std::istream* stream, glm::vec3* outVal);
	};
	template <>
	struct BinarySerializer<glm::vec4>
	{
		static void Serialize(std::ostream* stream, const glm::vec4* inVal);
		static void Deserialize(std::istream* stream, glm::vec4* outVal);
	};

#endif //  GLM_HPP




	/** BOOL */
	template <>
	struct BinarySerializer<bool>
	{
		static void Serialize(std::ostream* stream, const bool* inVal);
		static void Deserialize(std::istream* stream, bool* outVal);
	};

	/** FLOAT */
	template <>
	struct BinarySerializer<float>
	{
		static void Serialize(std::ostream* stream, const float* inVal);
		static void Deserialize(std::istream* stream, float* outVal);
	};

	/** DOUBLE */
	template <>
	struct BinarySerializer<double>
	{
		static void Serialize(std::ostream* stream, const double* inVal);
		static void Deserialize(std::istream* stream, double* outVal);
	};

	/** INT8 */
	template <>
	struct BinarySerializer<int8_t>
	{
		static void Serialize(std::ostream* stream, const int8_t* inVal);
		static void Deserialize(std::istream* stream, int8_t* outVal);
	};

	/** INT16 */
	template <>
	struct BinarySerializer<int16_t>
	{
		static void Serialize(std::ostream* stream, const int16_t* inVal);
		static void Deserialize(std::istream* stream, int16_t* outVal);
	};

	/** INT32 */
	template <>
	struct BinarySerializer<int32_t>
	{
		static void Serialize(std::ostream* stream, const int32_t* inVal);
		static void Deserialize(std::istream* stream, int32_t* outVal);
	};

	/** INT64 */
	template <>
	struct BinarySerializer<int64_t>
	{
		static void Serialize(std::ostream* stream, const int64_t* inVal);
		static void Deserialize(std::istream* stream, int64_t* outVal);
	};

	/** UINT8 */
	template <>
	struct BinarySerializer<uint8_t>
	{
		static void Serialize(std::ostream* stream, const uint8_t* inVal);
		static void Deserialize(std::istream* stream, uint8_t* outVal);
	};

	/** UINT16 */
	template <>
	struct BinarySerializer<uint16_t>
	{
		static void Serialize(std::ostream* stream, const uint16_t* inVal);
		static void Deserialize(std::istream* stream, uint16_t* outVal);
	};

	/** UINT32 */
	template <>
	struct BinarySerializer<uint32_t>
	{
		static void Serialize(std::ostream* stream, const uint32_t* inVal);
		static void Deserialize(std::istream* stream, uint32_t* outVal);
	};

	/** UINT64 */
	template <>
	struct BinarySerializer<uint64_t>
	{
		static void Serialize(std::ostream* stream, const uint64_t* inVal);
		static void Deserialize(std::istream* stream, uint64_t* outVal);
	};

	/** CONST CHAR PTR - 注意是指针的指针 */
	template <>
	struct BinarySerializer<char*>
	{
		static void Serialize(std::ostream* stream, const char* * inVal);
		static void Deserialize(std::istream* stream, const char** outVal);
	};

	// ============ STL 容器特化 ============

#ifdef _STRING_
	/** STRING */
	template <typename Elem, typename Traits, typename Alloc>
	struct BinarySerializer<std::basic_string<Elem, Traits, Alloc>>
	{
		static void Serialize(std::ostream* stream, const std::basic_string<Elem, Traits, Alloc>* inVal);
		static void Deserialize(std::istream* stream, std::basic_string<Elem, Traits, Alloc>* outVal);
	};
#endif

#ifdef _VECTOR_
	/** VECTOR */
	template <typename T, typename Alloc>
	struct BinarySerializer<std::vector<T, Alloc>>
	{
		static void Serialize(std::ostream* stream, const std::vector<T, Alloc>* inVal);
		static void Deserialize(std::istream* stream, std::vector<T, Alloc>* outVal);
	};

	template <typename Alloc>
	struct BinarySerializer<std::vector<bool, Alloc>>
	{
		static void Serialize(std::ostream* stream, const std::vector<bool, Alloc>* inVal);
		static void Deserialize(std::istream* stream, std::vector<bool, Alloc>* outVal);
	};
#endif

#ifdef _ARRAY_
	/** ARRAY */
	template <typename T, size_t Size>
	struct BinarySerializer<std::array<T, Size>>
	{
		static void Serialize(std::ostream* stream, const std::array<T, Size>* inVal);
		static void Deserialize(std::istream* stream, std::array<T, Size>* outVal);
	};
#endif

#ifdef _DEQUE_
	/** DEQUE */
	template <typename T, typename Alloc>
	struct BinarySerializer<std::deque<T, Alloc>>
	{
		static void Serialize(std::ostream* stream, const std::deque<T, Alloc>* inVal);
		static void Deserialize(std::istream* stream, std::deque<T, Alloc>* outVal);
	};
#endif

#ifdef _FORWARD_LIST_
	/** FORWARD LIST */
	template <typename T, typename Alloc>
	struct BinarySerializer<std::forward_list<T, Alloc>>
	{
		static void Serialize(std::ostream* stream, const std::forward_list<T, Alloc>* inVal);
		static void Deserialize(std::istream* stream, std::forward_list<T, Alloc>* outVal);
	};
#endif

#ifdef _LIST_
	/** LIST */
	template <typename T, typename Alloc>
	struct BinarySerializer<std::list<T, Alloc>>
	{
		static void Serialize(std::ostream* stream, const std::list<T, Alloc>* inVal);
		static void Deserialize(std::istream* stream, std::list<T, Alloc>* outVal);
	};
#endif

#ifdef _SET_
	/** UNORDERED SET */
	template <typename T, typename Hasher, typename Keyeq, typename Alloc>
	struct BinarySerializer<std::unordered_set<T, Hasher, Keyeq, Alloc>>
	{
		static void Serialize(std::ostream* stream, const std::unordered_set<T, Hasher, Keyeq, Alloc>* inVal);
		static void Deserialize(std::istream* stream, std::unordered_set<T, Hasher, Keyeq, Alloc>* outVal);
	};

	/** UNORDERED MULTI SET */
	template <typename T, typename Hasher, typename Keyeq, typename Alloc>
	struct BinarySerializer<std::unordered_multiset<T, Hasher, Keyeq, Alloc>>
	{
		static void Serialize(std::ostream* stream, const std::unordered_multiset<T, Hasher, Keyeq, Alloc>* inVal);
		static void Deserialize(std::istream* stream, std::unordered_multiset<T, Hasher, Keyeq, Alloc>* outVal);
	};
#endif

#ifdef _MAP_
	/** MAP */
	template <typename KeyEvent, typename Value, typename P, typename Alloc>
	struct BinarySerializer<std::map<KeyEvent, Value, P, Alloc>>
	{
		static void Serialize(std::ostream* stream, const std::map<KeyEvent, Value, P, Alloc>* inVal);
		static void Deserialize(std::istream* stream, std::map<KeyEvent, Value, P, Alloc>* outVal);
	};

	/** MULTI MAP */
	template <typename KeyEvent, typename Value, typename P, typename Alloc>
	struct BinarySerializer<std::multimap<KeyEvent, Value, P, Alloc>>
	{
		static void Serialize(std::ostream* stream, const std::multimap<KeyEvent, Value, P, Alloc>* inVal);
		static void Deserialize(std::istream* stream, std::multimap<KeyEvent, Value, P, Alloc>* outVal);
	};
#endif

#ifdef _UNORDERED_MAP_
	/** UNORDERED MAP */
	template <typename KeyEvent, typename Value, typename Hasher, typename Keyeq, typename Alloc>
	struct BinarySerializer<std::unordered_map<KeyEvent, Value, Hasher, Keyeq, Alloc>>
	{
		static void Serialize(std::ostream* stream, const std::unordered_map<KeyEvent, Value, Hasher, Keyeq, Alloc>* inVal);
		static void Deserialize(std::istream* stream, std::unordered_map<KeyEvent, Value, Hasher, Keyeq, Alloc>* outVal);
	};

	/** UNORDERED MULTI MAP */
	template <typename KeyEvent, typename Value, typename Hasher, typename Keyeq, typename Alloc>
	struct BinarySerializer<std::unordered_multimap<KeyEvent, Value, Hasher, Keyeq, Alloc>>
	{
		static void Serialize(std::ostream* stream, const std::unordered_multimap<KeyEvent, Value, Hasher, Keyeq, Alloc>* inVal);
		static void Deserialize(std::istream* stream, std::unordered_multimap<KeyEvent, Value, Hasher, Keyeq, Alloc>* outVal);
	};
#endif

#ifdef _MEMORY_
	/** UNIQUE PTR */
	template <typename T, typename Delete>
	struct BinarySerializer<std::unique_ptr<T, Delete>>
	{
		static void Serialize(std::ostream* stream, const std::unique_ptr<T, Delete>* inVal);
		static void Deserialize(std::istream* stream, std::unique_ptr<T, Delete>* outVal);
	};
#endif

#ifdef _OPTIONAL_
	/** OPTIONAL */
	template <typename T>
	struct BinarySerializer<std::optional<T>>
	{
		static void Serialize(std::ostream* stream, const std::optional<T>* inVal);
		static void Deserialize(std::istream* stream, std::optional<T>* outVal);
	};
#endif

#ifdef _UTILITY_
	/** PAIR */
	template <typename T1, typename T2>
	struct BinarySerializer<std::pair<T1, T2>>
	{
		static void Serialize(std::ostream* stream, const std::pair<T1, T2>* inVal);
		static void Deserialize(std::istream* stream, std::pair<T1, T2>* outVal);
	};

	/** TUPLE */
	template <typename ... Ts>
	struct BinarySerializer<std::tuple<Ts...>>
	{
		static void Serialize(std::ostream* stream, const std::tuple<Ts...>* inVal);
		static void Deserialize(std::istream* stream, std::tuple<Ts...>* outVal);
	};
#endif


	
}