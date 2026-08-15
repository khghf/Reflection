#include"Serialization_Binary.h"
#include<iostream>
#include"../Tool/Util.h"
#include <cstring>

namespace mirror::Serialization
{
	template <typename T>
	constexpr void FillTypeInfoBinary(TypeInfo& info)
	{
		if constexpr (std::is_same_v<void, T>)
		{
			info.BinarySerializer = nullptr;
			info.BinaryDeserializer = nullptr;
		}
		else if constexpr (CustomBinarySerializer<T>)
		{
			info.BinarySerializer = [](std::ostream* stream, const void* data)
				{
					BinarySerializer<T>::Serialize(stream, static_cast<const T*>(data));
				};

			info.BinaryDeserializer = [](std::istream* stream, void* data)
				{
					BinarySerializer<T>::Deserialize(stream, static_cast<T*>(data));
				};
		}
		else
		{
			info.BinarySerializer = [](std::ostream* stream, const void* data)
				{
					SerializeBinaryDefault(stream, static_cast<const T*>(data));
				};

			info.BinaryDeserializer = [](std::istream* stream, void* data)
				{
					DeserializeBinaryDefault(stream, static_cast<T*>(data));
				};
		}
	}

	// ============ 辅助读写函数 ============

	template <typename T>
	void WriteStream(std::ostream* stream, const T* inVal)
	{
		stream->write(reinterpret_cast<const char*>(inVal), sizeof(T));
	}

	template <typename T>
	T ReadStream(std::istream* stream)
	{
		T t{};
		stream->read(reinterpret_cast<char*>(&t), sizeof(T));
		return t;
	}


	template<typename T>
	void SerializeBinary(std::string_view outFilePath, const T* inVal)
	{
		std::ofstream out(outFilePath.data(), std::ios::out | std::ios::binary);
		SerializeBinary(&out, inVal);
		out.close();
	}

	template <typename T>
	void SerializeBinary(std::ostream* stream, const T* inVal)
	{
		if constexpr (CustomBinarySerializer<T>)BinarySerializer<T>::Serialize(stream, inVal);
		else SerializeBinaryDefault(stream, inVal);
	}

	template <typename T>
	void SerializeBinaryDefault(std::ostream* stream, const T* inVal)
	{
		if constexpr (std::is_trivially_copyable_v<T>)
		{
			WriteStream(stream, inVal);
		}
		else
		{
			TypeId actualTypeId = TypeId::Create<T>();
			if constexpr (std::is_polymorphic_v<T>)
			{
				const TypeId& idFromVTable = GetTypeId(GetVTable(inVal));
				// 根据虚表获取实际类型
				if (actualTypeId != idFromVTable)actualTypeId = idFromVTable;

				const TypeInfo& actualInfo = actualTypeId.GetInfo();

				// 调用父类的序列化函数
				for (const auto& baseInfo : actualInfo.BaseClasses)
				{
					SerializeBinaryDefault(stream, static_cast<const void*>(inVal), baseInfo.BaseId);
				}
			}
			SerializeBinaryDefault(stream, static_cast<const void*>(inVal), actualTypeId);
		}
	}

	inline void SerializeBinaryDefault(std::ostream* stream, const void* inVal, TypeId type)
	{
		auto& info = GetTypeInfo(type);
		// 序列化成员
		auto& members = info.Members;
		for (auto& member : members)
		{
			if (!!(member.Properties & MemberProperties::Serializable) && !member.Variable.IsRefOrPointer())
			{
				member.Variable.GetTypeId().GetInfo().BinarySerializer(stream, util::VoidOffset(inVal, member.Offset));
			}
		}
	}

	




	template<typename T>
	void DeserializeBinary(std::string_view inFilePath, T* outVal)
	{
		std::ifstream in(inFilePath.data(), std::ios::in | std::ios::binary);
		DeserializeBinary(&in, outVal);
		in.close();
	}

