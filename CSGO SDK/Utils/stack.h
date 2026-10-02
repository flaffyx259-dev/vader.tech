#pragma once

// get base ptr ( EBP (x86_32) / RBP (x86_64) ).
uintptr_t GetBasePointer() {
	return (uintptr_t)_AddressOfReturnAddress() - sizeof(uintptr_t);
}

class Stack : public Address {
public:
	// ctor.
	__forceinline Stack( ) : Address{ GetBasePointer( ) } {}

	// get return address.
	__forceinline Address ReturnAddress( ) {
		if( !m_addr )
			return Address{};

		return *( uintptr_t* )( m_addr + sizeof( uintptr_t ) );
	}

	// get address of return address.
	__forceinline Address AddressOfReturnAddress( ) {
		if( !m_addr )
			return Address{};

		return ( m_addr + sizeof( uintptr_t ) );
	}

	// go up stack frame.
	__forceinline Stack& next( ) {
		return *( Stack* )m_addr;
	}

	// get local variable.
	template< typename t = Address >
	__forceinline t local( size_t offset ) {
		return ( t )( m_addr - offset );
	}

	// get arg.
	template< typename t = Address >
	__forceinline t arg( size_t offset ) {
		return ( t )( m_addr + offset );
	}
};