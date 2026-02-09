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
	concept CustomBinarySerializer = requires(std::istream istream, std::ostream ostream, T t)
	{
		BinarySerializer<T>::Serialize(ostream, t);
		BinarySerializer<T>::Deserialize(istream, t);
	};
}

namespace mirror::Serialization
{
	template <typename T>
	void SerializeBinary(std::ostream& stream, const T& value);
	template <typename T>
	void DeserializeBinary(std::istream& stream, T& value);

	void SerializeBinaryDefault(std::ostream& stream, const void* data, mirror::TypeId type);
	void DeserializeBinaryDefault(std::istream& stream, void* data, mirror::TypeId type);

	template <typename T>
	void SerializeBinaryDefault(std::ostream& stream, const T& value);
	template <typename T>
	void DeserializeBinaryDefault(std::istream& stream, T& value);

#ifdef _STRING_
	/** STRING */
	template <typename Elem, typename Traits, typename Alloc>
	struct BinarySerializer<std::basic_string<Elem, Traits, Alloc>>
	{
		static void Serialize(std::ostream& stream, const std::basic_string<Elem, Traits, Alloc>& value);
		static void Deserialize(std::istream& stream, std::basic_string<Elem, Traits, Alloc>& value);
	};
#endif

#ifdef _VECTOR_
	/** VECTOR */
	template <typename T, typename Alloc>
	struct BinarySerializer<std::vector<T, Alloc>>
	{
		static void Serialize(std::ostream& stream, const std::vector<T, Alloc>& value);
		static void Deserialize(std::istream& stream, std::vector<T, Alloc>& value);
	};

	template <typename Alloc>
	struct BinarySerializer<std::vector<bool, Alloc>>
	{
		static void Serialize(std::ostream& stream, const std::vector<bool, Alloc>& value);
		static void Deserialize(std::istream& stream, std::vector<bool, Alloc>& value);
	};
#endif

#ifdef _ARRAY_
	/** ARRAY */
	template <typename T, size_t Size>
	struct BinarySerializer<std::array<T, Size>>
	{
		static void Serialize(std::ostream& stream, const std::array<T, Size>& value);
		static void Deserialize(std::istream& stream, std::array<T, Size>& value);
	};
#endif

#ifdef _DEQUE_
	/** DEQUE*/
	template <typename T, typename Alloc>
	struct BinarySerializer<std::deque<T, Alloc>>
	{
		static void Serialize(std::ostream& stream, const std::deque<T, Alloc>& value);
		static void Deserialize(std::istream& stream, std::deque<T, Alloc>& value);
	};
#endif

#ifdef _FORWARD_LIST_
	/** FORWARD LIST */
	template <typename T, typename Alloc>
	struct BinarySerializer<std::forward_list<T, Alloc>>
	{
		static void Serialize(std::ostream& stream, const std::forward_list<T, Alloc>& value);
		static void Deserialize(std::istream& stream, std::forward_list<T, Alloc>& value);
	};
#endif

#ifdef _LIST_
	/** LIST */
	template <typename T, typename Alloc>
	struct BinarySerializer<std::list<T, Alloc>>
	{
		static void Serialize(std::ostream& stream, const std::list<T, Alloc>& value);
		static void Deserialize(std::istream& stream, std::list<T, Alloc>& value);
	};
#endif

#ifdef _SET_
	/** UNORDERED SET */
	template <typename T, typename Hasher, typename Keyeq, typename Alloc>
	struct BinarySerializer<std::unordered_set<T, Hasher, Keyeq, Alloc>>
	{
		static void Serialize(std::ostream& stream, const std::unordered_set<T, Hasher, Keyeq, Alloc>& value);
		static void Deserialize(std::istream& stream, std::unordered_set<T, Hasher, Keyeq, Alloc>& value);
	};

	/** UNORDERED MULTI SET */
	template <typename T, typename Hasher, typename Keyeq, typename Alloc>
	struct BinarySerializer<std::unordered_multiset<T, Hasher, Keyeq, Alloc>>
	{
		static void Serialize(std::ostream& stream, const std::unordered_multiset<T, Hasher, Keyeq, Alloc>& value);
		static void Deserialize(std::istream& stream, std::unordered_multiset<T, Hasher, Keyeq, Alloc>& value);
	};
#endif

#ifdef _MAP_
	/** MAP */
	template <typename Key, typename Value, typename P, typename Alloc>
	struct BinarySerializer<std::map<Key, Value, P, Alloc>>
	{
		static void Serialize(std::ostream& stream, const std::map<Key, Value, P, Alloc>& value);
		static void Deserialize(std::istream& stream, std::map<Key, Value, P, Alloc>& value);
	};

	/** MULTI MAP */
	template <typename Key, typename Value, typename P, typename Alloc>
	struct BinarySerializer<std::multimap<Key, Value, P, Alloc>>
	{
		static void Serialize(std::ostream& stream, const std::multimap<Key, Value, P, Alloc>& value);
		static void Deserialize(std::istream& stream, std::multimap<Key, Value, P, Alloc>& value);
	};
#endif

#ifdef _UNORDERED_MAP_
	/** UNORDERED MAP */
	template <typename Key, typename Value, typename Hasher, typename Keyeq, typename Alloc>
	struct BinarySerializer<std::unordered_map<Key, Value, Hasher, Keyeq, Alloc>>
	{
		static void Serialize(std::ostream& stream, const std::unordered_map<Key, Value, Hasher, Keyeq, Alloc>& value);
		static void Deserialize(std::istream& stream, std::unordered_map<Key, Value, Hasher, Keyeq, Alloc>& value);
	};

	/** UNORDERED MULTI MAP */
	template <typename Key, typename Value, typename Hasher, typename Keyeq, typename Alloc>
	struct BinarySerializer<std::unordered_multimap<Key, Value, Hasher, Keyeq, Alloc>>
	{
		static void Serialize(std::ostream& stream, const std::unordered_multimap<Key, Value, Hasher, Keyeq, Alloc>& value);
		static void Deserialize(std::istream& stream, std::unordered_multimap<Key, Value, Hasher, Keyeq, Alloc>& value);
	};
#endif

#ifdef _MEMORY_
	/** UNIQUE PTR */
	template <typename T, typename Delete>
	struct BinarySerializer<std::unique_ptr<T, Delete>>
	{
		static void Serialize(std::ostream& stream, const std::unique_ptr<T, Delete>& value);
		static void Deserialize(std::istream& stream, std::unique_ptr<T, Delete>& value);
	};
#endif

#ifdef _OPTIONAL_
	/** OPTIONAL */
	template <typename T>
	struct BinarySerializer<std::optional<T>>
	{
		static void Serialize(std::ostream& stream, const std::optional<T>& value);
		static void Deserialize(std::istream& stream, std::optional<T>& value);
	};
#endif

#ifdef _UTILITY_
	/** PAIR */
	template <typename T1, typename T2>
	struct BinarySerializer<std::pair<T1, T2>>
	{
		static void Serialize(std::ostream& stream, const std::pair<T1, T2>& value);
		static void Deserialize(std::istream& stream, std::pair<T1, T2>& value);
	};

	/** TUPLE */
	template <typename ... Ts>
	struct BinarySerializer<std::tuple<Ts...>>
	{
		static void Serialize(std::ostream& stream, const std::tuple<Ts...>& value);
		static void Deserialize(std::istream& stream, std::tuple<Ts...>& value);
	};
#endif
}

