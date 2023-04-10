#pragma once

#include "Header.h"

#if defined _USE_MODULES

import <memory>;
import <exception>;
import <functional>;
import :Concepts;

#else

#include <memory>
#include <exception>
#include <functional>
#include "Concepts.h"

#endif

_EXPORT_UTILS namespace Utils
{
    struct TypeNotRegisteredError : public std::exception
    {
        const char* what() const noexcept override { return "Type not registered"; }
    };

    template <class ... RegisteredTypes>
        class ClassFactory
    {
    public:
        ClassFactory() : buildersPtrs{ std::make_tuple< BuilderPtr<RegisteredTypes>... >(std::make_shared< NotRegisteredBuilder<RegisteredTypes> >()...) } {}
        template <class Interface, class Implementation = Interface>
        void RegisterType()
        {
            std::get<BuilderPtr<Interface>>(buildersPtrs) = std::make_shared<TypeResolver<Interface, Implementation>>(*this);
        }

        template <class _Type>
        void Register(std::function<_Type()> function)
        {
            std::get<BuilderPtr<_Type>>(buildersPtrs) = std::make_shared<DelegateBuilder<_Type>>(function, *this);
        }

        template <class _Type>
        void RegisterInstance(_Type instance)
        {
            std::get<BuilderPtr<_Type>>(buildersPtrs) = std::make_shared<InstanceCopier<_Type>>(instance, *this);
        }

        template <class ... Args>
        class Registrator
        {
        public:
            template <class Interface, class Implementation>
            void RegisterType()
            {
                std::get<BuilderPtr<Interface>>(parent.buildersPtrs) = std::make_shared<TypeResolver<Interface, Implementation, Args...>>(parent);
            }
            friend class ClassFactory;
        private:
            Registrator(ClassFactory<RegisteredTypes ...>& parent) : parent{ parent } {}
            ClassFactory<RegisteredTypes ...>& parent;
        };

        template <class ... Args>
        Registrator<Args...> WithParams()
        {
            return { *this };
        }

    private:

        template <class Type>
        class IBuilder
        {
        public:
            virtual Type Build() = 0;
            virtual ~IBuilder() {}
        };

        template <class _Interface, class _Implementation, class ... Args >
        class TypeResolver : public IBuilder<_Interface>
        {
        public:
            TypeResolver(ClassFactory<RegisteredTypes... >& parent) : parent{ parent } {}
            virtual _Interface Build() override
            {
                if constexpr (is_shared_pointer_v<_Implementation>) {
                    return std::make_shared<typename _Implementation::element_type>(parent.Resolve<Args>()...);
                }
                else if constexpr (is_unique_pointer_v<_Implementation>) {
                    return std::make_unique<typename _Implementation::element_type>(parent.Resolve<Args>()...);
                }
                else
                    return _Implementation{ parent.Resolve<Args>()... };
            }
        private:
            ClassFactory<RegisteredTypes... >& parent;
        };

        template <class _Type>
        class InstanceCopier : public IBuilder<_Type>
        {
        public:
            InstanceCopier(_Type instance, ClassFactory<RegisteredTypes... >& parent) : instance{ std::move(instance) }, parent{ parent } {}
            virtual _Type Build() override
            {
                return instance;
            }
        private:
            _Type instance;
            ClassFactory<RegisteredTypes... >& parent;
        };

        template <class _Type>
        class DelegateBuilder : public IBuilder<_Type>
        {
        public:
            DelegateBuilder(std::function<_Type()> function, ClassFactory<RegisteredTypes... >& parent) : function{ std::move(function) }, parent{ parent } {}
            virtual _Type Build() override
            {
                return std::invoke(function);
            }
        private:
            std::function<_Type()> function;
            ClassFactory<RegisteredTypes... >& parent;
        };

        template <class _Type>
        class NotRegisteredBuilder : public IBuilder<_Type>
        {
        public:
            NotRegisteredBuilder() {}
            virtual _Type Build() override
            {
                throw TypeNotRegisteredError{};
            }
        };

        template <class Type>
        using BuilderPtr = std::shared_ptr<IBuilder<Type>>;

    public:

        template<class Interface>
        Interface Resolve()
        {
            static_assert(any_of< RegisteredTypes ... >::is_same_as<Interface>::value);
            BuilderPtr<Interface>& builder = std::get<BuilderPtr<Interface>>(buildersPtrs);
            return builder->Build();
        }

        //template <class ... ChildTypes>
        //ClassFactory<ChildTypes ...> GetChild()
        //{
        //    //struct AllOfChildTypesAreInRegisteredTypes : all_of< ChildTypes ... >::are_in< RegisteredTypes ... > {};
        //    //static_assert(all_of< ChildTypes ... >::are_in< RegisteredTypes ... >::value);
        //}

    private:
        std::tuple<BuilderPtr<RegisteredTypes>...> buildersPtrs;
    };
}