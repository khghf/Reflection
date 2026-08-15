#pragma once
#include"Serialization_Json.h"
#include<iostream>
#include"../Tool/Util.h"

namespace mirror::Serialization
{
	template <typename T>
	constexpr void FillTypeInfoJson(TypeInfo& info)
	{
		if constexpr (std::is_same_v<void, T>)
		{
			info.JsonSerializer = nullptr;
			info.JsonDeserializer = nullptr;
		}
		else if constexpr (CustomJsonSerializer<T>)
		{
			info.JsonSerializer = [](rapidjson::Value* jsonVal, const void* data, RapidJsonAllocator* allocator)
				{
					JsonSerializer<T>::Serialize(jsonVal, static_cast<const T*>(data), allocator);
				};

			info.JsonDeserializer = [](rapidjson::Value* jsonVal, void* data)
				{
					JsonSerializer<T>::Deserialize(jsonVal, static_cast<T*>(data));
				};
		}
		else
		{
			info.JsonSerializer = [](rapidjson::Value* jsonVal, const void* data, RapidJsonAllocator* allocator)
				{
					SerializeJsonDefault(jsonVal, static_cast<const T*>(data), allocator);
				};

			info.JsonDeserializer = [](rapidjson::Value* jsonVal, void* data)
				{
					DeserializeJsonDefault(jsonVal, static_cast<T*>(data));
				};
		}
	}


	template<typename T>
	void SerializeJson(std::string_view outFilePath, const T* inVal)
	{
		std::ofstream outSteram(outFilePath.data(), std::ios::out | std::ios::trunc);
		SerializeJson(&outSteram, inVal);
		outSteram.close();
	}

	template <typename T>
	void SerializeJson(std::ostream* stream, const T* inVal)
	{
		rapidjson::Document document;
		document.SetObject();
		TypeId typeId = TypeId::Create<T>();
		auto& info = typeId.GetInfo();
		TypeInfo::RapidJsonAllocator allocator{};

		SerializeJson(&document, inVal, &allocator);

		if (document.HasParseError())
		{
			std::cout << "Error " << document.GetParseError();
		}

		rapidjson::OStreamWrapper streamWrapper(*stream);

		rapidjson::PrettyWriter prettyWriter{ streamWrapper };
		document.Accept(prettyWriter);
	}

	template <typename T>
	void SerializeJson(rapidjson::Value* jsonVal, const T* inVal, RapidJsonAllocator* allocator)
	{
		if constexpr (CustomJsonSerializer<T>)JsonSerializer<T>::Serialize(jsonVal, inVal, allocator);
		else SerializeJsonDefault(jsonVal, inVal, allocator);
	}

	template <typename T>
	void SerializeJsonDefault(rapidjson::Value* jsonVal, const T* inVal, RapidJsonAllocator* allocator)
	{
		TypeId actualTypeId = TypeId::Create<T>();

		if constexpr (std::is_enum_v<T>)
		{
			using underType = std::underlying_type_t<T>;

			underType underVal = static_cast<underType>(*inVal);

			JsonSerializer<underType>::Serialize(jsonVal, &underVal,allocator);

		}
		else
		{
			if constexpr (std::is_polymorphic_v<T>)
			{
				const TypeId& idFromVTable = GetTypeId(GetVTable(inVal));

				//根据虚表获取实际类型
				if (actualTypeId != idFromVTable)
					actualTypeId = idFromVTable;

				const TypeInfo& actualInfo = actualTypeId.GetInfo();

				//调用父类的序列化函数
				for (const auto& baseInfo : actualInfo.BaseClasses)
				{
					SerializeJsonDefault(jsonVal, static_cast<const void*>(inVal), baseInfo.BaseId, allocator);
				}
			}
			SerializeJsonDefault(jsonVal, static_cast<const void*>(inVal), actualTypeId, allocator);
		}
	}

	inline void SerializeJsonDefault(rapidjson::Value* jsonVal, const void* inVal, mirror::TypeId type, RapidJsonAllocator* allocator)
	{
		auto& info = type.GetInfo();

		auto& members = info.Members;

		for (auto& member : members)
		{
			if (!member.Variable.IsRefOrPointer())
			{
				rapidjson::Value jsonValToAdd{ rapidjson::kObjectType };
				auto& memberInfo = member.Variable.GetTypeId().GetInfo();
				memberInfo.JsonSerializer(&jsonValToAdd, util::VoidOffset(inVal, member.Offset), allocator);
				jsonVal->AddMember(rapidjson::GenericStringRef{ member.Name.c_str() }, jsonValToAdd, *allocator);
			}
		}
	}





	template<typename T>
	void DeserializeJson(std::string_view inFilePath, T* outVal)
	{
		std::ifstream inStream(inFilePath.data(), std::ios::in);
		DeserializeJson(&inStream, outVal);
		inStream.close();
	}