	template <typename T>
	void DeserializeBinary(std::istream* stream, T* outVal)
	{
		if constexpr (CustomBinarySerializer<T>)BinarySerializer<T>::Deserialize(stream, outVal);
		else DeserializeBinaryDefault(stream, outVal);
	}

	template <typename T>
	void DeserializeBinaryDefault(std::istream* stream, T* outVal)
	{
		if constexpr (std::is_trivially_copyable_v<T>)
		{
			*outVal = ReadStream<T>(stream);
		}
		else
		{
			TypeId actualTypeId = TypeId::Create<T>();
			if constexpr (std::is_polymorphic_v<T>)
			{
				const TypeId& idFromVTable = GetTypeId(GetVTable(outVal));
				// 根据虚表获取实际类型
				if (actualTypeId != idFromVTable)actualTypeId = idFromVTable;

				const TypeInfo& actualInfo = actualTypeId.GetInfo();

				// 调用父类的序列化函数
				for (const auto& baseInfo : actualInfo.BaseClasses)
				{
					DeserializeBinaryDefault(stream, static_cast<void*>(outVal), baseInfo.BaseId);
				}
			}
			DeserializeBinaryDefault(stream, static_cast<void*>(outVal), actualTypeId);
		}
	}

	inline void DeserializeBinaryDefault(std::istream* stream, void* outVal, TypeId type)
	{
		auto& info = GetTypeInfo(type);
		// 反序列化成员
		auto& members = info.Members;
		for (auto& member : members)
		{
			if (!!(member.Properties & MemberProperties::Serializable) && !member.Variable.IsRefOrPointer())
			{
				member.Variable.GetTypeId().GetInfo().BinaryDeserializer(stream, util::VoidOffset(outVal, member.Offset));
			}
		}
	}



	/*inline void BinarySerializer<TypeId>::Serialize(std::ostream* stream, const TypeId* inVal)
	{
		const auto id = inVal->GetId();
		WriteStream(stream,&id);
	}
	inline void BinarySerializer<TypeId>::Deserialize(std::istream* stream, TypeId* outVal)
	{
		uint64_t id = ReadStream<uint64_t>(stream);
		outVal->SetTypeId(id);
	}
	inline void BinarySerializer<VariableId>::Serialize(std::ostream* stream, const VariableId* inVal)
	{
		const auto id = inVal->GetTypeId().GetId();
		const auto isConst = inVal->IsConst();
		const auto isVolatile = inVal->IsVolatile();
		const auto isReference = inVal->IsReference();
		const auto isRValReference = inVal->IsRValReference();
		const auto pointerAmount = inVal->GetPointerAmount();
		const auto arraySize = inVal->GetArraySize();

		WriteStream(stream, &id);
		WriteStream(stream, &isConst);
		WriteStream(stream, &isVolatile);
		WriteStream(stream, &isReference);
		WriteStream(stream, &isRValReference);
		WriteStream(stream, &pointerAmount);
		WriteStream(stream, &arraySize);
	}
	inline void BinarySerializer<VariableId>::Deserialize(std::istream* stream, VariableId* outVal)
	{
	
	}
	inline void BinarySerializer<FunctionId>::Serialize(std::ostream* stream, const FunctionId* inVal)
	{
	}
	inline void BinarySerializer<FunctionId>::Deserialize(std::istream* stream, FunctionId* outVal)
	{
	}*/
#ifdef  GLM_ENABLE
	inline void BinarySerializer<glm::vec1>::Serialize(std::ostream* stream, const glm::vec1* inVal)
	{
		WriteStream(stream, &inVal->x);
	}
	inline void BinarySerializer<glm::vec1>::Deserialize(std::istream* stream, glm::vec1* outVal)
	{
		outVal->x = ReadStream<decltype(outVal->x)>(stream);
	}
	inline void BinarySerializer<glm::vec2>::Serialize(std::ostream* stream, const glm::vec2* inVal)
	{
		WriteStream(stream, &inVal->x);
		WriteStream(stream, &inVal->y);
	}
	inline void BinarySerializer<glm::vec2>::Deserialize(std::istream* stream, glm::vec2* outVal)
	{
		outVal->x = ReadStream<decltype(outVal->x)>(stream);
		outVal->y = ReadStream<decltype(outVal->y)>(stream);
	}
	inline void BinarySerializer<glm::vec3>::Serialize(std::ostream* stream, const glm::vec3* inVal)
	{
		WriteStream(stream, &inVal->x);
		WriteStream(stream, &inVal->y);
		WriteStream(stream, &inVal->z);
	}
	inline void BinarySerializer<glm::vec3>::Deserialize(std::istream* stream, glm::vec3* outVal)
	{
		outVal->x = ReadStream<decltype(outVal->x)>(stream);
		outVal->y = ReadStream<decltype(outVal->y)>(stream);
		outVal->z = ReadStream<decltype(outVal->z)>(stream);
	}
	inline void BinarySerializer<glm::vec4>::Serialize(std::ostream* stream, const glm::vec4* inVal)
	{
		WriteStream(stream, &inVal->x);
		WriteStream(stream, &inVal->y);
		WriteStream(stream, &inVal->z);
		WriteStream(stream, &inVal->w);
	}
	inline void BinarySerializer<glm::vec4>::Deserialize(std::istream* stream, glm::vec4* outVal)
	{
		outVal->x = ReadStream<decltype(outVal->x)>(stream);
		outVal->y = ReadStream<decltype(outVal->y)>(stream);
		outVal->z = ReadStream<decltype(outVal->z)>(stream);
		outVal->w = ReadStream<decltype(outVal->w)>(stream);
	}
#endif //  GLM_ENABLE


	// ============ 基础类型特化实现 ============

	/** BOOL */
	inline void BinarySerializer<bool>::Serialize(std::ostream* stream, const bool* inVal)
	{
		WriteStream(stream, inVal);
	}

	inline void BinarySerializer<bool>::Deserialize(std::istream* stream, bool* outVal)
	{
		*outVal = ReadStream<bool>(stream);
	}

	/** FLOAT */
	inline void BinarySerializer<float>::Serialize(std::ostream* stream, const float* inVal)
	{
		WriteStream(stream, inVal);
	}

	inline void BinarySerializer<float>::Deserialize(std::istream* stream, float* outVal)
	{
		*outVal = ReadStream<float>(stream);
	}

	/** DOUBLE */
	inline void BinarySerializer<double>::Serialize(std::ostream* stream, const double* inVal)
	{
		WriteStream(stream, inVal);
	}

	inline void BinarySerializer<double>::Deserialize(std::istream* stream, double* outVal)
	{
		*outVal = ReadStream<double>(stream);
	}

	/** INT8 */
	inline void BinarySerializer<int8_t>::Serialize(std::ostream* stream, const int8_t* inVal)
	{
		WriteStream(stream, inVal);
	}

	inline void BinarySerializer<int8_t>::Deserialize(std::istream* stream, int8_t* outVal)
	{
		*outVal = ReadStream<int8_t>(stream);
	}

	/** INT16 */
	inline void BinarySerializer<int16_t>::Serialize(std::ostream* stream, const int16_t* inVal)
	{
		WriteStream(stream, inVal);
	}

	inline void BinarySerializer<int16_t>::Deserialize(std::istream* stream, int16_t* outVal)
	{
		*outVal = ReadStream<int16_t>(stream);
	}

	/** INT32 */
	inline void BinarySerializer<int32_t>::Serialize(std::ostream* stream, const int32_t* inVal)
	{
		WriteStream(stream, inVal);
	}

	inline void BinarySerializer<int32_t>::Deserialize(std::istream* stream, int32_t* outVal)
	{
		*outVal = ReadStream<int32_t>(stream);
	}

