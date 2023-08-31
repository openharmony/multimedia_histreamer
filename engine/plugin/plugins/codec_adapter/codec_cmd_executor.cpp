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
#if defined(VIDEO_SUPPORT)

#define HST_LOG_TAG "CodecCmdExecutor"

#include "codec_cmd_executor.h"
#include "codec_utils.h"
#include "foundation/log.h"
#include "hdf_base.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace CodecAdapter {
CodecCmdExecutor::CodecCmdExecutor(CodecComponentType* component, uint32_t inPortIndex)
    : codecComp_(component), inPortIndex_(inPortIndex)
{
    resultMap_[OMX_CommandStateSet] = OMX_StateInvalid;
    resultMap_[OMX_CommandFlush] = std::pair<Result, Result>{Result::INVALID, Result::INVALID};
    resultMap_[OMX_CommandPortEnable] = std::pair<Result, Result>{Result::INVALID, Result::INVALID};
    resultMap_[OMX_CommandPortDisable] = std::pair<Result, Result>{Result::INVALID, Result::INVALID};
}

Status CodecCmdExecutor::OnEvent(OMX_EVENTTYPE event, EventInfo* info)
{
    MEDIA_LOG_I("OnEvent begin - eEvent: " PUBLIC_LOG_D32 ", nData1: " PUBLIC_LOG_U32 ", nData2: " PUBLIC_LOG_U32,
        static_cast<int>(event), info->data1, info->data2);
    switch (event) {
        case OMX_EventCmdComplete:
            HandleEventCmdComplete(info->data1, info->data2); // data2 indicates a state
            break;
        case OMX_EventPortSettingsChanged:
            HandleEventPortSettingsChanged(info->data1, info->data2);
            break;
        case OMX_EventBufferFlag:
            HandleEventBufferFlag(info->data1, info->data2);
            break;
        case OMX_EventError:
            HandleEventError(info->data1, info->data2);
            break;
        default:
            break;
    }
    MEDIA_LOG_D("OnEvent end");
    return Status::OK;
}

Status CodecCmdExecutor::SendCmd(OMX_COMMANDTYPE cmd, const Plugin::Any& param)
{
    MEDIA_LOG_D("SendCmd Start");
    switch (cmd) {
        case OMX_CommandStateSet: {
            resultMap_[cmd] = OMX_StateInvalid;
            auto ret = HdiSendCommand(codecComp_, cmd,  Plugin::AnyCast<OMX_STATETYPE>(param), 0);
            FALSE_RETURN_V_MSG(ret == HDF_SUCCESS, Status::ERROR_INVALID_OPERATION, "HdiSendCommand failed");
            break;
        }
        case OMX_CommandFlush: {
            uint32_t portIndex;
            if (Plugin::Any::IsSameTypeWith<int32_t >(param) && Plugin::AnyCast<int32_t>(param) == -1) {
                portIndex = static_cast<uint32_t>(-1);
            } else {
                portIndex = Plugin::AnyCast<uint32_t>(param);
            }
            auto ret = HdiSendCommand(codecComp_, cmd, portIndex, 0);
            FALSE_RETURN_V_MSG(ret == HDF_SUCCESS, Status::ERROR_INVALID_OPERATION, "HdiSendCommand failed");
            break;
        }
        case OMX_CommandPortEnable:
        case OMX_CommandPortDisable: {
            auto ret = HdiSendCommand(codecComp_, cmd, Plugin::AnyCast<uint32_t>(param), 0);
            FALSE_RETURN_V_MSG(ret == HDF_SUCCESS, Status::ERROR_INVALID_OPERATION, "HdiSendCommand failed");
            break;
        }
        default:
            break;
    }
    return Status::OK;
}