	template <typename T>
	void DeserializeJson(std::istream* stream, T* outVal)
	{
		rapidjson::Document document;
		document.SetObject();

		rapidjson::IStreamWrapper streamWrapper(*stream);
		document.ParseStream(streamWrapper);

		DeserializeJson(&document, outVal);

		if (document.HasParseError())
		{
			std::cout << "Error " << document.GetParseError();
		}
	}

	template <typename T>
	void DeserializeJson(rapidjson::Value* jsonVal, T* outVal)
	{
		if constexpr (CustomJsonSerializer<T>)JsonSerializer<T>::Deserialize(jsonVal, outVal);
		else DeserializeJsonDefault(jsonVal, outVal);
	}

	template <typename T>
	void DeserializeJsonDefault(rapidjson::Value* jsonVal, T* outVal)
	{
		constexpr TypeId typeId = TypeId::Create<T>();
		const TypeInfo& typeInfo = typeId.GetInfo();

		if constexpr (std::is_enum_v<T>&& typeId.IsValid())
		{
			using underType = std::underlying_type_t<T>;
			underType underVal = static_cast<underType>(*outVal);

			JsonSerializer<underType>::Deserialize(jsonVal, &underVal);
			*outVal = static_cast<T>(underVal);
		}
		else
		{
			for (const auto& baseInfo : typeInfo.BaseClasses)
			{
				DeserializeJsonDefault(jsonVal, static_cast<void*>(outVal), baseInfo.BaseId);
			}
			DeserializeJsonDefault(jsonVal, static_cast<void*>(outVal), typeId);
		}
	}

	inline void DeserializeJsonDefault(rapidjson::Value* jsonVal, void* outVal, mirror::TypeId type)
	{
		auto& info = type.GetInfo();
		auto& members = info.Members;

		for (auto& member : members)
		{
			if (!member.Variable.IsRefOrPointer())
			{
				auto jsonMemberVal = jsonVal->FindMember(member.Name.c_str());

				if (jsonMemberVal != jsonVal->MemberEnd())
				{
					auto& memberInfo = member.Variable.GetTypeId().GetInfo();
					memberInfo.JsonDeserializer(&jsonMemberVal->value, util::VoidOffset(outVal, member.Offset));
				}
			}
		}
	}

	///** TypeId */
	//inline void JsonSerializer<TypeId>::Serialize(rapidjson::Value* jsonVal, const TypeId* inVal, RapidJsonAllocator*)
	//{
	//	jsonVal->SetUint64(inVal->GetId());
	//}

	//inline void JsonSerializer<TypeId>::Deserialize(rapidjson::Value* jsonVal, TypeId* outVal)
	//{
	//	outVal->SetTypeId(jsonVal->GetUint64());
	//}

	///** VariableId */
	//inline void JsonSerializer<VariableId>::Serialize(rapidjson::Value* jsonVal, const VariableId* inVal, RapidJsonAllocator* allocator)
	//{
	//	rapidjson::Value typeVal{};
	//	rapidjson::Value constFlag{};
	//	rapidjson::Value volatileFlag{};
	//	rapidjson::Value referenceFlag{};
	//	rapidjson::Value rValReferenceFlag{};
	//	rapidjson::Value pointerAmountFlag{};
	//	rapidjson::Value arraySizeFlag{};

	//	typeVal.SetUint64(inVal->GetTypeId().GetId());
	//	constFlag.SetBool(inVal->IsConst());
	//	volatileFlag.SetBool(inVal->IsVolatile());
	//	referenceFlag.SetBool(inVal->IsReference());
	//	rValReferenceFlag.SetBool(inVal->IsRValReference());
	//	pointerAmountFlag.SetUint(inVal->GetPointerAmount());
	//	arraySizeFlag.SetUint(inVal->GetArraySize());

	//	jsonVal->AddMember("Type", typeVal, *allocator);
	//	jsonVal->AddMember("Const", constFlag, *allocator);
	//	jsonVal->AddMember("Volatile", volatileFlag, *allocator);
	//	jsonVal->AddMember("Reference", referenceFlag, *allocator);
	//	jsonVal->AddMember("R Value", rValReferenceFlag, *allocator);
	//	jsonVal->AddMember("Pointer Amount", pointerAmountFlag, *allocator);
	//	jsonVal->AddMember("Array Size", arraySizeFlag, *allocator);
	//}

