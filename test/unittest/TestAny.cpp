/*
 * Copyright (c) 2021-2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstdlib>
#include <memory>
#include <string>
#include "gtest/gtest.h"
#define private public
#define protected public
#include "plugin/common/any.h"

using namespace testing::ext;

namespace OHOS {
namespace Media {
namespace Test {
using namespace OHOS::Media::Plugin;
bool CompareFunctionTable(const Any::FunctionTable* ft1, const Any::FunctionTable* ft2)
{
#ifndef HST_ANY_WITH_NO_RTTI
    return ft1->type == ft2->type && ft1->destroy == ft2->destroy && ft1->getPtr == ft2->getPtr &&
           ft1->move == ft2->move && ft1->copy == ft2->copy && ft1->getConstPtr == ft2->getConstPtr;
#else
    return ft1->type_name == ft2->type_name && ft1->destroy == ft2->destroy && ft1->getPtr == ft2->getPtr &&
           ft1->move == ft2->move && ft1->copy == ft2->copy && ft1->getConstPtr == ft2->getConstPtr;
#endif
}

template <typename T, template <class> class U>
bool UseStorage()
{
    auto ft1 = Any::GetFunctionTable<T>();
    Any::FunctionTable ft2 {
#ifndef HST_ANY_WITH_NO_RTTI
        .type = U<T>::Type,
#else
        .type_name = U<T>::TypeName,
#endif
        .destroy = U<T>::Destroy,
        .copy = U<T>::Copy,
        .move = U<T>::Move,
        .getPtr = U<T>::GetPtr,
        .getConstPtr = U<T>::GetConstPtr
    };
    return CompareFunctionTable(ft1, &ft2);
}

class NonTrivialCopyable {
public:
   explicit NonTrivialCopyable(uint8_t a) : a_(a)
    {
    }
    NonTrivialCopyable(const NonTrivialCopyable& a) : a_(a.a_)
    {
    }
    NonTrivialCopyable& operator=(const NonTrivialCopyable& other)
    {
        if (this != &other) {
            this->a_ = other.a_;
        }
        return *this;
    }
    uint8_t a_;
};

class NonTrivialNonThrowMove {
public:
    explicit NonTrivialNonThrowMove(uint8_t a) : a_(a)
    {
    }
    explicit NonTrivialNonThrowMove(const NonTrivialNonThrowMove& a) : a_(a.a_)
    {
    }
    NonTrivialNonThrowMove& operator=(const NonTrivialNonThrowMove& other)
    {
        if (this != &other) {
            a_ = other.a_;
        }
        return *this;
    }
    NonTrivialNonThrowMove(NonTrivialNonThrowMove&& t) noexcept
    {
        a_ = t.a_;
    }
    NonTrivialNonThrowMove& operator=(NonTrivialNonThrowMove&& other)
    {
        a_ = other.a_;
        return *this;
    }
    uint8_t a_;
};

HWTEST(AnyTest, testAnyUseTrivial, TestSize.Level1)
{
    auto tmp = UseStorage<uint8_t, Any::TrivialStackFunctionTable>();
    ASSERT_TRUE(tmp);
    tmp = UseStorage<int8_t, Any::TrivialStackFunctionTable>();
    ASSERT_TRUE(tmp);
    tmp = UseStorage<uint16_t, Any::TrivialStackFunctionTable>();
    ASSERT_TRUE(tmp);
    tmp = UseStorage<int16_t, Any::TrivialStackFunctionTable>();
    ASSERT_TRUE(tmp);
    tmp = UseStorage<uint32_t, Any::TrivialStackFunctionTable>();
    ASSERT_TRUE(tmp);
    tmp = UseStorage<int32_t, Any::TrivialStackFunctionTable>();
    ASSERT_TRUE(tmp);
    tmp = UseStorage<uint64_t, Any::TrivialStackFunctionTable>();
    ASSERT_TRUE(tmp);
    tmp = UseStorage<int64_t, Any::TrivialStackFunctionTable>();
    ASSERT_TRUE(tmp);
    tmp = UseStorage<NonTrivialCopyable, Any::TrivialStackFunctionTable>();
    ASSERT_FALSE(tmp);
}

HWTEST(AnyTest, testAnyUseStack, TestSize.Level1)
{
    auto tmp = UseStorage<NonTrivialCopyable, Any::StackFunctionTable>();
    ASSERT_FALSE(tmp);
    tmp = UseStorage<NonTrivialNonThrowMove, Any::StackFunctionTable>();
    ASSERT_TRUE(tmp);
    tmp = UseStorage<std::shared_ptr<int>, Any::StackFunctionTable>();
    ASSERT_TRUE(tmp);
}

HWTEST(AnyTest, testAnyUseHeap, TestSize.Level1)
{
    auto tmp = UseStorage<NonTrivialCopyable, Any::HeapFunctionTable>();
    ASSERT_TRUE(tmp);
}

HWTEST(AnyTest, testAnyCast, TestSize.Level1)
{
    const int number = 12;
    auto a = Any(number);
    int* i = AnyCast<int>(&a);
    ASSERT_TRUE(*i == number);
    a = std::string("hello");
    auto& ra = AnyCast<std::string&>(a);
    ra[1] = 'o';
    const auto& refString = AnyCast<const std::string&>(a);
    ASSERT_STREQ(refString.c_str(), "hollo");
    auto copyString = AnyCast<std::string>(a);
    copyString[1] = 'l';
    ASSERT_STREQ(refString.c_str(), "hollo");
    ASSERT_STREQ(copyString.c_str(), "hlllo");

    ASSERT_THROW(AnyCast<std::string>("test"), BadAnyCast);
    ASSERT_THROW(AnyCast<char*>("test"), BadAnyCast);
}

class Star {
    std::string name_;
    int id_;

public:
    Star(std::string name, int id) : name_ {std::move(name)}, id_ {id}
    {
    }

    bool operator==(const Star& other) const
    {
        return name_ == other.name_ && id_ == other.id_;
    }
};

HWTEST(AnyTest, testAnyEmplace, TestSize.Level1)
{
    Any celestial;
    celestial.Emplace<Star>("Procyon", 2943);
    Star star1("Procyon", 2943);
    const auto* star = AnyCast<Star>(&celestial);
    ASSERT_TRUE(star1 == (*star));

    Any av;
    av.Emplace<std::vector<char>>({'C', '+', '+', '1', '7'});
    const auto* va = AnyCast<std::vector<char>>(&av);
    std::vector<char> vector1({'C', '+', '+', '1', '7'});
    ASSERT_TRUE(vector1 == (*va));
}

HWTEST(AnyTest, testMakeAny, TestSize.Level1)
{
    Star star1("Procyon", 2943);
    Any celestial = MakeAny<Star, std::string, int32_t>("Procyon", 2943);
    ASSERT_TRUE(AnyCast<Star>(celestial) == star1);

    std::vector<char> vector1({'C', '+', '+', '1', '7'});
    Any av = MakeAny<std::vector<char>>({'C', '+', '+', '1', '7'});
    ASSERT_TRUE(AnyCast<std::vector<char>>(av) == vector1);
}

HWTEST(AnyTest, testSwap, TestSize.Level1)
{
    NonTrivialNonThrowMove n1(100);
    Any a1 = n1;
    Any a2 = 4;
    a1.Swap(a2);
    ASSERT_TRUE(AnyCast<int32_t>(a1) == 4);
    ASSERT_EQ(AnyCast<NonTrivialNonThrowMove>(a2).a_, n1.a_);

    a2 = "test";
    const Star star("star", 100);
    a1 = star;
    a2.Swap(a1);
    ASSERT_STREQ(AnyCast<const char*>(a1), "test");
    ASSERT_TRUE(AnyCast<Star>(a2) == star);

    std::swap(a1, a2);
    ASSERT_STREQ(AnyCast<const char*>(a2), "test");
    ASSERT_TRUE(AnyCast<Star>(a1) == star);
}

HWTEST(AnyTest, testHasValue_Reset, TestSize.Level1)
{
    Any a;
    ASSERT_FALSE(a.HasValue());
    a = "test";
    ASSERT_TRUE(a.HasValue());
    a.Reset();
    ASSERT_FALSE(a.HasValue());
}
} // namespace Test
} // namespace Media
} // namespace OHOS