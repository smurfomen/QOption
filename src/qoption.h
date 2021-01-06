/*
    MIT License

    Copyright (c) 2020 Agadzhanov Vladimir

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
                                                                                    */

#ifndef QOPTION_H
#define QOPTION_H
#include <functional>
#include <type_traits>
#include <queue>

template <typename Type>
class QOption {
public:
#define NONE None()
    typedef typename std::aligned_storage<sizeof (Type),  alignof(Type)> value_storage;
    QOption(const QOption & o) = delete;
    QOption & operator=(const QOption & o) = delete;

    QOption(QOption && o) noexcept {
        if(o.isSome()) {
            *__ptr_v() = o.unwrap();
            available = true;
        }
        else
            available = false;
    }

    QOption(QOption & o) noexcept {
        if(o.isSome()) {
            *__ptr_v() = o.unwrap();
            available = true;
        }
        else
            available = false;
    }

    QOption() noexcept
        : available(false)
    {

    }

    QOption(const Type & t) noexcept {
        *__ptr_v() = t;
        available = true;
    }

    QOption(Type && t) noexcept {
        std::swap(*__ptr_v(), t);
        available = true;
    }

    bool operator==(const QOption & o) {
        return isSome() == o.isSome() && ((isSome() && *__ptr_v() == *o.__ptr_v()) || isNone());
    }

    QOption & operator=(QOption && o) noexcept {
        if(o.isSome()) {
            *__ptr_v() = o.unwrap();
            available = true;
        }
        else
            available = false;

        return *this;
    }

    QOption & operator=(QOption & o) noexcept {
        if(o.isSome()) {
            o.available = false;
            value = o.value;
            available = true;
        }
        else
            available = false;

        return *this;
    }

    ///@brief Create QOption None value
    static QOption None() {
        return QOption();
    }

    ///@brief Create QOption Some value use copy val into QOption value
    static QOption Some(const Type & val) {
        return QOption(val);
    }

    ///@brief Create QOption Some value use forwarding val into QOption value
    static QOption Some(Type && val) {
        return QOption(std::forward<Type>(val));
    }

    ///@brief Returns true if statement is None
    bool isNone() const { return !available; }

    ///@brief Returns true if statement is Some
    bool isSome() const { return available; }

    ///@brief Return QOptionComposer object for compose handlers, already contained @e fn handler for some case
    template<typename some_callable>
    QOption & if_some(some_callable && fn) {
        if(isSome())
            fn(*__ptr_v());
        return *this;
    }

    ///@brief Return QOptionComposer object for compose handlers, already contained @e fn handler for none case
    template <typename none_callable>
    QOption & if_none(none_callable && fn) {
        if(isNone())
            fn();
        return *this;
    }

    ///@brief Returns value if statement is Some, or throws E type exception if statement is None
    template< typename E = std::logic_error>
    Type unwrap() {
        if(isNone())
            throw E("Option is None value");

        available = false;
        return std::move(*__ptr_v());
    }


    ///@brief Returns value if statement is Some, or E type exception with text message if statement is None
    template <typename E = std::logic_error>
    Type expect (const char * text) {
        if(isNone())
            throw E(text);

        available = false;
        return std::move(*__ptr_v());
    }

    ///@brief Returns value if Some, or returns result of call none_invoke if statement is None
    ///@arg     none_ifn - std::function<value_type()> object - must not provide args and returns @e value_type value
    template<typename none_callable>
    Type unwrap_or(none_callable && none_fn) {
        if(isNone())
            *__ptr_v() = none_fn();

        available = false;
        return std::move(*__ptr_v());
    }

    ///@brief Call some_handler if statement is Some, or call none_invoke if statement is None.
    ///@arg     some_fn - std::functuion<Res(value_type)> object - must be provide one @e value_type type arg and returns @e Res type value
    ///         none_fn - std::function<Res()> object - must not provide args and returns @e Res type value
    ///
    ///@warning be careful if using [&] in functors, scope of lambda drops after exit from scope. Intstead of this use move-semantic or copy current scope.
    template<typename Res, typename some_callable, typename none_callable>
    Res match(some_callable && some_fn, none_callable && none_fn) {
        if(isNone())
            return none_fn();

        available = false;
        return some_fn(std::move(*__ptr_v()));
    }

    ///@brief Returns value if statement is Some, or def_value if statement is None
    Type unwrap_def(const Type & def_value) {
        if(isNone())
            // copy
            *__ptr_v() = def_value;

        available = false;
        return std::move(*__ptr_v());
    }


    ///@brief Returns value if statement is Some, or def_value if statement is None
    Type unwrap_def(Type && def_value) {
        if(isNone())
            // move
            *__ptr_v() = std::forward<Type>(def_value);

        available = false;
        return std::move(*__ptr_v());
    }

private:
    Type * __ptr_v() {
        return reinterpret_cast<Type*>(&value);
    }

    ///@brief validation statement
    bool available {false};

    ///@brief aligned storage with @e keeping data of value
    value_storage value;
};

#endif // QOPTION_H