	//inline void JsonSerializer<VariableId>::Deserialize(rapidjson::Value* jsonVal, VariableId* outVal)
	//{
	//	auto typeVal = jsonVal->FindMember("Type");
	//	auto constVal = jsonVal->FindMember("Const");
	//	auto volatileVal = jsonVal->FindMember("Volatile");
	//	auto referenceVal = jsonVal->FindMember("Reference");
	//	auto rValRefVal = jsonVal->FindMember("R Value");
	//	auto pointerVal = jsonVal->FindMember("Pointer Amount");
	//	auto arrayVal = jsonVal->FindMember("Array Size");

	//	if (typeVal != jsonVal->MemberEnd())
	//	{
	//		TypeId id{};
	//		JsonSerializer<TypeId>::Deserialize(&typeVal->value, &id);
	//		outVal->SetTypeId(id);
	//	}
	//	if (constVal != jsonVal->MemberEnd())
	//	{
	//		if (constVal->value.GetBool()) outVal->SetConstFlag();
	//		else outVal->RemoveConstFlag();
	//	}
	//	if (volatileVal != jsonVal->MemberEnd())
	//	{
	//		if (volatileVal->value.GetBool()) outVal->SetVolatileFlag();
	//		else outVal->RemoveVolatileFlag();
	//	}
	//	if (referenceVal != jsonVal->MemberEnd())
	//	{
	//		if (referenceVal->value.GetBool()) outVal->SetReferenceFlag();
	//		else outVal->RemoveReferenceFlag();
	//	}
	//	if (rValRefVal != jsonVal->MemberEnd())
	//	{
	//		if (rValRefVal->value.GetBool()) outVal->SetRValReferenceFlag();
	//		else outVal->RemoveRValReferenceFlag();
	//	}
	//	if (pointerVal != jsonVal->MemberEnd())
	//	{
	//		outVal->SetPointerAmount(static_cast<uint16_t>(pointerVal->value.GetUint()));
	//	}
	//	if (arrayVal != jsonVal->MemberEnd())
	//	{
	//		outVal->SetArraySize(arrayVal->value.GetUint());
	//	}
	//}

	///** FunctionId */
	//inline void JsonSerializer<FunctionId>::Serialize(rapidjson::Value* jsonVal, const FunctionId* inVal, RapidJsonAllocator*)
	//{
	//	jsonVal->SetUint64(inVal->GetId());
	//}

	//inline void JsonSerializer<FunctionId>::Deserialize(rapidjson::Value* jsonVal, FunctionId* outVal)
	//{
	//	outVal->SetId(jsonVal->GetUint64());
	//}


#ifdef  GLM_ENABLE

	inline void JsonSerializer<glm::vec1>::Serialize(rapidjson::Value* jsonVal, const glm::vec1* inVal, RapidJsonAllocator* allocator)
	{
		jsonVal->SetArray();

		rapidjson::Value x{ rapidjson::kObjectType };

		SerializeJson(&x, &inVal->x, allocator);

		jsonVal->PushBack(x, *allocator);
	}
	inline void JsonSerializer<glm::vec1>::Deserialize(rapidjson::Value* jsonVal, glm::vec1* outVal)
	{
		const auto& array = jsonVal->GetArray();

		DeserializeJson(&array[0], &outVal->x);
	}


	inline void JsonSerializer<glm::vec2>::Serialize(rapidjson::Value* jsonVal, const glm::vec2* inVal, RapidJsonAllocator* allocator)
	{
		jsonVal->SetArray();

		rapidjson::Value x{ rapidjson::kObjectType };
		rapidjson::Value y{ rapidjson::kObjectType };

		SerializeJson(&x, &inVal->x, allocator);
		SerializeJson(&y, &inVal->y, allocator);

		jsonVal->PushBack(x, *allocator);
		jsonVal->PushBack(y, *allocator);
	}
	inline void JsonSerializer<glm::vec2>::Deserialize(rapidjson::Value* jsonVal, glm::vec2* outVal)
	{
		const auto& array = jsonVal->GetArray();

		DeserializeJson(&array[0], &outVal->x);
		DeserializeJson(&array[1], &outVal->y);
	}
	

	inline void JsonSerializer<glm::vec3>::Serialize(rapidjson::Value* jsonVal, const glm::vec3* inVal, RapidJsonAllocator* allocator)
	{
		jsonVal->SetArray();

		rapidjson::Value x{ rapidjson::kObjectType };
		rapidjson::Value y{ rapidjson::kObjectType };
		rapidjson::Value z{ rapidjson::kObjectType };

		SerializeJson(&x, &inVal->x, allocator);
		SerializeJson(&y, &inVal->y, allocator);
		SerializeJson(&z, &inVal->z, allocator);

		jsonVal->PushBack(x, *allocator);
		jsonVal->PushBack(y, *allocator);
		jsonVal->PushBack(z, *allocator);
	}
	inline void JsonSerializer<glm::vec3>::Deserialize(rapidjson::Value* jsonVal, glm::vec3* outVal)
	{
		const auto& array = jsonVal->GetArray();

		DeserializeJson(&array[0], &outVal->x);
		DeserializeJson(&array[1], &outVal->y);
		DeserializeJson(&array[2], &outVal->z);
	}


