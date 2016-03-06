/* Copyright 2014 yiyuanzhong@gmail.com (Yiyuan Zhong)
 *
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

#ifndef __FLINTER_LINKAGE_EASY_HANDLER_H__
#define __FLINTER_LINKAGE_EASY_HANDLER_H__

#include <sys/types.h>

namespace flinter {

class EasyContext;

class EasyHandler {
public:
    virtual ~EasyHandler() {}

    /// Called within I/O threads.
    ///
    /// Return the packet size even if it's incomplete as long as you can tell.
    /// Very handy when you write the message length in the header.
    ///
    /// @return >0 message length.
    /// @return  0 message length is yet determined, keep receiving.
    /// @return <0 message is invalid.
    virtual ssize_t GetMessageLength(const EasyContext &context,
                                     const void *buffer,
                                     size_t length) = 0;

    /// Called within worker threads, in case there's none, within I/O threads.
    ///
    /// @return >0 keep coming.
    /// @return  0 hang up gracefully.
    /// @return <0 error occurred, hang up immediately.
    virtual int OnMessage(const EasyContext &context,
                          const void *buffer,
                          size_t length) = 0;

    /// Called within I/O threads.
    virtual void OnDisconnected(const EasyContext &context);

    /// Called within I/O threads.
    virtual bool OnConnected(const EasyContext &context);

    /// Called within I/O threads.
    virtual void OnError(const EasyContext &context,
                         bool reading_or_writing,
                         int errnum);

    /// Called within I/O threads.
    virtual bool Cleanup(const EasyContext &context, int64_t now);

}; // class EasyHandler

} // namespace flinter

#endif // __FLINTER_LINKAGE_EASY_HANDLER_H__
