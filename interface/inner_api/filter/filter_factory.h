/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef HISTREAMER_PIPELINE_FILTER_FACTORY_H
#define HISTREAMER_PIPELINE_FILTER_FACTORY_H

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "filter/filter.h"
#include "cpp_ext/type_cast_ext.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
using InstanceGenerator = std::function<std::shared_ptr<Filter>(const std::string&, const FilterType type)>;

class FilterFactory {
public:
    ~FilterFactory() = default;

    FilterFactory(const FilterFactory&) = delete;

    FilterFactory operator=(const FilterFactory&) = delete;

    static FilterFactory& Instance();

    template <typename T>
    std::shared_ptr<T> CreateFilter(const std::string& filterName, const FilterType type)
    {
        auto filter = CreateFilterPriv(filterName, type);
        auto typedFilter = ReinterpretPointerCast<T>(filter);
        return typedFilter;
    }

    template <typename T>
    void RegisterFilter(const std::string& name, const FilterType type, const InstanceGenerator& generator = nullptr)
    {
        RegisterFilterPriv<T>(name, type, generator);
    }

private:
    FilterFactory() = default;

    std::shared_ptr<Filter> CreateFilterPriv(const std::string& filterName, const FilterType type);

    template <typename T>
    void RegisterFilterPriv(const std::string& name, const FilterType type, const InstanceGenerator& generator)
    {
        if (generator == nullptr) {
            auto result = generators.emplace(
                type, [](const std::string &aliaName, const FilterType type) {
                    return std::make_shared<T>(aliaName, type);
                });
            if (!result.second) {
                result.first->second = generator;
            }
        } else {
            auto result = generators.emplace(type, generator);
            if (!result.second) {
                result.first->second = generator;
            }
        }
    }

    std::unordered_map<FilterType, InstanceGenerator> generators;
};

template <typename T>
class AutoRegisterFilter {
public:
    explicit AutoRegisterFilter(const std::string& name, const FilterType type)
    {
        FilterFactory::Instance().RegisterFilter<T>(name, type);
    }

    AutoRegisterFilter(const std::string& name, const FilterType type, const InstanceGenerator& generator)
    {
        FilterFactory::Instance().RegisterFilter<T>(name, type, generator);
    }

    ~AutoRegisterFilter() = default;
};
} // namespace Pipeline
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PIPELINE_FILTER_FACTORY_H
