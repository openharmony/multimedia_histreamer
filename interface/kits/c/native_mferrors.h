/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#ifndef NATIVE_MFERRORS_H
#define NATIVE_MFERRORS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief MF error code
 * @syscap SystemCapability.Multimedia.Media.xxx
 * @since 11
 */
typedef enum OH_MFErrorCode {
  /**
   * the operation completed successfully.
   */
  MF_ERROR_OK = 0,
  /**
   * permission_denied(AccessToken).
   */
  MF_ERROR_NO_PERMISSION = 201,
  /**
   * invalid argument.
   */
  MF_ERROR_INVALID_VAL = 401,
  /**
   * unsupport interface.
   */
  MF_ERROR_UNSUPPORT = 801,
  /**
   * no memory.
   */
  MF_ERROR_NO_MEMORY = 5400101,
  /**
   * the state is not support this operation.
   */
  MF_ERROR_INVALID_STATE = 5400102,
  /**
   * IO error.
   */
  MF_ERROR_IO = 5400103,
  /**
   * network timeout.
   */
  MF_ERROR_TIMEOUT = 5400104,
  /**
   * unknown error.
   */
  MF_ERROR_UNKNOWN = 5400105,
  /**
   * service died.
   */
  MF_ERROR_SERVICE_DIED = 5400106,
  /**
   * unsupport format.
   */
  MF_ERROR_UNSUPPORTED_FORMAT = 5400107,
  /**
   * opertation not be permitted.
   */
  MF_ERROR_OPERATE_NOT_PERMIT = 5400108,
  /**
   * incompatible version.
   */
  MF_ERROR_INCOMPATIBLE_VERSION = 5400109
} OH_MFErrorCode;

#ifdef __cplusplus
}
#endif

#endif // NATIVE_MFERRORS_H