	/** INT64 */
	inline void BinarySerializer<int64_t>::Serialize(std::ostream* stream, const int64_t* inVal)
	{
		WriteStream(stream, inVal);
	}

	inline void BinarySerializer<int64_t>::Deserialize(std::istream* stream, int64_t* outVal)
	{
		*outVal = ReadStream<int64_t>(stream);
	}

	/** UINT8 */
	inline void BinarySerializer<uint8_t>::Serialize(std::ostream* stream, const uint8_t* inVal)
	{
		WriteStream(stream, inVal);
	}

	inline void BinarySerializer<uint8_t>::Deserialize(std::istream* stream, uint8_t* outVal)
	{
		*outVal = ReadStream<uint8_t>(stream);
	}

	/** UINT16 */
	inline void BinarySerializer<uint16_t>::Serialize(std::ostream* stream, const uint16_t* inVal)
	{
		WriteStream(stream, inVal);
	}

	inline void BinarySerializer<uint16_t>::Deserialize(std::istream* stream, uint16_t* outVal)
	{
		*outVal = ReadStream<uint16_t>(stream);
	}

	/** UINT32 */
	inline void BinarySerializer<uint32_t>::Serialize(std::ostream* stream, const uint32_t* inVal)
	{
		WriteStream(stream, inVal);
	}

	inline void BinarySerializer<uint32_t>::Deserialize(std::istream* stream, uint32_t* outVal)
	{
		*outVal = ReadStream<uint32_t>(stream);
	}

	/** UINT64 */
	inline void BinarySerializer<uint64_t>::Serialize(std::ostream* stream, const uint64_t* inVal)
	{
		WriteStream(stream, inVal);
	}

	inline void BinarySerializer<uint64_t>::Deserialize(std::istream* stream, uint64_t* outVal)
	{
		*outVal = ReadStream<uint64_t>(stream);
	}

	/**CHAR PTR */
	inline void BinarySerializer< char*>::Serialize(std::ostream* stream, const char* * inVal)
	{
		size_t len = *inVal ? std::char_traits<char>::length(*inVal) : 0;
		WriteStream(stream, &len);
		if (len > 0)
		{
			stream->write(*inVal, len);
		}
	}

	inline void BinarySerializer<char*>::Deserialize(std::istream* stream, const char** outVal)
	{
		size_t len = ReadStream<size_t>(stream);
		if (len > 0)
		{
			char* buffer = new char[len + 1];
			stream->read(buffer, len);
			buffer[len] = '\0';
			*outVal = buffer;
		}
		else
		{
			*outVal = nullptr;
		}
	}

	// ============ STL 容器特化实现 ============

#ifdef _STRING_
	/** STRING */
	template <typename Elem, typename Traits, typename Alloc>
	void BinarySerializer<std::basic_string<Elem, Traits, Alloc>>::Serialize(std::ostream* stream, const std::basic_string<Elem, Traits, Alloc>* inVal)
	{
		size_t size = inVal->size();
		WriteStream(stream, &size);
		stream->write(inVal->data(), inVal->size() * sizeof(Elem));
	}

	template <typename Elem, typename Traits, typename Alloc>
	void BinarySerializer<std::basic_string<Elem, Traits, Alloc>>::Deserialize(std::istream* stream, std::basic_string<Elem, Traits, Alloc>* outVal)
	{
		size_t size = ReadStream<size_t>(stream);

		outVal->clear();
		outVal->resize(size);

		stream->read(outVal->data(), size * sizeof(Elem));
	}
#endif

#ifdef _VECTOR_
	/** VECTOR */
	template <typename T, typename Alloc>
	void BinarySerializer<std::vector<T, Alloc>>::Serialize(std::ostream* stream, const std::vector<T, Alloc>* inVal)
	{
		if constexpr (std::is_trivially_copyable_v<T>)
		{
			size_t size = inVal->size();

			WriteStream(stream, &size);
			stream->write(reinterpret_cast<const char*>(inVal->data()), inVal->size() * sizeof(T));
		}
		else
		{
			size_t size = inVal->size();

			WriteStream(stream, &size);

			for (auto& element : *inVal)
			{
				SerializeBinary(stream, &element);
			}
		}
	}

