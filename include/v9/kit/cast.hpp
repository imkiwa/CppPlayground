#pragma once

#include <cassert>
#include <memory>
#include <type_traits>

namespace llvm {
    template <typename T>
    class is_integral_or_enum {
        using UnderlyingT = typename std::remove_reference<T>::type;

    public:
        static constexpr bool value =
            !std::is_class_v<UnderlyingT> && // Filter conversion operators.
            !std::is_pointer_v<UnderlyingT> &&
            !std::is_floating_point_v<UnderlyingT> &&
            (std::is_enum_v<UnderlyingT> ||
             std::is_convertible_v<UnderlyingT, unsigned long long>);
    };

    /// If T is a pointer, just return it. If it is not, return T&.
    template <typename T, bool = std::is_pointer_v<T>>
    struct add_lvalue_reference_if_not_pointer {
        using type = T &;
    };

    template <typename T>
    struct add_lvalue_reference_if_not_pointer<T, true> {
        using type = T;
    };

    /// If T is a pointer to X, return a pointer to const X. If it is not,
    /// return const T.
    template <typename T, bool = std::is_pointer_v<T>>
    struct add_const_past_pointer {
        using type = const T;
    };

    template <typename T>
    struct add_const_past_pointer<T, true> {
        using type = const std::remove_pointer_t<T> *;
    };

    //===----------------------------------------------------------------------===//
    //                          isa<x> Support Templates
    //===----------------------------------------------------------------------===//

    // Define a template that can be specialized by smart pointers to reflect the
    // fact that they are automatically dereferenced, and are not involved with the
    // template selection process...  the default implementation is a noop.
    //
    template <typename From>
    struct SimplifyType {
        using SimpleType = From;

        static SimpleType &getSimplifiedValue(From &Val) { return Val; }
    };

    template <typename From>
    struct SimplifyType<const From> {
        using NonConstSimpleType = typename SimplifyType<From>::SimpleType;
        using SimpleType =
        typename add_const_past_pointer<NonConstSimpleType>::type;
        using ReturnType =
        typename add_lvalue_reference_if_not_pointer<SimpleType>::type;

        static ReturnType getSimplifiedValue(const From &Val) {
            return SimplifyType<From>::getSimplifiedValue(const_cast<From &>(Val));
        }
    };

    // The core of the implementation of isa<X> is here; To and From should be
    // the names of classes.  This template can be specialized to customize the
    // implementation of isa<> without rewriting it from scratch.
    template <typename To, typename From, typename Enabler = void>
    struct isa_impl {
        static inline bool doit(const From &Val) {
            return To::classof(&Val);
        }
    };

    /// Always allow upcasts, and perform no dynamic check for them.
    template <typename To, typename From>
    struct isa_impl<
        To, From, std::enable_if_t<std::is_base_of_v<To, From>>> {
        static inline bool doit(const From &) { return true; }
    };

    template <typename To, typename From>
    struct isa_impl_cl {
        static inline bool doit(const From &Val) {
            return isa_impl<To, From>::doit(Val);
        }
    };

    template <typename To, typename From>
    struct isa_impl_cl<To, const From> {
        static inline bool doit(const From &Val) {
            return isa_impl<To, From>::doit(Val);
        }
    };

    template <typename To, typename From>
    struct isa_impl_cl<To, const std::unique_ptr<From>> {
        static inline bool doit(const std::unique_ptr<From> &Val) {
            assert(Val && "isa<> used on a null pointer");
            return isa_impl_cl<To, From>::doit(*Val);
        }
    };

    template <typename To, typename From>
    struct isa_impl_cl<To, From *> {
        static inline bool doit(const From *Val) {
            assert(Val && "isa<> used on a null pointer");
            return isa_impl<To, From>::doit(*Val);
        }
    };

    template <typename To, typename From>
    struct isa_impl_cl<To, From *const> {
        static inline bool doit(const From *Val) {
            assert(Val && "isa<> used on a null pointer");
            return isa_impl<To, From>::doit(*Val);
        }
    };