bool CodecCmdExecutor::WaitCmdResult(OMX_COMMANDTYPE cmd, const Plugin::Any& param)
{
    OSAL::ScopedLock lock(mutex_);
    MEDIA_LOG_D("WaitCmdResult lastCmd: " PUBLIC_LOG_D32 ", cmd:" PUBLIC_LOG_D32, lastCmd_,  static_cast<int32_t>(cmd));
    bool result {true};
    static constexpr int32_t timeout = 2000; // ms
    switch (cmd) {
        case OMX_CommandStateSet: {
            cond_.WaitFor(lock, timeout, [&] {
                if (lastCmd_ == -1) {
                    resultMap_[cmd] = OMX_StateInvalid;
                    result = false;
                    return true;
                }
                return Plugin::AnyCast<OMX_STATETYPE>(resultMap_[cmd]) == AnyCast<OMX_STATETYPE>(param);
            });
            return result;
        }
        case OMX_CommandFlush:
        case OMX_CommandPortEnable:
        case OMX_CommandPortDisable: {
            auto portIndex = AnyCast<uint32_t>(param);
            cond_.WaitFor(lock, timeout, [&] {
                auto tempPair = AnyCast<std::pair<Result, Result>>(resultMap_[cmd]);
                if (lastCmd_ == -1) {
                    if (portIndex == inPortIndex_) {
                        resultMap_[cmd] = std::pair<Result, Result>{Result::FAIL, tempPair.second};
                    } else {
                        resultMap_[cmd] = std::pair<Result, Result>{tempPair.second, Result::FAIL};
                    }
                    result = false;
                    return true;
                }
                if (portIndex == inPortIndex_) {
                    if (tempPair.first != Result::INVALID) {
                        resultMap_[cmd] = std::pair<Result, Result>{Result::INVALID, tempPair.second};
                    }
                    return tempPair.first == Result::SUCCESS;
                } else {
                    if (tempPair.second != Result::INVALID) {
                        resultMap_[cmd] = std::pair<Result, Result>{tempPair.second, Result::INVALID};
                    }
                    return tempPair.second == Result::SUCCESS;
                }
            });
            return result;
        }
        default:
            return true;
    }
}

Status CodecCmdExecutor::SetCallback(Callback* cb)
{
    callback_ = cb;
    return Status::OK;
}

void CodecCmdExecutor::HandleEventCmdComplete(uint32_t data1, uint32_t data2)
{
    MEDIA_LOG_D("HandleEventCmdComplete begin");
    OSAL::ScopedLock lock(mutex_);
    auto cmd = static_cast<OMX_COMMANDTYPE>(data1);
    switch (data1) {
        case OMX_CommandStateSet:
            resultMap_[cmd] = static_cast<OMX_STATETYPE>(data2);
            break;
        case OMX_CommandFlush:
        case OMX_CommandPortEnable:
        case OMX_CommandPortDisable: {
            auto tempPair = AnyCast<std::pair<Result, Result>>(resultMap_[cmd]);
            if (data2 == inPortIndex_) {
                resultMap_[cmd] = std::pair<Result, Result>{Result::SUCCESS, tempPair.second};
            } else {
                resultMap_[cmd] = std::pair<Result, Result>{tempPair.second, Result::SUCCESS};
            }
            break;
        }
        default:
            break;
    }
    lastCmd_ = static_cast<int32_t>(cmd);
    cond_.NotifyAll();
    MEDIA_LOG_D("HandelCmdCompleteEvent end");
}

void CodecCmdExecutor::HandleEventPortSettingsChanged(OMX_U32 data1, OMX_U32 data2)
{
    MEDIA_LOG_I("HandleEventPortSettingsChanged begin");
    OSAL::ScopedLock lock(mutex_);
}

void CodecCmdExecutor::HandleEventBufferFlag(OMX_U32 data1, OMX_U32 data2)
{
    MEDIA_LOG_I("HandleEventBufferFlag begin");
    OSAL::ScopedLock lock(mutex_);
    if (data1 == 1 && (data2 & OMX_BUFFERFLAG_EOS)) {
        MEDIA_LOG_D("it is eos, wait buffer eos");
    }
}

void CodecCmdExecutor::HandleEventError(OMX_U32 data1, OMX_U32 data2)
{
    {
        OSAL::ScopedLock lock(mutex_);
        lastCmd_ = -1;
        cond_.NotifyAll();
    }
    // Sometimes, hdi return data1 does not indicate an OMX_ERRORTYPE, call OmxErrorType2String() return OMX_ErrorNone
    auto errorType = OmxErrorType2String(data1);
    MEDIA_LOG_E("HandleEventError begin, error msg: " PUBLIC_LOG_S, errorType.c_str());
    if (errorType == "OMX_ErrorNone" && static_cast<OMX_INDEXTYPE>(data2) == OMX_IndexParamPortDefinition) {
        if (data1 == inPortIndex_) {
            MEDIA_LOG_E("Unknown error on input port");
        } else {
            MEDIA_LOG_E("Input data does not contain keyframes, unable to obtain output data.");
        }
    }
}
} // namespace CodecAdapter
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif