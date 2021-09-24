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

#ifndef HISTREAMER_FOUNDATION_META_H
#define HISTREAMER_FOUNDATION_META_H

#include <cstdint>
#include <map>
#include <string>

#include "plugin/common/plugin_types.h"
#include "plugin/core/plugin_meta.h"

namespace OHOS {
namespace Media {
class Meta {
public:
    explicit Meta()= default;
    ~Meta();
    bool Empty() const;
    bool SetString(Plugin::MetaID, const std::string &);
    bool SetInt32(Plugin::MetaID, int32_t);
    bool SetUint32(Plugin::MetaID, uint32_t);
    bool SetInt64(Plugin::MetaID, int64_t);
    bool SetUint64(Plugin::MetaID, uint64_t);
    bool SetFloat(Plugin::MetaID, float);

    /**
     * this function will copy from ptr with size.
     * @param ptr pointer
     * @param size size
     * @return
     */
    bool SetPointer(Plugin::MetaID, const void* ptr, size_t size);

    bool GetString(Plugin::MetaID, std::string &) const;
    bool GetInt32(Plugin::MetaID, int32_t &) const;
    bool GetUint32(Plugin::MetaID, uint32_t &) const;
    bool GetInt64(Plugin::MetaID, int64_t &) const;
    bool GetUint64(Plugin::MetaID, uint64_t &) const;
    bool GetFloat(Plugin::MetaID, float &) const;

    /**
     * this function will copy from inner storage with size if exists. The user should delete the memory when it's not
     * needed.
     * @param ptr pointer
     * @param size size
     * @return
     */
    bool GetPointer(Plugin::MetaID, void **ptr, size_t &size) const;

    template<typename T>
    bool GetData(Plugin::MetaID id, T &value) const
    {
        auto ite = items_.find(id);
        if (ite == items_.end() || typeid(T) != ite->second.Type()) {
            return false;
        }
        value = Plugin::AnyCast<T>(ite->second);
        return true;
    }

    void Clear();

    /**
     * remove item with the input name
     * @param name target name
     * @return true if the target exists; or false.
     */
    bool Remove(Plugin::MetaID id);

    void Update(const Meta& meta);

    /**
     * user should always use like SetData<T> to set data, otherwise it may be wrong to GetData.
     * e.g. for string  if use SetData(id, "abc") then GetData<std::string>(id, value) cannot work.
     * for uint64 if use SetData(id, 1), then GetData<uint64>(id, value) cannot work.
     *
     * @tparam T
     * @param id
     * @param value
     * @return
     */
    template<typename T>
    bool SetData(Plugin::MetaID id, const T &value)
    {
        items_[id] = value;
        return true;
    }

private:
    std::map<Plugin::MetaID, Plugin::ValueType> items_ {};
};
}
}
#endif // HISTREAMER_FOUNDATION_META_H