	template <typename T, typename Alloc>
	void BinarySerializer<std::vector<T, Alloc>>::Deserialize(std::istream* stream, std::vector<T, Alloc>* outVal)
	{
		if constexpr (std::is_trivially_copyable_v<T>)
		{
			size_t size = ReadStream<size_t>(stream);

			outVal->clear();
			outVal->resize(size);

			stream->read(reinterpret_cast<char*>(outVal->data()), size * sizeof(T));
		}
		else
		{
			size_t size = ReadStream<size_t>(stream);

			outVal->clear();
			outVal->reserve(size);

			for (size_t i{}; i < size; ++i)
			{
				auto& element = outVal->emplace_back();
				DeserializeBinary(stream, &element);
			}
		}
	}

	/** BOOLEAN VECTOR */
	template <typename Alloc>
	void BinarySerializer<std::vector<bool, Alloc>>::Serialize(std::ostream* stream, const std::vector<bool, Alloc>* inVal)
	{
		size_t size = inVal->size();

		WriteStream(stream, &size);
		stream->write(reinterpret_cast<const char*>(inVal->data()), inVal->size() / 8);
	}

	template <typename Alloc>
	void BinarySerializer<std::vector<bool, Alloc>>::Deserialize(std::istream* stream, std::vector<bool, Alloc>* outVal)
	{
		size_t size = ReadStream<size_t>(stream);

		outVal->clear();
		outVal->resize(size);

		stream->read(reinterpret_cast<char*>(outVal->data()), size / 8);
	}
#endif

#ifdef _ARRAY_
	/** ARRAY */
	template <typename T, size_t Size>
	void BinarySerializer<std::array<T, Size>>::Serialize(std::ostream* stream, const std::array<T, Size>* inVal)
	{
		if constexpr (std::is_trivially_copyable_v<T>)
		{
			WriteStream(stream, inVal);
		}
		else
		{
			for (auto& element : *inVal)
				SerializeBinary(stream, &element);
		}
	}

	template <typename T, size_t Size>
	void BinarySerializer<std::array<T, Size>>::Deserialize(std::istream* stream, std::array<T, Size>* outVal)
	{
		if constexpr (std::is_trivially_copyable_v<T>)
		{
			*outVal = ReadStream<std::array<T, Size>>(stream);
		}
		else
		{
			for (auto& element : *outVal)
				DeserializeBinary(stream, &element);
		}
	}
#endif

#ifdef _DEQUE_
	/** DEQUE */
	template <typename T, typename Alloc>
	void BinarySerializer<std::deque<T, Alloc>>::Serialize(std::ostream* stream, const std::deque<T, Alloc>* inVal)
	{
		size_t size = inVal->size();

		WriteStream(stream, &size);

		for (auto& element : *inVal)
			SerializeBinary(stream, &element);
	}

	template <typename T, typename Alloc>
	void BinarySerializer<std::deque<T, Alloc>>::Deserialize(std::istream* stream, std::deque<T, Alloc>* outVal)
	{
		size_t size = ReadStream<size_t>(stream);

		outVal->clear();

		for (size_t i{}; i < size; ++i)
		{
			auto& element = outVal->emplace_back();
			DeserializeBinary(stream, &element);
		}
	}
#endif

#ifdef _FORWARD_LIST_
	/** FORWARD LIST */
	template <typename T, typename Alloc>
	void BinarySerializer<std::forward_list<T, Alloc>>::Serialize(std::ostream* stream, const std::forward_list<T, Alloc>* inVal)
	{
		size_t size = std::distance(inVal->begin(), inVal->end());
		WriteStream(stream, &size);

		for (auto& element : *inVal)
			SerializeBinary(stream, &element);
	}

