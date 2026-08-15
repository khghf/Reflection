#pragma once
#include"TypeInfo.h"
#include"Property.h"
namespace mirror
{
	/// <summary>
	/// 用于描述变量的类，内部持有描述变量类型的 TypeId，并提供多种方法来操作和查询变量的属性。
	/// </summary>
	class VariableId final
	{
	
	public:
		constexpr explicit VariableId(TypeId id) : m_Type{ id } {}
		constexpr VariableId() = default;

		template <typename T>
		static constexpr VariableId Create();

	public:

		constexpr TypeId	GetTypeId()const					{ return m_Type; }
		constexpr uint32_t	GetPointerAmount()const				{ return m_PointerAmount; }
		constexpr uint32_t	GetArraySize()const					{ return m_ArraySize; }
		constexpr uint64_t	GetHash()const						{ return m_Type.GetId() ^ m_ArraySize ^ (static_cast<uint64_t>(m_PointerAmount) << 32) ^ (static_cast<uint64_t>(m_TraitFlags) << 40); }
		constexpr uint32_t	GetSize()const						{ return IsRefOrPointer() ? static_cast<uint32_t>(sizeof(void*)) : GetTypeId().GetInfo().Size * GetArraySize(); }
		constexpr uint32_t	GetAlign()const 					{ return IsRefOrPointer() ? static_cast<uint32_t>(alignof(void*)) : GetTypeId().GetInfo().Align; }

		constexpr void		SetTypeId(TypeId id)				{ m_Type = id; }
		constexpr void		SetConstFlag()						{ m_TraitFlags |= ConstFlag; }
		constexpr void		SetReferenceFlag()					{ m_TraitFlags |= ReferenceFlag; }
		constexpr void		SetVolatileFlag()					{ m_TraitFlags |= VolatileFlag; }
		constexpr void		SetRValReferenceFlag()				{ m_TraitFlags |= RValReferenceFlag; }
		constexpr void		SetPointerAmount(uint16_t amount)	{ m_PointerAmount = amount; }
		constexpr void		SetArraySize(uint32_t Size)			{ m_ArraySize = Size; }

		constexpr void		RemoveConstFlag()					{ m_TraitFlags &= ~ConstFlag; }
		constexpr void		RemoveReferenceFlag()				{ m_TraitFlags &= ~ReferenceFlag; }
		constexpr void		RemoveVolatileFlag()				{ m_TraitFlags &= ~VolatileFlag; }
		constexpr void		RemoveRValReferenceFlag()			{ m_TraitFlags &= ~RValReferenceFlag; }

		

		constexpr bool		IsConst()const						{ return m_TraitFlags & ConstFlag; }
		constexpr bool		IsReference()const					{ return m_TraitFlags & ReferenceFlag; }
		constexpr bool		IsVolatile()const					{ return m_TraitFlags & VolatileFlag; }
		constexpr bool		IsRValReference()const				{ return m_TraitFlags & RValReferenceFlag; }
		constexpr bool		IsPointer()	const					{ return m_PointerAmount; }
		constexpr bool		IsArray()const						{ return m_ArraySize > 1; }
		constexpr bool		IsRefOrPointer()const				{ return IsPointer() || IsReference() || IsRValReference(); }

		

		friend constexpr bool operator==(const VariableId& lhs, const VariableId& rhs);
		friend std::ostream& operator<<(std::ostream& lhs, const VariableId& rhs);
		friend std::istream& operator>>(std::istream& lhs,VariableId& rhs);

		std::string			ToString()const
		{
			std::string name = std::string(m_Type.GetInfo().Name);

			if (IsVolatile()) name = "volatile " + name;
			if (IsConst()) name = "const " + name;

			const uint32_t pointerAmount = GetPointerAmount();
			for (uint32_t i{}; i < pointerAmount; ++i)
			{
				name += '*';
			}

			if (IsRValReference()) name += "&&";
			else if (IsReference()) name += '&';

			return name;
		}
	private:
		static constexpr uint32_t ConstFlag = 1 << 0; //const int
		static constexpr uint32_t ReferenceFlag = 1 << 1; //const int&
		static constexpr uint32_t VolatileFlag = 1 << 2; //volatile int
		static constexpr uint32_t RValReferenceFlag = 1 << 3; //int&&
	private:
		TypeId		m_Type{ };	
		uint32_t	m_ArraySize{ };	
		uint16_t	m_PointerAmount{ };	
		uint32_t		m_TraitFlags{ };
	};

	class TypeTuple;

	/// <summary>
	/// 描述一个数据成员的信息，包括名称、类型、偏移量、大小、对齐方式以及属性。
	/// </summary>
	struct MemberInfo final
	{
		friend class TypeId;

		std::string			Name{ };
		VariableId			Variable{ }; 
		uint32_t			Offset{ };
		uint32_t			Size{ }; 
		uint32_t			Align{ };

		MemberProperties	Properties{ };

		constexpr bool operator<(const MemberInfo& rhs) const { return Offset < rhs.Offset; }

		constexpr bool IsPropertySet(MemberProperties property) const { return !!(Properties & property); }

		template<typename T>
		void Set(void* instance, T&& data)const;
		template<typename T>
		T Get(void* instance)const;
		template<typename T>
		T& GetRef(void* instance)const;
		
		void	(*Setter)(void* instance, TypeTuple&typeTuple, uint32_t offset);
		void*	(*Getter)(void* instance, uint32_t offset);
	};

	

}