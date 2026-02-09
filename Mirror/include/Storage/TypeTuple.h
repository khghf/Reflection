#pragma once
#include<span>
#include<tuple>
#include"../Info/MemberInfo.h"
#include<memory>
#include"Storage_Internal.h"
namespace mirror
{
	/*
	类似std::tuple可以存储任何类型实例，所有的数据均存在一块连续内存中，整块内存分为三部分
	第一部分:存储跳转表数据,		0~sizeof(void*)*m_Size
	第二部分:存储VariableId数据,	sizeof(void*)*m_Size~sizeof(void*)*m_Size+sizeof(Variaable)*Variaable.size
	第三部分:存储实例数据,			sizeof(void*)*m_Size+sizeof(Variaable)*Variaable.size~end
	@跳转表:	按顺序指向每个实例数据
	@VariableId:存储了每个实例的大小、内存对齐等信息
	内存分配、跳转表构建@See Initialize
	*/
	class TypeTuple final
	{
	public:
		template<typename ...T>
		TypeTuple(T && ...arg);

		~TypeTuple();

		TypeTuple(const TypeTuple& other) = delete; // TODO
		TypeTuple(TypeTuple&& other) noexcept 
		{
			this->m_Size = other.m_Size;
			this->m_Data = std::move(other.m_Data);
			other.m_Data = nullptr;
			other.m_Size = 0;
		}
		TypeTuple& operator=(const TypeTuple& other) = delete; // TODO
		TypeTuple& operator=(TypeTuple&& other) noexcept
		{
			this->m_Size = other.m_Size;
			this->m_Data = std::move(other.m_Data);
			other.m_Data = nullptr;
			other.m_Size = 0;
			return *this;
		}


		template <typename... T>
		static TypeTuple			Create(const std::tuple<T...>& tuple);

		template <typename... T>
		static TypeTuple			Create(std::tuple<T...>&& tuple);

		template <typename... T>
		static TypeTuple			CreateNoReferences();
		template <typename... T>
		static TypeTuple			CreateNoReferences(T&&... val);

		VariableId					GetVariable(size_t index) const;

		void						SetVariableUnsafe(size_t index, VariableId id);

		void*						GetVoid(size_t index) const;

		template <typename T>
		T&							Get(size_t index);

		template <typename T>
		const T&					Get(size_t index) const;

		constexpr size_t			GetJumpTableSize()const			{ return m_Size * sizeof(void*); }
		constexpr size_t			GetVariableIdsSize()const		{ return m_Size * sizeof(VariableId); }

		constexpr size_t			GetJumpTableOffset()const		{ return 0; }
		constexpr size_t			GetVariableIdsOffset()	const	{ return GetJumpTableOffset() + GetJumpTableSize(); }
		constexpr size_t			GetVariableDataOffset()	const	{ return GetVariableIdsOffset() + GetVariableIdsSize(); }

		const void*					GetJumpTablePtr()const			{ return m_Data.get(); }
		const VariableId*			GetVariableIdsPtr()	const		{ return reinterpret_cast<VariableId*>(m_Data.get() + GetVariableIdsOffset()); }
		const void*					GetVariableDataPtr()const		{ return m_Data.get() + GetVariableDataOffset(); }

		std::span<VariableId>		GetVariableIds()				{ return { GetVariableIdsPtr(), m_Size }; }
		void*						GetJumpTablePtr()				{ return m_Data.get(); }
		VariableId*					GetVariableIdsPtr()				{ return reinterpret_cast<VariableId*>(m_Data.get() + GetVariableIdsOffset()); }
		void*						GetVariableDataPtr()			{ return m_Data.get() + GetVariableDataOffset(); }

		std::span<const VariableId> GetVariableIds()const			{ return { GetVariableIdsPtr(), m_Size }; }

		constexpr uint32_t			GetSize()const					{ return m_Size; }

	private:
		void		Initialize(std::span<VariableId> variables, bool InitializeToDefault);

		uint32_t	CalculateAlignment(std::span<VariableId> variables) const;
	private:
		std::unique_ptr<uint8_t[]> m_Data{};
		uint32_t m_Size{};
	};

	template <size_t Index, typename... Types>
	inline void InitializeDataTupleCopy(TypeTuple& typeTuple, const std::tuple<Types...>& tuple)
	{
		using Type = std::remove_reference_t<std::tuple_element_t<Index, std::tuple<Types...>>>;

		new (typeTuple.GetVoid(Index)) Type(std::get<Index>(tuple));

		if constexpr (Index + 1 < sizeof...(Types))
		{
			InitializeDataTupleCopy<Index + 1, Types...>(typeTuple, tuple);
		}
	}

