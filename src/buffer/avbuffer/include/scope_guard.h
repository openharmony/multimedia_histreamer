/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef SCOPE_GUARD_H
#define SCOPE_GUARD_H

#include <functional>

namespace OHOS {
/*
 * The RAII feature is used to implement scope exit protection so that resources
 * can be released and the closure action can be performed in a unified manner when
 * the scope exits.
 */
namespace Detail {
template<typename ExitAction>
class ScopeGuard {
public:
    explicit ScopeGuard(ExitAction &&action) : action_(action), enable_(true) {}
    ~ScopeGuard()
    {
        if (enable_) {
            action_();
        }
    }

    void Disable()
    {
        enable_ = false;
    }

private:
    ExitAction action_;
    bool enable_;
};

struct ScopeExitGuardHelper {};
template<typename ExitAction>
static inline ScopeGuard<ExitAction> operator+(ScopeExitGuardHelper, ExitAction &&action)
{
    return ScopeGuard<ExitAction>(std::forward<ExitAction>(action));
}
}

#define ON_SCOPE_EXIT(id) \
    auto onScopeExitGuard##id = Detail::ScopeExitGuardHelper{} + [ & ]

#define CANCEL_SCOPE_EXIT_GUARD(id) \
    onScopeExitGuard##id.Disable()
}
#endif
