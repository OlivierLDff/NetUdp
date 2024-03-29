// Copyright 2019 - 2021 Olivier Le Doeuff
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright noticeand this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef __NETUDP_PROPERTY_HPP__
#define __NETUDP_PROPERTY_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

#include <QtQml/QQmlEngine>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

#define _NETUDP_Q_PROPERTY(type, attribute, Attribute)                                                                                     \
protected:                                                                                                                                 \
    Q_PROPERTY(type attribute READ attribute WRITE set##Attribute RESET reset##Attribute NOTIFY attribute##Changed)

#define _NETUDP_Q_PROPERTY_RO(type, attribute, Attribute)                                                                                  \
protected:                                                                                                                                 \
    Q_PROPERTY(type attribute READ attribute NOTIFY attribute##Changed)

#define _NETUDP_Q_PROPERTY_CONST(type, attribute)                                                                                          \
protected:                                                                                                                                 \
    Q_PROPERTY(type attribute READ attribute CONSTANT)

#define _NETUDP_PROPERTY_MEMBER(type, attribute, def)                                                                                      \
private:                                                                                                                                   \
    type _##attribute = def;

#define _NETUDP_PROPERTY_GETTER(type, attribute, override)                                                                                 \
public:                                                                                                                                    \
    type attribute() const override                                                                                                        \
    {                                                                                                                                      \
        return _##attribute;                                                                                                               \
    }

#define _NETUDP_PROPERTY_GETTER_ABSTRACT(type, attribute)                                                                                  \
public:                                                                                                                                    \
    virtual type attribute() const = 0;

#define _NETUDP_PROPERTY_SETTER(type, attribute, Attribute, override)                                                                      \
public:                                                                                                                                    \
    virtual bool set##Attribute(const type& value) override                                                                                \
    {                                                                                                                                      \
        if(value != _##attribute)                                                                                                          \
        {                                                                                                                                  \
            _##attribute = value;                                                                                                          \
            Q_EMIT attribute##Changed(value);                                                                                              \
            return true;                                                                                                                   \
        }                                                                                                                                  \
        return false;                                                                                                                      \
    }

#define _NETUDP_ATTRIBUTE_SETTER(type, attribute, Attribute, override)                                                                     \
public:                                                                                                                                    \
    virtual bool set##Attribute(const type& value) override                                                                                \
    {                                                                                                                                      \
        if(value != _##attribute)                                                                                                          \
        {                                                                                                                                  \
            _##attribute = value;                                                                                                          \
            return true;                                                                                                                   \
        }                                                                                                                                  \
        return false;                                                                                                                      \
    }

#define _NETUDP_PROPERTY_SETTER_ABSTRACT(type, Attribute)                                                                                  \
public:                                                                                                                                    \
    virtual bool set##Attribute(const type& value) = 0;

#define _NETUDP_PROPERTY_RESET(type, Attribute, def)                                                                                       \
public:                                                                                                                                    \
    void reset##Attribute()                                                                                                                \
    {                                                                                                                                      \
        set##Attribute(def);                                                                                                               \
    }

#define _NETUDP_PROPERTY_SIGNAL(type, attribute)                                                                                           \
Q_SIGNALS:                                                                                                                                 \
    void attribute##Changed(type attribute);

#define _NETUDP_ABSTRACT_PROPERTY_SHARED(type, attribute, Attribute, def)                                                                  \
    _NETUDP_PROPERTY_GETTER_ABSTRACT(type, attribute)                                                                                      \
    _NETUDP_PROPERTY_SETTER_ABSTRACT(type, Attribute)                                                                                      \
    _NETUDP_PROPERTY_RESET(type, Attribute, def)                                                                                           \
    _NETUDP_PROPERTY_SIGNAL(type, attribute)

#define NETUDP_ABSTRACT_PROPERTY_D(type, attribute, Attribute, def)                                                                        \
    _NETUDP_Q_PROPERTY(type, attribute, Attribute)                                                                                         \
    _NETUDP_ABSTRACT_PROPERTY_SHARED(type, attribute, Attribute, def)

#define NETUDP_ABSTRACT_PROPERTY(type, attribute, Attribute) NETUDP_ABSTRACT_PROPERTY_D(type, attribute, Attribute, {})

#define NETUDP_ABSTRACT_PROPERTY_RO_D(type, attribute, Attribute, def)                                                                     \
    _NETUDP_Q_PROPERTY_RO(type, attribute, Attribute)                                                                                      \
    _NETUDP_ABSTRACT_PROPERTY_SHARED(type, attribute, Attribute, def)

#define NETUDP_ABSTRACT_PROPERTY_RO(type, attribute, Attribute) NETUDP_ABSTRACT_PROPERTY_RO_D(type, attribute, Attribute, {})

#define NETUDP_IMPL_PROPERTY_D(type, attribute, Attribute, def)                                                                            \
    _NETUDP_PROPERTY_MEMBER(type, attribute, def)                                                                                          \
    _NETUDP_PROPERTY_GETTER(type, attribute, override)                                                                                     \
    _NETUDP_PROPERTY_SETTER(type, attribute, Attribute, override)

#define NETUDP_IMPL_PROPERTY(type, attribute, Attribute) NETUDP_IMPL_PROPERTY_D(type, attribute, Attribute, {})

#define _NETUDP_PROPERTY_SHARED(type, attribute, Attribute, def)                                                                           \
    _NETUDP_PROPERTY_MEMBER(type, attribute, def)                                                                                          \
    _NETUDP_PROPERTY_GETTER(type, attribute, )                                                                                             \
    _NETUDP_PROPERTY_SETTER(type, attribute, Attribute, )                                                                                  \
    _NETUDP_PROPERTY_RESET(type, Attribute, def)                                                                                           \
    _NETUDP_PROPERTY_SIGNAL(type, attribute)

#define NETUDP_PROPERTY_D(type, attribute, Attribute, def)                                                                                 \
    _NETUDP_Q_PROPERTY(type, attribute, Attribute)                                                                                         \
    _NETUDP_PROPERTY_SHARED(type, attribute, Attribute, def)

#define NETUDP_PROPERTY(type, attribute, Attribute) NETUDP_PROPERTY_D(type, attribute, Attribute, {})

#define NETUDP_PROPERTY_RO_D(type, attribute, Attribute, def)                                                                              \
    _NETUDP_Q_PROPERTY_RO(type, attribute, Attribute)                                                                                      \
    _NETUDP_PROPERTY_SHARED(type, attribute, Attribute, def)

#define NETUDP_PROPERTY_RO(type, attribute, Attribute) NETUDP_PROPERTY_RO_D(type, attribute, Attribute, {})

#define _NETUDP_ATTRIBUTE_SHARED(type, attribute, Attribute, def)                                                                          \
    _NETUDP_PROPERTY_MEMBER(type, attribute, def)                                                                                          \
    _NETUDP_PROPERTY_GETTER(type, attribute, )                                                                                             \
    _NETUDP_ATTRIBUTE_SETTER(type, attribute, Attribute, )

#define NETUDP_PROPERTY_CONST_D(type, attribute, Attribute, def)                                                                           \
    _NETUDP_Q_PROPERTY_CONST(type, attribute)                                                                                              \
    _NETUDP_ATTRIBUTE_SHARED(type, attribute, Attribute, def)

#define NETUDP_PROPERTY_CONST(type, attribute, Attribute) NETUDP_PROPERTY_CONST_D(type, attribute, Attribute, {})

#define NETUDP_ATTRIBUTE_D(type, attribute, Attribute, def) _NETUDP_ATTRIBUTE_SHARED(type, attribute, Attribute, def)

#define NETUDP_ATTRIBUTE(type, attribute, Attribute) NETUDP_ATTRIBUTE_D(type, attribute, Attribute, {})

#define NETUDP_REGISTER_TO_QML(Type)                                                                                                       \
public:                                                                                                                                    \
    static void registerToQml(const char* uri, const int majorVersion, const int minorVersion, const char* name = #Type)                   \
    {                                                                                                                                      \
        qmlRegisterType<Type>(uri, majorVersion, minorVersion, name);                                                                      \
    }                                                                                                                                      \
                                                                                                                                           \
private:
#define NETUDP_REGISTER_UNCREATABLE_TO_QML(Type)                                                                                           \
public:                                                                                                                                    \
    static void registerToQml(const char* uri, const int majorVersion, const int minorVersion, const char* name = #Type)                   \
    {                                                                                                                                      \
        qmlRegisterUncreatableType<Type>(uri, majorVersion, minorVersion, name, "");                                                       \
    }                                                                                                                                      \
                                                                                                                                           \
private:

#define NETUDP_SINGLETON_IMPL(Class, name, Name)                                                                                           \
public:                                                                                                                                    \
    static Class& name()                                                                                                                   \
    {                                                                                                                                      \
        static Class ret;                                                                                                                  \
        return ret;                                                                                                                        \
    }                                                                                                                                      \
    static QObject* set##Name(QQmlEngine* qmlEngine, QJSEngine* jsEngine)                                                                  \
    {                                                                                                                                      \
        Q_UNUSED(jsEngine)                                                                                                                 \
        Q_UNUSED(qmlEngine)                                                                                                                \
        QObject* ret = &name();                                                                                                            \
        QQmlEngine::setObjectOwnership(ret, QQmlEngine::CppOwnership);                                                                     \
        return ret;                                                                                                                        \
    }                                                                                                                                      \
    static void registerSingleton(const char* uri, const int majorVersion, const int minorVersion, const char* n = #Name)                  \
    {                                                                                                                                      \
        qmlRegisterSingletonType<Class>(uri, majorVersion, minorVersion, n, &Class::set##Name);                                            \
    }

#endif
