#pragma once

#include <iostream>

#ifdef TRACE_TO_STDIO

#define TRACE Trace(__FILE__, __LINE__)

class SetAutoSpace
{
public:
    SetAutoSpace(bool autoSpace) : m_autoSpace(autoSpace)
    {
    }

    bool autoSpace() const
    {
        return m_autoSpace;
    }

private:
    bool m_autoSpace;
};

class Trace
{
public:
    Trace(const char *file, int line);
    ~Trace();
    Trace &operator<<(const SetAutoSpace &sas);

    template <typename T>
    Trace &operator <<(const T &value)
    {
        if (m_enabled)
        {
            std::cout << value;

            if (m_autoSpace) std::cout <<" ";
        }
        return *this;
    }

    Trace &operator<<(const tm &dateTime);

private:
    bool m_enabled;
    bool m_autoSpace = true;
};

#else // TRACE_TO_STDIO

#define TRACE Trace()

class SetAutoSpace
{
public:
    SetAutoSpace(bool /*autoSpace*/) {}
};

class Trace
{
public:
    template <typename T>
    Trace &operator <<(const T &value)
    {
        return *this;
    }
};

#endif // TRACE_TO_STDIO