	template <typename T, typename Alloc>
	void BinarySerializer<std::forward_list<T, Alloc>>::Deserialize(std::istream* stream, std::forward_list<T, Alloc>* outVal)
	{
		size_t size = ReadStream<size_t>(stream);

		outVal->clear();

		for (size_t i{}; i < size; ++i)
		{
			auto& element = outVal->emplace_front();
			DeserializeBinary(stream, &element);
		}
	}
#endif

#ifdef _LIST_
	/** LIST */
	template <typename T, typename Alloc>
	void BinarySerializer<std::list<T, Alloc>>::Serialize(std::ostream* stream, const std::list<T, Alloc>* inVal)
	{
		size_t size = inVal->size();

		WriteStream(stream, &size);

		for (auto& element : *inVal)
			SerializeBinary(stream, &element);
	}

	template <typename T, typename Alloc>
	void BinarySerializer<std::list<T, Alloc>>::Deserialize(std::istream* stream, std::list<T, Alloc>* outVal)
	{
		size_t size = ReadStream<size_t>(stream);

		outVal->clear();

		for (size_t i{}; i < size; ++i)
		{
			auto& element = outVal->emplace_back();
			DeserializeBinary(stream, &element);
		}
	}
#endif

#ifdef _SET_
	/** UNORDERED SET */
	template <typename T, typename Hasher, typename Keyeq, typename Alloc>
	void BinarySerializer<std::unordered_set<T, Hasher, Keyeq, Alloc>>::Serialize(std::ostream* stream, const std::unordered_set<T, Hasher, Keyeq, Alloc>* inVal)
	{
		size_t size = inVal->size();

		WriteStream(stream, &size);

		for (auto& element : *inVal)
			SerializeBinary(stream, &element);
	}

	template <typename T, typename Hasher, typename Keyeq, typename Alloc>
	void BinarySerializer<std::unordered_set<T, Hasher, Keyeq, Alloc>>::Deserialize(std::istream* stream, std::unordered_set<T, Hasher, Keyeq, Alloc>* outVal)
	{
		size_t size = ReadStream<size_t>(stream);

		outVal->clear();

		for (size_t i{}; i < size; ++i)
		{
			T element{};
			DeserializeBinary(stream, &element);
			outVal->insert(element);
		}
	}

	/** UNORDERED MULTI SET */
	template <typename T, typename Hasher, typename Keyeq, typename Alloc>
	void BinarySerializer<std::unordered_multiset<T, Hasher, Keyeq, Alloc>>::Serialize(std::ostream* stream, const std::unordered_multiset<T, Hasher, Keyeq, Alloc>* inVal)
	{
		size_t size = inVal->size();

		WriteStream(stream, &size);

		for (auto& element : *inVal)
			SerializeBinary(stream, &element);
	}

	template <typename T, typename Hasher, typename Keyeq, typename Alloc>
	void BinarySerializer<std::unordered_multiset<T, Hasher, Keyeq, Alloc>>::Deserialize(std::istream* stream, std::unordered_multiset<T, Hasher, Keyeq, Alloc>* outVal)
	{
		size_t size = ReadStream<size_t>(stream);

		outVal->clear();

		for (size_t i{}; i < size; ++i)
		{
			T element{};
			DeserializeBinary(stream, &element);
			outVal->insert(element);
		}
	}
#endif

#ifdef _MAP_
	/** MAP */
	template <typename KeyEvent, typename Value, typename P, typename Alloc>
	void BinarySerializer<std::map<KeyEvent, Value, P, Alloc>>::Serialize(std::ostream* stream, const std::map<KeyEvent, Value, P, Alloc>* inVal)
	{
		size_t size = inVal->size();

		WriteStream(stream, &size);

		for (auto& element : *inVal)
			SerializeBinary(stream, &element);
	}

