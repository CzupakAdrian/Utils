#pragma once

#include "Header.h"

#if defined _USE_MODULES

import <functional>;

#else

#include <functional>

#endif

_EXPORT_UTILS namespace Utils
{
    namespace detail
    {
        template <class _Class, class _Out, class ... Args>
        struct FunctionAsExtension
        {
            using type = std::function<_Out(_Class&)>;
            using functor = _Out(*)(_Class&, Args ...);
            type method;
            FunctionAsExtension(functor function, Args ... args)
                : method{ [=](_Class& self) -> _Out { return function(self, args ...); } } {}
            _Out operator()(_Class& self) { return method(self); }
        };
    }

    template <class _Class, class _Out, class ... Args, class ... Argg>
    std::function<_Out(_Class&)> applyMethod(
        _Out(*function)(_Class&, Args ...),
        Argg ... args)
    {
        return detail::FunctionAsExtension<_Class, _Out, Args ...>{function, args ...};
    }
}

_EXPORT_UTILS template <class _Class, class _Out>
_Out operator | (_Class& self, std::function<_Out(_Class&)> method)
{
    return method(self);
}