	inline void JsonSerializer<glm::vec4>::Serialize(rapidjson::Value* jsonVal, const glm::vec4* inVal, RapidJsonAllocator* allocator)
	{
		jsonVal->SetArray();

		rapidjson::Value x{ rapidjson::kObjectType };
		rapidjson::Value y{ rapidjson::kObjectType };
		rapidjson::Value z{ rapidjson::kObjectType };
		rapidjson::Value w{ rapidjson::kObjectType };

		SerializeJson(&x, &inVal->x, allocator);
		SerializeJson(&y, &inVal->y, allocator);
		SerializeJson(&z, &inVal->z, allocator);
		SerializeJson(&w, &inVal->w, allocator);

		jsonVal->PushBack(x, *allocator);
		jsonVal->PushBack(y, *allocator);
		jsonVal->PushBack(z, *allocator);
		jsonVal->PushBack(w, *allocator);
	}
	inline void JsonSerializer<glm::vec4>::Deserialize(rapidjson::Value* jsonVal, glm::vec4* outVal)
	{
		const auto& array = jsonVal->GetArray();

		DeserializeJson(&array[0], &outVal->x);
		DeserializeJson(&array[1], &outVal->y);
		DeserializeJson(&array[2], &outVal->z);
		DeserializeJson(&array[3], &outVal->w);
	}

#endif 


	// ============ 基础类型特化实现 ============

	/** FLOAT */
	inline void JsonSerializer<float>::Serialize(rapidjson::Value* jsonVal, const float* inVal, RapidJsonAllocator*)
	{
		jsonVal->SetFloat(*inVal);
	}

	inline void JsonSerializer<float>::Deserialize(rapidjson::Value* jsonVal, float* outVal)
	{
		*outVal = jsonVal->GetFloat();
	}

	/** DOUBLE */
	inline void JsonSerializer<double>::Serialize(rapidjson::Value* jsonVal, const double* inVal, RapidJsonAllocator*)
	{
		jsonVal->SetDouble(*inVal);
	}

	inline void JsonSerializer<double>::Deserialize(rapidjson::Value* jsonVal, double* outVal)
	{
		*outVal = jsonVal->GetDouble();
	}

	/** INT 8 */
	inline void JsonSerializer<int8_t>::Serialize(rapidjson::Value* jsonVal, const int8_t* inVal, RapidJsonAllocator*)
	{
		jsonVal->SetInt(*inVal);
	}

	inline void JsonSerializer<int8_t>::Deserialize(rapidjson::Value* jsonVal, int8_t* outVal)
	{
		*outVal = static_cast<int8_t>(jsonVal->GetInt());
	}

	/** INT 16 */
	inline void JsonSerializer<int16_t>::Serialize(rapidjson::Value* jsonVal, const int16_t* inVal, RapidJsonAllocator*)
	{
		jsonVal->SetInt(*inVal);
	}

	inline void JsonSerializer<int16_t>::Deserialize(rapidjson::Value* jsonVal, int16_t* outVal)
	{
		*outVal = static_cast<int16_t>(jsonVal->GetInt());
	}

	/** INT 32 */
	inline void JsonSerializer<int32_t>::Serialize(rapidjson::Value* jsonVal, const int32_t* inVal, RapidJsonAllocator*)
	{
		jsonVal->SetInt(*inVal);
	}

	inline void JsonSerializer<int32_t>::Deserialize(rapidjson::Value* jsonVal, int32_t* outVal)
	{
		*outVal = jsonVal->GetInt();
	}

	/** INT 64 */
	inline void JsonSerializer<int64_t>::Serialize(rapidjson::Value* jsonVal, const int64_t* inVal, RapidJsonAllocator*)
	{
		jsonVal->SetInt64(*inVal);
	}

	inline void JsonSerializer<int64_t>::Deserialize(rapidjson::Value* jsonVal, int64_t* outVal)
	{
		*outVal = jsonVal->GetInt64();
	}

	/** UINT 8 */
	inline void JsonSerializer<uint8_t>::Serialize(rapidjson::Value* jsonVal, const uint8_t* inVal, RapidJsonAllocator*)
	{
		jsonVal->SetUint(*inVal);
	}

	inline void JsonSerializer<uint8_t>::Deserialize(rapidjson::Value* jsonVal, uint8_t* outVal)
	{
		*outVal = static_cast<uint8_t>(jsonVal->GetUint());
	}