	template <typename KeyEvent, typename Value, typename P, typename Alloc>
	void BinarySerializer<std::map<KeyEvent, Value, P, Alloc>>::Deserialize(std::istream* stream, std::map<KeyEvent, Value, P, Alloc>* outVal)
	{
		size_t size = ReadStream<size_t>(stream);

		outVal->clear();

		for (size_t i{}; i < size; ++i)
		{
			std::pair<KeyEvent, Value> element{};
			DeserializeBinary(stream, &element);
			outVal->insert(element);
		}
	}

	/** MULTI MAP */
	template <typename KeyEvent, typename Value, typename P, typename Alloc>
	void BinarySerializer<std::multimap<KeyEvent, Value, P, Alloc>>::Serialize(std::ostream* stream, const std::multimap<KeyEvent, Value, P, Alloc>* inVal)
	{
		size_t size = inVal->size();

		WriteStream(stream, &size);

		for (auto& element : *inVal)
			SerializeBinary(stream, &element);
	}

	template <typename KeyEvent, typename Value, typename P, typename Alloc>
	void BinarySerializer<std::multimap<KeyEvent, Value, P, Alloc>>::Deserialize(std::istream* stream, std::multimap<KeyEvent, Value, P, Alloc>* outVal)
	{
		size_t size = ReadStream<size_t>(stream);

		outVal->clear();

		for (size_t i{}; i < size; ++i)
		{
			std::pair<KeyEvent, Value> element{};
			DeserializeBinary(stream, &element);
			outVal->insert(element);
		}
	}
#endif

#ifdef _UNORDERED_MAP_
	/** UNORDERED MAP */
	template <typename KeyEvent, typename Value, typename Hasher, typename Keyeq, typename Alloc>
	void BinarySerializer<std::unordered_map<KeyEvent, Value, Hasher, Keyeq, Alloc>>::Serialize(std::ostream* stream, const std::unordered_map<KeyEvent, Value, Hasher, Keyeq, Alloc>* inVal)
	{
		size_t size = inVal->size();

		WriteStream(stream, &size);

		for (auto& element : *inVal)
			SerializeBinary(stream, &element);
	}

	template <typename KeyEvent, typename Value, typename Hasher, typename Keyeq, typename Alloc>
	void BinarySerializer<std::unordered_map<KeyEvent, Value, Hasher, Keyeq, Alloc>>::Deserialize(std::istream* stream, std::unordered_map<KeyEvent, Value, Hasher, Keyeq, Alloc>* outVal)
	{
		size_t size = ReadStream<size_t>(stream);

		outVal->clear();

		for (size_t i{}; i < size; ++i)
		{
			std::pair<KeyEvent, Value> element{};
			DeserializeBinary(stream, &element);
			outVal->insert(element);
		}
	}

	/** UNORDERED MULTI MAP */
	template <typename KeyEvent, typename Value, typename Hasher, typename Keyeq, typename Alloc>
	void BinarySerializer<std::unordered_multimap<KeyEvent, Value, Hasher, Keyeq, Alloc>>::Serialize(std::ostream* stream, const std::unordered_multimap<KeyEvent, Value, Hasher, Keyeq, Alloc>* inVal)
	{
		size_t size = inVal->size();

		WriteStream(stream, &size);

		for (auto& element : *inVal)
			SerializeBinary(stream, &element);
	}

