#pragma once

#include "Header.h"

#if defined _USE_MODULES

import <memory>;
import <type_traits>;
import <exception>;
import <functional>;
import :Concepts;

#else

#include <memory>
#include <type_traits>
#include <exception>
#include <functional>
#include "Concepts.h"

#endif

_EXPORT_UTILS namespace Utils
{
	struct NullPointerError : public std::exception
	{
		const char* what() const noexcept override { return "Null pointer error"; }
	};

	// TODO: instead of specifing shared or unique it could be dependent on existance of copy assignment e.g. if (std::is_copy_assignable_v<Pointer>) then it is shared pointer like
#if _HAS_CXX20
	template <Concepts::OwningPointer Pointer>
#else
	template <class Pointer, class = std::enable_if_t<is_owning_pointer<Pointer>::value>>
#endif
	struct NotNull
	{
	private:
		_INLINE_VAR constexpr static bool is_shared_like = std::is_copy_assignable<Pointer>::value;
		_INLINE_VAR constexpr static bool is_unique_like = std::is_move_assignable<Pointer>::value;
	public:
		using element_type = typename Pointer::element_type;
		Pointer pointer;
		NotNull(Pointer pointer) : pointer{ std::move(pointer) } { check(); }
		NotNull(NotNull&& other) : pointer{ std::move(other.pointer) } { check(); }
		NotNull& operator=(NotNull&& other)
		{
			pointer = std::move(other.pointer);
			return *this;
		}
		NotNull(const NotNull& other) : pointer{ other.pointer } {}
		NotNull& operator=(const NotNull& other)
		{
			static_assert(is_shared_like, "Not null pointer is not shared like");
			pointer = other.pointer;
			check();
			return *this;
		}

		NotNull& operator=(std::unique_ptr<element_type>&& other)
		{
			check();
			pointer = std::move(other);
			return *this;
		}

		NotNull() = delete;
		~NotNull() {}
		element_type& operator*() { check(); return *pointer; }
		element_type* operator->() { check(); return pointer.operator->(); }
		operator bool() const { return pointer.operator bool(); }
		bool operator==(const NotNull& other) const { return pointer == other.pointer; }
		bool operator!=(const NotNull& other) const { return pointer != other.pointer; }
		bool operator<(const NotNull& other) const { return pointer < other.pointer; }
		bool operator>(const NotNull& other) const { return pointer > other.pointer; }
		bool operator<=(const NotNull& other) const { return pointer <= other.pointer; }
		bool operator>=(const NotNull& other) const { return pointer >= other.pointer; }

		operator const std::unique_ptr<element_type>& () const&
		{
			static_assert(is_unique_like, "Not null pointer is not unique like");
			check(); return pointer;
		}
		operator const std::shared_ptr<element_type>& () const&
		{
			static_asset(is_shared_like, "Not null pointer is not shared like");
			check(); return pointer;
		}
		operator const std::weak_ptr<element_type>& () const&
		{
			static_asset(is_shared_like, "Not null pointer is not shared like");
			check(); return pointer;
		}

		operator std::unique_ptr<element_type> && () &&
		{
			static_assert(is_unique_like, "Not null pointer is not unique like");
			check(); return std::move(pointer);
		}
		operator std::shared_ptr<element_type> && () &&
		{
			static_assert(is_shared_like || is_unique_like, "Not null pointer is not unique nor shared like");
			check(); return std::move(pointer);
		}
		operator std::weak_ptr<element_type> && () &&
		{
			static_assert(is_shared_like, "Not null pointer is not shared like");
			check(); return std::move(pointer);
		}

		element_type* get() { check(); return pointer.get(); }
		const element_type* get() const { check(); return pointer.get(); }
	private:
		void check() const { if (!pointer) throw NullPointerError{}; }
	};

	template <class T> using SharedNotNull = NotNull<std::shared_ptr<T>>;
	template <class T> using UniqueNotNull = NotNull<std::unique_ptr<T>>;

}