	/** UINT 16 */
	inline void JsonSerializer<uint16_t>::Serialize(rapidjson::Value* jsonVal, const uint16_t* inVal, RapidJsonAllocator*)
	{
		jsonVal->SetUint(*inVal);
	}

	inline void JsonSerializer<uint16_t>::Deserialize(rapidjson::Value* jsonVal, uint16_t* outVal)
	{
		*outVal = static_cast<uint16_t>(jsonVal->GetUint());
	}

	/** UINT 32 */
	inline void JsonSerializer<uint32_t>::Serialize(rapidjson::Value* jsonVal, const uint32_t* inVal, RapidJsonAllocator*)
	{
		jsonVal->SetUint(*inVal);
	}

	inline void JsonSerializer<uint32_t>::Deserialize(rapidjson::Value* jsonVal, uint32_t* outVal)
	{
		*outVal = jsonVal->GetUint();
	}

	/** UINT 64 */
	inline void JsonSerializer<uint64_t>::Serialize(rapidjson::Value* jsonVal, const uint64_t* inVal, RapidJsonAllocator*)
	{
		jsonVal->SetUint64(*inVal);
	}

	inline void JsonSerializer<uint64_t>::Deserialize(rapidjson::Value* jsonVal, uint64_t* outVal)
	{
		*outVal = jsonVal->GetUint64();
	}

	/** BOOL */
	inline void JsonSerializer<bool>::Serialize(rapidjson::Value* jsonVal, const bool* inVal, RapidJsonAllocator*)
	{
		jsonVal->SetBool(*inVal);
	}

	inline void JsonSerializer<bool>::Deserialize(rapidjson::Value* jsonVal, bool* outVal)
	{
		*outVal = jsonVal->GetBool();
	}




	/** CHAR* */
	inline void JsonSerializer<char*>::Serialize(rapidjson::Value* jsonVal, const char** inVal, RapidJsonAllocator* allocator)
	{
		jsonVal->SetString(*inVal, *allocator);
	}

	inline void JsonSerializer<char*>::Deserialize(rapidjson::Value* jsonVal, const char** outVal)
	{
		*outVal = jsonVal->GetString();
	}

	

	// ============ 容器序列化辅助函数 ============

	template <typename T, typename Container>
	void SerializeJsonContainer(rapidjson::Value* jsonVal, const Container* container, RapidJsonAllocator* allocator)
	{
		jsonVal->SetArray();

		for (auto& element : *container)
		{
			auto arrayElement = rapidjson::Value(rapidjson::kObjectType);
			SerializeJson(&arrayElement, &element, allocator);
			jsonVal->PushBack(arrayElement, *allocator);
		}
	}

	template <typename T, typename Adder>
	void DeserializeJsonContainer(rapidjson::Value* jsonVal, Adder adder)
	{
		for (auto& element : jsonVal->GetArray())
		{
			T value{};
			DeserializeJson(&element, &value);
			adder(value);
		}
	}

	// ============ C-ARRAY 特化实现 ============

	template <typename T>
	void JsonSerializer<T[]>::Serialize(rapidjson::Value* jsonVal, const T* inVal, RapidJsonAllocator* allocator)
	{
		SerializeJsonContainer<T>(jsonVal, inVal, allocator);
	}

	template <typename T>
	void JsonSerializer<T[]>::Deserialize(rapidjson::Value* jsonVal, T* outVal)
	{
		size_t counter{};
		for (auto& element : jsonVal->GetArray())
		{
			DeserializeJson(&element, &outVal[counter]);
			++counter;
		}
	}

	// ============ STL 容器特化实现 ============

#ifdef _STRING_
	/** STRING */
	template <typename Elem, typename Traits, typename Alloc>
	void JsonSerializer<std::basic_string<Elem, Traits, Alloc>>::Serialize(
		rapidjson::Value* jsonVal, const std::basic_string<Elem, Traits, Alloc>* inVal, RapidJsonAllocator* allocator)
	{
		jsonVal->SetString(rapidjson::GenericStringRef(inVal->c_str()), *allocator);
	}

	template <typename Elem, typename Traits, typename Alloc>
	void JsonSerializer<std::basic_string<Elem, Traits, Alloc>>::Deserialize(
		rapidjson::Value* jsonVal, std::basic_string<Elem, Traits, Alloc>* outVal)
	{
		*outVal = jsonVal->GetString();
	}
#endif

#ifdef _VECTOR_
	/** VECTOR */
	template <typename T, typename Alloc>
	void JsonSerializer<std::vector<T, Alloc>>::Serialize(
		rapidjson::Value* jsonVal, const std::vector<T, Alloc>* inVal, RapidJsonAllocator* allocator)
	{
		SerializeJsonContainer<T>(jsonVal, inVal, allocator);
	}