	template <size_t Index, typename... Types>
	inline void InitializeDataTupleMove(TypeTuple& typeTuple, std::tuple<Types...>&& tuple)
	{
		using Type = std::remove_reference_t<std::tuple_element_t<Index, std::tuple<Types...>>>;

		new (typeTuple.GetVoid(Index)) Type(std::move(std::get<Index>(tuple)));

		if constexpr (Index + 1 < sizeof...(Types))
		{
			InitializeDataTupleMove<Index + 1, Types...>(typeTuple, std::move(tuple));
		}
	}

	template <size_t Index, typename T, typename ... Types>
	inline	void InitializeDataMove(TypeTuple& typeTuple, T&& val, Types&&... vals)
	{
		using Type = std::remove_reference_t<T>;
		using RawType = trait::strip_type_t<Type>;
		int index = Index;
		auto varId = typeTuple.GetVariable(Index);
		if constexpr (std::is_array_v<std::remove_reference_t<T>>)
		{
			void* start = typeTuple.GetVoid(Index);
			auto size = varId.GetTypeId().GetInfo().Size;
			for (uint32_t i = 0; i < varId.GetArraySize(); ++i)
			{
				new(start)RawType(std::move(val[i]));
				start = static_cast<unsigned char*>(start) + size;
			}
		}
		else
		{
			new (typeTuple.GetVoid(Index)) Type(std::move(val));
		}

		if constexpr (sizeof...(Types) > 0)
		{
			InitializeDataMove<Index + 1, Types...>(typeTuple, std::forward<Types>(vals)...);
		}
	}

	template <size_t Index, typename T, typename ... Types>
	inline	void InitializeDataCopy(TypeTuple& typeTuple, T&& val, Types&&... vals)
	{
		using Type = std::remove_reference_t<T>;
		using RawType = trait::strip_type_t<T>;
		auto varId = typeTuple.GetVariable(Index);
		if constexpr (std::is_array_v<std::remove_reference_t<T>>)
		{
			void* start = typeTuple.GetVoid(Index);
			auto size= varId.GetTypeId().GetInfo().Size;
			for (uint32_t i = 0; i < varId.GetArraySize(); ++i)
			{
				new(start)Type(val[i]);
				start = static_cast<unsigned char*>(start) + size;
			}
		}
		else
		{
			new (typeTuple.GetVoid(Index)) Type(std::forward<T>(val));
		}

		if constexpr (sizeof...(Types) > 0)
		{
			InitializeDataCopy<Index + 1, Types...>(typeTuple, std::forward<Types>(vals)...);
		}
	}

	//计算偏移量，若当前对齐块的剩余空间能存下typeAlign则直接返回当前偏移否则返回下一个对齐块的头部的偏移量
	inline constexpr uint32_t GetOffset(uint32_t structAlign, uint32_t typeAlign, uint32_t currentOffset)
	{
		return ((currentOffset % structAlign) + typeAlign > structAlign) ?
			(currentOffset / structAlign + 1) * structAlign :
			currentOffset;
	}

	static_assert(GetOffset(8, 2, 8) == 8);
	static_assert(GetOffset(8, 2, 10) == 10);
	static_assert(GetOffset(8, 4, 10) == 10);
	static_assert(GetOffset(8, 8, 8) == 8);
	static_assert(GetOffset(8, 8, 9) == 16);
	static_assert(GetOffset(8, 8, 17) == 24);

	inline	TypeTuple::~TypeTuple()
	{
		for (uint32_t i{}; i < m_Size; ++i)
		{
			VariableId id = GetVariable(i);
			if (!id.IsRefOrPointer())
			{
				uint32_t size = id.GetTypeId().GetInfo().Size;
				if (const auto destructor = id.GetTypeId().GetInfo().Destructor)
				{
					void* start = GetVoid(i);
					for (uint32_t j = 0; j < id.GetArraySize(); ++j)
					{
						destructor(start);
						start = reinterpret_cast<unsigned char*>(start) + size;
					}
				}
			}
		}
	}

	inline VariableId TypeTuple::GetVariable(size_t index) const
	{
		return GetVariableIds()[index];
	}

	inline void TypeTuple::SetVariableUnsafe(size_t index, VariableId id)
	{
		GetVariableIds()[index] = id;
	}

	inline void* TypeTuple::GetVoid(size_t index) const
	{
		return reinterpret_cast<void**>(m_Data.get())[index];
	}

	inline void TypeTuple::Initialize(std::span<VariableId> variables, bool InitializeToDefault)
	{
		m_Size = static_cast<uint32_t>(variables.size());
		const size_t jumpTableSize{ sizeof(void*) * variables.size() };
		const size_t variableIdsSize{ sizeof(VariableId) * variables.size() };

		//计算存储类型中的最大对齐值
		const uint32_t structAlignment = CalculateAlignment(variables);

		// 计算用于存储实例数据所需的内存
		uint32_t DataSize{};
		for (auto& var : variables)
		{
			var.RemoveReferenceFlag();
			var.RemoveRValReferenceFlag();

			DataSize = GetOffset(structAlignment, var.GetAlign(), DataSize) + var.GetSize();
		}

		// 分配内存
		const size_t allocatedSize = DataSize + jumpTableSize + variableIdsSize+2;
		m_Data = std::make_unique<uint8_t[]>(allocatedSize);
		std::memset(m_Data.get(), 0, allocatedSize);

		// 将变量ID复制到已分配内存的变量ID部分
		std::ranges::copy(variables, GetVariableIds().begin());

		// 构建跳转表
		uint32_t accumulatedOffset{};
		for (uint32_t i{}; i < variables.size(); ++i)
		{
			auto& var = variables[i];
			accumulatedOffset = GetOffset(structAlignment, var.GetAlign(), accumulatedOffset);

			reinterpret_cast<void**>(m_Data.get())[i] = util::VoidOffset(GetVariableDataPtr(), accumulatedOffset);

			accumulatedOffset += var.GetSize();
		}

		if (InitializeToDefault)
		{
			for (uint32_t i{}; i < variables.size(); ++i)
			{
				auto& var = variables[i];
				if (!var.IsPointer())
				{
					if (const auto constructor = var.GetTypeId().GetInfo().Constructor)
					{
						constructor(GetVoid(i));
					}
				}
			}
		}
	}

	inline uint32_t TypeTuple::CalculateAlignment(std::span<VariableId> variables) const
	{
		uint32_t maxAlign = 1;
		for (auto& variable : variables)
		{
			maxAlign = std::max(variable.GetAlign(), maxAlign);
		}
		return maxAlign;
	}




	

	template<typename ...T>
	inline TypeTuple::TypeTuple(T && ...arg)
	{
		auto variableIds=GetVariableArray<std::remove_reference_t<T>...>();
		Initialize(variableIds, false);
		InitializeDataMove<0, T...>(*this, std::forward<T>(arg)...);
	}

	template<typename ...T>
	inline TypeTuple TypeTuple::Create(const std::tuple<T...>& tuple)
	{
		TypeTuple ret{};
		auto variables = GetVariableArray<T...>();
		Initialize(variables, false);
		InitializeDataTupleCopy<0, T...>(&ret, tuple);
	}

	template<typename ...T>
	inline TypeTuple TypeTuple::Create(std::tuple<T...>&& tuple)
	{
		TypeTuple ret{};
		auto variables = GetVariableArray<T...>();
		Initialize(variables, false);
		InitializeDataTupleMove<0, T...>(&ret, std::move(tuple));
	}

	template<typename ...T>
	inline TypeTuple TypeTuple::CreateNoReferences()
	{
		auto variableIds =GetVariableArray<T...>();
		for (auto& variable : variableIds)
		{
			variable.SetReferenceFlag(false);
		}
		return TypeTuple(variableIds);
	}

	template<typename ...T>
	inline TypeTuple TypeTuple::CreateNoReferences(T && ...val)
	{
		return TypeTuple(std::tuple<std::remove_reference_t<T>...>(std::forward<T>(val)...));
	}

	template<typename T>
	inline T& TypeTuple::Get(size_t index)
	{
		assert(VariableId::Create<T>().GetTypeId() == GetVariableIds()[index].GetTypeId());
		return *static_cast<std::remove_reference_t<T>*>(reinterpret_cast<void**>(m_Data.get())[index]);
	}

	template<typename T>
	inline const T& TypeTuple::Get(size_t index) const
	{
		assert(VariableId::Create<T>().GetTypeId() == GetVariableIds()[index].GetTypeId());
		return *static_cast<std::remove_reference_t<T>*>(reinterpret_cast<void**>(m_Data.get())[index]);
	}

}