	template <typename KeyEvent, typename Value, typename Hasher, typename Keyeq, typename Alloc>
	void BinarySerializer<std::unordered_multimap<KeyEvent, Value, Hasher, Keyeq, Alloc>>::Deserialize(std::istream* stream, std::unordered_multimap<KeyEvent, Value, Hasher, Keyeq, Alloc>* outVal)
	{
		size_t size = ReadStream<size_t>(stream);

		outVal->clear();

		for (size_t i{}; i < size; ++i)
		{
			std::pair<KeyEvent, Value> element{};
			DeserializeBinary(stream, &element);
			outVal->insert(element);
		}
	}
#endif

#ifdef _MEMORY_
	/** UNIQUE PTR */
	template <typename T, typename Delete>
	void BinarySerializer<std::unique_ptr<T, Delete>>::Serialize(std::ostream* stream, const std::unique_ptr<T, Delete>* inVal)
	{
		bool hasValue = inVal && *inVal;
		WriteStream(stream, &hasValue);
		if (hasValue)
			SerializeBinary(stream, inVal->get());
	}

	template <typename T, typename Delete>
	void BinarySerializer<std::unique_ptr<T, Delete>>::Deserialize(std::istream* stream, std::unique_ptr<T, Delete>* outVal)
	{
		if (ReadStream<bool>(stream))
		{
			*outVal = std::make_unique<T>();
			DeserializeBinary(stream, outVal->get());
		}
		else
		{
			outVal->reset();
		}
	}
#endif

#ifdef _OPTIONAL_
	/** OPTIONAL */
	template <typename T>
	void BinarySerializer<std::optional<T>>::Serialize(std::ostream* stream, const std::optional<T>* inVal)
	{
		bool hasValue = inVal && inVal->has_value();
		WriteStream(stream, &hasValue);
		if (hasValue)
		{
			SerializeBinary(stream, &inVal->value());
		}
	}

	template <typename T>
	void BinarySerializer<std::optional<T>>::Deserialize(std::istream* stream, std::optional<T>* outVal)
	{
		if (ReadStream<bool>(stream))
		{
			auto& val = outVal->emplace();
			DeserializeBinary(stream, &val);
		}
		else
		{
			outVal->reset();
		}
	}
#endif

#ifdef _UTILITY_
	/** PAIR */
	template <typename T1, typename T2>
	void BinarySerializer<std::pair<T1, T2>>::Serialize(std::ostream* stream, const std::pair<T1, T2>* inVal)
	{
		SerializeBinary(stream, &inVal->first);
		SerializeBinary(stream, &inVal->second);
	}

	template <typename T1, typename T2>
	void BinarySerializer<std::pair<T1, T2>>::Deserialize(std::istream* stream, std::pair<T1, T2>* outVal)
	{
		DeserializeBinary(stream, &outVal->first);
		DeserializeBinary(stream, &outVal->second);
	}

	/** TUPLE - 辅助函数 */
	template <size_t Index, typename Tuple>
	void SerializeBinaryTuple(std::ostream* stream, const Tuple* tuple)
	{
		using Type = std::tuple_element_t<Index, Tuple>;
		constexpr size_t size = std::tuple_size_v<Tuple>;

		SerializeBinary(stream, &std::get<Index>(*tuple));

		if constexpr (Index + 1 < size)
		{
			SerializeBinaryTuple<Index + 1, Tuple>(stream, tuple);
		}
	}

	template <size_t Index, typename Tuple>
	void DeserializeBinaryTuple(std::istream* stream, Tuple* tuple)
	{
		using Type = std::tuple_element_t<Index, Tuple>;
		constexpr size_t size = std::tuple_size_v<Tuple>;

		DeserializeBinary(stream, &std::get<Index>(*tuple));

		if constexpr (Index + 1 < size)
		{
			DeserializeBinaryTuple<Index + 1, Tuple>(stream, tuple);
		}
	}

	template <typename ... Ts>
	void BinarySerializer<std::tuple<Ts...>>::Serialize(std::ostream* stream, const std::tuple<Ts...>* inVal)
	{
		SerializeBinaryTuple<0>(stream, inVal);
	}

	template <typename ... Ts>
	void BinarySerializer<std::tuple<Ts...>>::Deserialize(std::istream* stream, std::tuple<Ts...>* outVal)
	{
		DeserializeBinaryTuple<0>(stream, outVal);
	}
#endif
}