	template <typename T, typename Alloc>
	void JsonSerializer<std::vector<T, Alloc>>::Deserialize(
		rapidjson::Value* jsonVal, std::vector<T, Alloc>* outVal)
	{
		DeserializeJsonContainer<T>(jsonVal, [&outVal](T& element) { outVal->push_back(element); });
	}
#endif

#ifdef _ARRAY_
	/** ARRAY */
	template <typename T, size_t Size>
	void JsonSerializer<std::array<T, Size>>::Serialize(
		rapidjson::Value* jsonVal, const std::array<T, Size>* inVal, RapidJsonAllocator* allocator)
	{
		SerializeJsonContainer<T>(jsonVal, inVal, allocator);
	}

	template <typename T, size_t Size>
	void JsonSerializer<std::array<T, Size>>::Deserialize(
		rapidjson::Value* jsonVal, std::array<T, Size>* outVal)
	{
		size_t counter{};
		for (auto& element : jsonVal->GetArray())
		{
			DeserializeJson(&element, &(*outVal)[counter]);
			++counter;
		}
	}
#endif

#ifdef _DEQUE_
	/** DEQUE */
	template <typename T, typename Alloc>
	void JsonSerializer<std::deque<T, Alloc>>::Serialize(
		rapidjson::Value* jsonVal, const std::deque<T, Alloc>* inVal, RapidJsonAllocator* allocator)
	{
		SerializeJsonContainer<T>(jsonVal, inVal, allocator);
	}

	template <typename T, typename Alloc>
	void JsonSerializer<std::deque<T, Alloc>>::Deserialize(
		rapidjson::Value* jsonVal, std::deque<T, Alloc>* outVal)
	{
		DeserializeJsonContainer<T>(jsonVal, [&outVal](T& element) { outVal->push_back(element); });
	}
#endif

#ifdef _FORWARD_LIST_
	/** FORWARD LIST */
	template <typename T, typename Alloc>
	void JsonSerializer<std::forward_list<T, Alloc>>::Serialize(
		rapidjson::Value* jsonVal, const std::forward_list<T, Alloc>* inVal, RapidJsonAllocator* allocator)
	{
		SerializeJsonContainer<T>(jsonVal, inVal, allocator);
	}

	template <typename T, typename Alloc>
	void JsonSerializer<std::forward_list<T, Alloc>>::Deserialize(
		rapidjson::Value* jsonVal, std::forward_list<T, Alloc>* outVal)
	{
		DeserializeJsonContainer<T>(jsonVal, [&outVal](T& element) { outVal->push_front(element); });
	}
#endif

#ifdef _LIST_
	/** LIST */
	template <typename T, typename Alloc>
	void JsonSerializer<std::list<T, Alloc>>::Serialize(
		rapidjson::Value* jsonVal, const std::list<T, Alloc>* inVal, RapidJsonAllocator* allocator)
	{
		SerializeJsonContainer<T>(jsonVal, inVal, allocator);
	}

	template <typename T, typename Alloc>
	void JsonSerializer<std::list<T, Alloc>>::Deserialize(
		rapidjson::Value* jsonVal, std::list<T, Alloc>* outVal)
	{
		DeserializeJsonContainer<T>(jsonVal, [&outVal](T& element) { outVal->push_back(element); });
	}
#endif

#ifdef _SET_
	/** UNORDERED SET */
	template <typename T, typename Hasher, typename Keyeq, typename Alloc>
	void JsonSerializer<std::unordered_set<T, Hasher, Keyeq, Alloc>>::Serialize(
		rapidjson::Value* jsonVal, const std::unordered_set<T, Hasher, Keyeq, Alloc>* inVal, RapidJsonAllocator* allocator)
	{
		SerializeJsonContainer<T>(jsonVal, inVal, allocator);
	}

	template <typename T, typename Hasher, typename Keyeq, typename Alloc>
	void JsonSerializer<std::unordered_set<T, Hasher, Keyeq, Alloc>>::Deserialize(
		rapidjson::Value* jsonVal, std::unordered_set<T, Hasher, Keyeq, Alloc>* outVal)
	{
		DeserializeJsonContainer<T>(jsonVal, [&outVal](T& element) { outVal->insert(element); });
	}

	template <typename T, typename Hasher, typename Keyeq, typename Alloc>
	void JsonSerializer<std::unordered_multiset<T, Hasher, Keyeq, Alloc>>::Serialize(
		rapidjson::Value* jsonVal, const std::unordered_multiset<T, Hasher, Keyeq, Alloc>* inVal, RapidJsonAllocator* allocator)
	{
		SerializeJsonContainer<T>(jsonVal, inVal, allocator);
	}

	template <typename T, typename Hasher, typename Keyeq, typename Alloc>
	void JsonSerializer<std::unordered_multiset<T, Hasher, Keyeq, Alloc>>::Deserialize(
		rapidjson::Value* jsonVal, std::unordered_multiset<T, Hasher, Keyeq, Alloc>* outVal)
	{
		DeserializeJsonContainer<T>(jsonVal, [&outVal](T& element) { outVal->insert(element); });
	}
#endif

#ifdef _MAP_
	/** MAP */
	template <typename KeyEvent, typename Value, typename P, typename Alloc>
	void JsonSerializer<std::map<KeyEvent, Value, P, Alloc>>::Serialize(
		rapidjson::Value* jsonVal, const std::map<KeyEvent, Value, P, Alloc>* inVal, RapidJsonAllocator* allocator)
	{
		SerializeJsonContainer<std::pair<KeyEvent, Value>>(jsonVal, inVal, allocator);
	}

	template <typename KeyEvent, typename Value, typename P, typename Alloc>
	void JsonSerializer<std::map<KeyEvent, Value, P, Alloc>>::Deserialize(
		rapidjson::Value* jsonVal, std::map<KeyEvent, Value, P, Alloc>* outVal)
	{
		DeserializeJsonContainer<std::pair<KeyEvent, Value>>(jsonVal, [&outVal](std::pair<KeyEvent, Value>& element) { outVal->insert(element); });
	}

	template <typename KeyEvent, typename Value, typename P, typename Alloc>
	void JsonSerializer<std::multimap<KeyEvent, Value, P, Alloc>>::Serialize(
		rapidjson::Value* jsonVal, const std::multimap<KeyEvent, Value, P, Alloc>* inVal, RapidJsonAllocator* allocator)
	{
		SerializeJsonContainer<std::pair<KeyEvent, Value>>(jsonVal, inVal, allocator);
	}

	template <typename KeyEvent, typename Value, typename P, typename Alloc>
	void JsonSerializer<std::multimap<KeyEvent, Value, P, Alloc>>::Deserialize(
		rapidjson::Value* jsonVal, std::multimap<KeyEvent, Value, P, Alloc>* outVal)
	{
		DeserializeJsonContainer<std::pair<KeyEvent, Value>>(jsonVal, [&outVal](std::pair<KeyEvent, Value>& element) { outVal->insert(element); });
	}
#endif

#ifdef _UNORDERED_MAP_
	/** UNORDERED MAP */
	template <typename KeyEvent, typename Value, typename Hasher, typename Keyeq, typename Alloc>
	void JsonSerializer<std::unordered_map<KeyEvent, Value, Hasher, Keyeq, Alloc>>::Serialize(
		rapidjson::Value* jsonVal, const std::unordered_map<KeyEvent, Value, Hasher, Keyeq, Alloc>* inVal, RapidJsonAllocator* allocator)
	{
		SerializeJsonContainer<std::pair<KeyEvent, Value>>(jsonVal, inVal, allocator);
	}

	template <typename KeyEvent, typename Value, typename Hasher, typename Keyeq, typename Alloc>
	void JsonSerializer<std::unordered_map<KeyEvent, Value, Hasher, Keyeq, Alloc>>::Deserialize(
		rapidjson::Value* jsonVal, std::unordered_map<KeyEvent, Value, Hasher, Keyeq, Alloc>* outVal)
	{
		DeserializeJsonContainer<std::pair<KeyEvent, Value>>(jsonVal, [&outVal](std::pair<KeyEvent, Value>& element) { outVal->insert(element); });
	}

	template <typename KeyEvent, typename Value, typename Hasher, typename Keyeq, typename Alloc>
	void JsonSerializer<std::unordered_multimap<KeyEvent, Value, Hasher, Keyeq, Alloc>>::Serialize(
		rapidjson::Value* jsonVal, const std::unordered_multimap<KeyEvent, Value, Hasher, Keyeq, Alloc>* inVal, RapidJsonAllocator* allocator)
	{
		SerializeJsonContainer<std::pair<KeyEvent, Value>>(jsonVal, inVal, allocator);
	}

	template <typename KeyEvent, typename Value, typename Hasher, typename Keyeq, typename Alloc>
	void JsonSerializer<std::unordered_multimap<KeyEvent, Value, Hasher, Keyeq, Alloc>>::Deserialize(
		rapidjson::Value* jsonVal, std::unordered_multimap<KeyEvent, Value, Hasher, Keyeq, Alloc>* outVal)
	{
		DeserializeJsonContainer<std::pair<KeyEvent, Value>>(jsonVal, [&outVal](std::pair<KeyEvent, Value>& element) { outVal->insert(element); });
	}
#endif

#ifdef _MEMORY_
	/** UNIQUE PTR */
	template <typename T, typename Delete>
	void JsonSerializer<std::unique_ptr<T, Delete>>::Serialize(
		rapidjson::Value* jsonVal, const std::unique_ptr<T, Delete>* inVal, RapidJsonAllocator* allocator)
	{
		if (*inVal)
		{
			rapidjson::Value ownedVal{ rapidjson::kObjectType };
			SerializeJson(&ownedVal, inVal->get(), allocator);
			jsonVal->AddMember("Value", ownedVal, *allocator);
		}
		else
		{
			jsonVal->SetNull();
		}
	}

	template <typename T, typename Delete>
	void JsonSerializer<std::unique_ptr<T, Delete>>::Deserialize(
		rapidjson::Value* jsonVal, std::unique_ptr<T, Delete>* outVal)
	{
		if (!jsonVal->IsNull())
		{
			*outVal = std::make_unique<T>();
			DeserializeJson(&(*jsonVal)["Value"], outVal->get());
		}
	}
#endif

#ifdef _OPTIONAL_
	/** OPTIONAL */
	template <typename T>
	void JsonSerializer<std::optional<T>>::Serialize(
		rapidjson::Value* jsonVal, const std::optional<T>* inVal, RapidJsonAllocator* allocator)
	{
		if (*inVal)
		{
			rapidjson::Value ownedVal{ rapidjson::kObjectType };
			SerializeJson(&ownedVal, &inVal->value(), allocator);
			jsonVal->AddMember("Value", ownedVal, *allocator);
		}
		else
		{
			jsonVal->SetNull();
		}
	}

	template <typename T>
	void JsonSerializer<std::optional<T>>::Deserialize(
		rapidjson::Value* jsonVal, std::optional<T>* outVal)
	{
		if (!jsonVal->IsNull())
		{
			auto& val = outVal->emplace();
			DeserializeJson(&(*jsonVal)["Value"], &val);
		}
	}
#endif

#ifdef _UTILITY_
	/** PAIR */
	template <typename T1, typename T2>
	void JsonSerializer<std::pair<T1, T2>>::Serialize(
		rapidjson::Value* jsonVal, const std::pair<T1, T2>* inVal, RapidJsonAllocator* allocator)
	{
		rapidjson::Value Val1{ rapidjson::kObjectType };
		rapidjson::Value Val2{ rapidjson::kObjectType };

		SerializeJson(&Val1, &inVal->first, allocator);
		SerializeJson(&Val2, &inVal->second, allocator);

		jsonVal->AddMember("First", Val1, *allocator);
		jsonVal->AddMember("Second", Val2, *allocator);
	}

	template <typename T1, typename T2>
	void JsonSerializer<std::pair<T1, T2>>::Deserialize(
		rapidjson::Value* jsonVal, std::pair<T1, T2>* outVal)
	{
		DeserializeJson(&(*jsonVal)["First"], &outVal->first);
		DeserializeJson(&(*jsonVal)["Second"], &outVal->second);
	}

	template <size_t Index, typename Tuple>
	void SerializeJsonTuple(rapidjson::Value* jsonVal, const Tuple* tuple, RapidJsonAllocator* allocator)
	{
		using Type = std::tuple_element_t<Index, Tuple>;
		constexpr size_t size = std::tuple_size_v<Tuple>;

		rapidjson::Value name{ std::to_string(Index), *allocator };
		rapidjson::Value val{ rapidjson::kObjectType };

		SerializeJson(&val, &std::get<Index>(*tuple), allocator);

		jsonVal->AddMember(name, val, *allocator);

		if constexpr (Index + 1 < size)
		{
			SerializeJsonTuple<Index + 1, Tuple>(jsonVal, tuple, allocator);
		}
	}

	template <size_t Index, typename Tuple>
	void DeserializeJsonTuple(rapidjson::Value* jsonVal, Tuple* tuple)
	{
		using Type = std::tuple_element_t<Index, Tuple>;
		constexpr size_t size = std::tuple_size_v<Tuple>;

		DeserializeJson(&(*jsonVal)[std::to_string(Index)], &std::get<Index>(*tuple));

		if constexpr (Index + 1 < size)
		{
			DeserializeJsonTuple<Index + 1, Tuple>(jsonVal, tuple);
		}
	}

	template <typename ... Ts>
	void JsonSerializer<std::tuple<Ts...>>::Serialize(
		rapidjson::Value* jsonVal, const std::tuple<Ts...>* inVal, RapidJsonAllocator* allocator)
	{
		SerializeJsonTuple<0>(jsonVal, inVal, allocator);
	}

	template <typename ... Ts>
	void JsonSerializer<std::tuple<Ts...>>::Deserialize(
		rapidjson::Value* jsonVal, std::tuple<Ts...>* outVal)
	{
		DeserializeJsonTuple<0>(jsonVal, outVal);
	}
#endif
}