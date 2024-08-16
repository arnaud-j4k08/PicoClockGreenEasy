#pragma once

// Make a global function that calls the method of the given Class so that it is suitable for 
// passing to a C function that expects a function pointer as callback. The type parameter 
// determines where the user pointer is placed in the parameters of the global function. This user
// pointer must point to the object.
#define MAKE_TRAMPOLINE(Class, method, type) \
    auto method = &Trampoline<decltype(&Class::method)>::type<&Class::method>;

template <typename MethodPtr>
class Trampoline;

template <typename Return, class Class, typename... Param>
struct Trampoline <Return (Class::*)(Param... param)>
{
    template <Return (Class::*methodPtr)(Param...)>
    static Return userPtrAtBegin(void *user, Param... params)
    {
        auto self = static_cast<Class *>(user);
        return (self->*methodPtr)(params...);
    }

    template <Return (Class::*methodPtr)(Param...)>
    static Return userPtrAtEnd(Param... params, void *user)
    {
        auto self = static_cast<Class *>(user);
        return (self->*methodPtr)(params...);
    }

    // For use with add_repeating_timer_ms
    template <Return (Class::*methodPtr)()>
    static Return repeating_timer_t(repeating_timer_t *rt)
    {
        auto self = static_cast<Class *>(rt->user_data);
        return (self->*methodPtr)();
    }    
};
