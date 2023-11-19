#pragma once
#include <functional>


class CFunction {
public:
    template<typename Function, typename...Args>
    CFunction(Function func, Args...args) {
        m_binder = std::bind(func, args...);
    }

    ~CFunction() {}

    int operator()() {
        return m_binder();
    }
private:
    std::function<int()> m_binder;
};