    template <typename To, typename From>
    struct isa_impl_cl<To, const From *> {
        static inline bool doit(const From *Val) {
            assert(Val && "isa<> used on a null pointer");
            return isa_impl<To, From>::doit(*Val);
        }
    };

    template <typename To, typename From>
    struct isa_impl_cl<To, const From *const> {
        static inline bool doit(const From *Val) {
            assert(Val && "isa<> used on a null pointer");
            return isa_impl<To, From>::doit(*Val);
        }
    };

    template <typename To, typename From, typename SimpleFrom>
    struct isa_impl_wrap {
        // When From != SimplifiedType, we can simplify the type some more by using
        // the SimplifyType template.
        static bool doit(const From &Val) {
            return isa_impl_wrap<To, SimpleFrom,
                typename SimplifyType<SimpleFrom>::SimpleType>::doit(
                SimplifyType<const From>::getSimplifiedValue(Val));
        }
    };

    template <typename To, typename FromTy>
    struct isa_impl_wrap<To, FromTy, FromTy> {
        // When From == SimpleType, we are as simple as we are going to get.
        static bool doit(const FromTy &Val) {
            return isa_impl_cl<To, FromTy>::doit(Val);
        }
    };

// isa<X> - Return true if the parameter to the template is an instance of the
// template type argument.  Used like this:
//
//  if (isa<Type>(myVal)) { ... }
//
    template <class X, class Y>
    inline bool isa(const Y &Val) {
        return isa_impl_wrap<X, const Y,
            typename SimplifyType<const Y>::SimpleType>::doit(Val);
    }

// isa_and_nonnull<X> - Functionally identical to isa, except that a null value
// is accepted.
//
    template <class X, class Y>
    inline bool isa_and_nonnull(const Y &Val) {
        if (!Val)
            return false;
        return isa<X>(Val);
    }

//===----------------------------------------------------------------------===//
//                          cast<x> Support Templates
//===----------------------------------------------------------------------===//

    template <class To, class From>
    struct cast_retty;

// Calculate what type the 'cast' function should return, based on a requested
// type of To and a source type of From.
    template <class To, class From>
    struct cast_retty_impl {
        using ret_type = To &;       // Normal case, return Ty&
    };
    template <class To, class From>
    struct cast_retty_impl<To, const From> {
        using ret_type = const To &; // Normal case, return Ty&
    };

    template <class To, class From>
    struct cast_retty_impl<To, From *> {
        using ret_type = To *;       // Pointer arg case, return Ty*
    };

    template <class To, class From>
    struct cast_retty_impl<To, const From *> {
        using ret_type = const To *; // Constant pointer arg case, return const Ty*
    };

    template <class To, class From>
    struct cast_retty_impl<To, const From *const> {
        using ret_type = const To *; // Constant pointer arg case, return const Ty*
    };

    template <class To, class From>
    struct cast_retty_impl<To, std::unique_ptr<From>> {
    private:
        using PointerType = typename cast_retty_impl<To, From *>::ret_type;
        using ResultType = typename std::remove_pointer<PointerType>::type;

    public:
        using ret_type = std::unique_ptr<ResultType>;
    };

    template <class To, class From, class SimpleFrom>
    struct cast_retty_wrap {
        // When the simplified type and the from type are not the same, use the type
        // simplifier to reduce the type, then reuse cast_retty_impl to get the
        // resultant type.
        using ret_type = typename cast_retty<To, SimpleFrom>::ret_type;
    };

    template <class To, class FromTy>
    struct cast_retty_wrap<To, FromTy, FromTy> {
        // When the simplified type is equal to the from type, use it directly.
        using ret_type = typename cast_retty_impl<To, FromTy>::ret_type;
    };

    template <class To, class From>
    struct cast_retty {
        using ret_type = typename cast_retty_wrap<
            To, From, typename SimplifyType<From>::SimpleType>::ret_type;
    };

// Ensure the non-simple values are converted using the SimplifyType template
// that may be specialized by smart pointers...
//
    template <class To, class From, class SimpleFrom>
    struct cast_convert_val {
        // This is not a simple type, use the template to simplify it...
        static typename cast_retty<To, From>::ret_type doit(From &Val) {
            return cast_convert_val<To, SimpleFrom,
                typename SimplifyType<SimpleFrom>::SimpleType>::doit(
                SimplifyType<From>::getSimplifiedValue(Val));
        }
    };

    template <class To, class FromTy>
    struct cast_convert_val<To, FromTy, FromTy> {
        // This _is_ a simple type, just cast it.
        static typename cast_retty<To, FromTy>::ret_type doit(const FromTy &Val) {
            typename cast_retty<To, FromTy>::ret_type Res2
                = (typename cast_retty<To, FromTy>::ret_type) const_cast<FromTy &>(Val);
            return Res2;
        }
    };

    template <class X>
    struct is_simple_type {
        static const bool value =
            std::is_same<X, typename SimplifyType<X>::SimpleType>::value;
    };

// cast<X> - Return the argument parameter cast to the specified type.  This
// casting operator asserts that the type is correct, so it does not return null
// on failure.  It does not allow a null argument (use cast_or_null for that).
// It is typically used like this:
//
//  cast<Instruction>(myVal)->getParent()
//
    template <class X, class Y>
    inline typename std::enable_if<!is_simple_type<Y>::value,
        typename cast_retty<X, const Y>::ret_type>::type
    cast(const Y &Val) {
        assert(isa<X>(Val) && "cast<Ty>() argument of incompatible type!");
        return cast_convert_val<
            X, const Y, typename SimplifyType<const Y>::SimpleType>::doit(Val);
    }

    template <class X, class Y>
    inline typename cast_retty<X, Y>::ret_type cast(Y &Val) {
        assert(isa<X>(Val) && "cast<Ty>() argument of incompatible type!");
        return cast_convert_val<X, Y,
            typename SimplifyType<Y>::SimpleType>::doit(Val);
    }

    template <class X, class Y>
    inline typename cast_retty<X, Y *>::ret_type cast(Y *Val) {
        assert(isa<X>(Val) && "cast<Ty>() argument of incompatible type!");
        return cast_convert_val<X, Y *,
            typename SimplifyType<Y *>::SimpleType>::doit(Val);
    }

    template <class X, class Y>
    inline typename cast_retty<X, std::unique_ptr<Y>>::ret_type
    cast(std::unique_ptr<Y> &&Val) {
        assert(isa<X>(Val.get()) && "cast<Ty>() argument of incompatible type!");
        using ret_type = typename cast_retty<X, std::unique_ptr<Y>>::ret_type;
        return ret_type(
            cast_convert_val<X, Y *, typename SimplifyType<Y *>::SimpleType>::doit(
                Val.release()));
    }

// cast_or_null<X> - Functionally identical to cast, except that a null value is
// accepted.
//
    template <class X, class Y>
    inline
    typename std::enable_if<!is_simple_type<Y>::value,
        typename cast_retty<X, const Y>::ret_type>::type
    cast_or_null(const Y &Val) {
        if (!Val)
            return nullptr;
        assert(isa<X>(Val) && "cast_or_null<Ty>() argument of incompatible type!");
        return cast<X>(Val);
    }

    template <class X, class Y>
    inline
    typename std::enable_if<!is_simple_type<Y>::value,
        typename cast_retty<X, Y>::ret_type>::type
    cast_or_null(Y &Val) {
        if (!Val)
            return nullptr;
        assert(isa<X>(Val) && "cast_or_null<Ty>() argument of incompatible type!");
        return cast<X>(Val);
    }

    template <class X, class Y>
    inline typename cast_retty<X, Y *>::ret_type
    cast_or_null(Y *Val) {
        if (!Val) return nullptr;
        assert(isa<X>(Val) && "cast_or_null<Ty>() argument of incompatible type!");
        return cast<X>(Val);
    }

    template <class X, class Y>
    inline typename cast_retty<X, std::unique_ptr<Y>>::ret_type
    cast_or_null(std::unique_ptr<Y> &&Val) {
        if (!Val)
            return nullptr;
        return cast<X>(std::move(Val));
    }

// dyn_cast<X> - Return the argument parameter cast to the specified type.  This
// casting operator returns null if the argument is of the wrong type, so it can
// be used to test for a type as well as cast if successful.  This should be
// used in the context of an if statement like this:
//
//  if (const Instruction *I = dyn_cast<Instruction>(myVal)) { ... }
//

    template <class X, class Y>
    inline
    typename std::enable_if<!is_simple_type<Y>::value,
        typename cast_retty<X, const Y>::ret_type>::type
    dyn_cast(const Y &Val) {
        return isa<X>(Val) ? cast<X>(Val) : nullptr;
    }

    template <class X, class Y>
    inline typename cast_retty<X, Y>::ret_type dyn_cast(Y &Val) {
        return isa<X>(Val) ? cast<X>(Val) : nullptr;
    }

    template <class X, class Y>
    inline typename cast_retty<X, Y *>::ret_type dyn_cast(Y *Val) {
        return isa<X>(Val) ? cast<X>(Val) : nullptr;
    }

// dyn_cast_or_null<X> - Functionally identical to dyn_cast, except that a null
// value is accepted.
//
    template <class X, class Y>
    inline
    typename std::enable_if<!is_simple_type<Y>::value,
        typename cast_retty<X, const Y>::ret_type>::type
    dyn_cast_or_null(const Y &Val) {
        return (Val && isa<X>(Val)) ? cast<X>(Val) : nullptr;
    }

    template <class X, class Y>
    inline
    typename std::enable_if<!is_simple_type<Y>::value,
        typename cast_retty<X, Y>::ret_type>::type
    dyn_cast_or_null(Y &Val) {
        return (Val && isa<X>(Val)) ? cast<X>(Val) : nullptr;
    }

    template <class X, class Y>
    inline typename cast_retty<X, Y *>::ret_type
    dyn_cast_or_null(Y *Val) {
        return (Val && isa<X>(Val)) ? cast<X>(Val) : nullptr;
    }

// unique_dyn_cast<X> - Given a unique_ptr<Y>, try to return a unique_ptr<X>,
// taking ownership of the input pointer iff isa<X>(Val) is true.  If the
// cast is successful, From refers to nullptr on exit and the casted value
// is returned.  If the cast is unsuccessful, the function returns nullptr
// and From is unchanged.
    template <class X, class Y>
    inline auto unique_dyn_cast(std::unique_ptr<Y> &Val)
    -> decltype(cast<X>(Val)) {
        if (!isa<X>(Val))
            return nullptr;
        return cast<X>(std::move(Val));
    }

    template <class X, class Y>
    inline auto unique_dyn_cast(std::unique_ptr<Y> &&Val)
    -> decltype(cast<X>(Val)) {
        return unique_dyn_cast<X, Y>(Val);
    }

// dyn_cast_or_null<X> - Functionally identical to unique_dyn_cast, except that
// a null value is accepted.
    template <class X, class Y>
    inline auto unique_dyn_cast_or_null(std::unique_ptr<Y> &Val)
    -> decltype(cast<X>(Val)) {
        if (!Val)
            return nullptr;
        return unique_dyn_cast<X, Y>(Val);
    }

    template <class X, class Y>
    inline auto unique_dyn_cast_or_null(std::unique_ptr<Y> &&Val)
    -> decltype(cast<X>(Val)) {
        return unique_dyn_cast_or_null<X, Y>(Val);
    }

} // end namespace llvm

