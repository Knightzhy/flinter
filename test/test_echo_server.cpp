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

#include <gtest/gtest.h>

#include <netinet/in.h>

#include "flinter/linkage/file_descriptor_io.h"
#include "flinter/linkage/interface.h"
#include "flinter/linkage/listener.h"
#include "flinter/linkage/linkage_handler.h"
#include "flinter/linkage/linkage_peer.h"
#include "flinter/linkage/linkage_worker.h"
#include "flinter/linkage/linkage.h"
#include "flinter/logger.h"
#include "flinter/signals.h"

static flinter::LinkageHandler *g_handler;
static flinter::LinkageWorker *g_worker;

class H : public flinter::LinkageHandler {
public:
    virtual ~H() {}
    virtual ssize_t GetMessageLength(flinter::Linkage * /*linkage*/,
                                     const void * /*buffer*/, size_t length)
    {
        return static_cast<ssize_t>(length);
    }

    virtual int OnMessage(flinter::Linkage *linkage, const void *buffer, size_t length)
    {
        linkage->Send(buffer, length);
        if (memcmp(buffer, "quit\r\n", 6) == 0 ||
            memcmp(buffer, "quit\n", 5) == 0   ){

            if (!linkage->Send("QUIT\r\n", 6)) {
                return -1;
            }

            return linkage->Disconnect();
        }

        return 1;
    }

}; // class H

class L : public flinter::Listener {
public:
    virtual ~L() {}
protected:
    virtual flinter::LinkageBase *CreateLinkage(flinter::LinkageWorker * /*worker*/,
                                                const flinter::LinkagePeer &peer,
                                                const flinter::LinkagePeer &me)
    {
        LOG(INFO) << "Accept: " << me.ip_str() << ":" << me.port()
                  << " <- " << peer.ip_str() << ":" << peer.port();

        flinter::Interface *i = new flinter::Interface;
        i->Accepted(peer.fd());
        flinter::FileDescriptorIo *io = new flinter::FileDescriptorIo(i, false);
        flinter::Linkage *l = new flinter::Linkage(io, g_handler, peer, me);
        l->set_receive_timeout(15000000000);
        return l;
    }

}; // class L

static void on_signal_quit(int /*signum*/)
{
    g_worker->Shutdown();
}

TEST(echoServer, TestListen)
{
    flinter::Logger::SetFilter(flinter::Logger::kLevelVerbose);

    signals_set_handler(SIGHUP,  on_signal_quit);
    signals_set_handler(SIGINT,  on_signal_quit);
    signals_set_handler(SIGQUIT, on_signal_quit);
    signals_set_handler(SIGTERM, on_signal_quit);

    H handler;
    g_handler = &handler;

    L listener;
    flinter::Interface::Socket s;
    s.type = SOCK_STREAM;
    s.socket_bind_port = 5566;

    flinter::Interface::Option o;
    o.socket_non_blocking = true;
    o.socket_close_on_exec = true;
    ASSERT_TRUE(listener.Listen(s, o));

    flinter::LinkageWorker worker;
    ASSERT_TRUE(listener.Attach(&worker));

    g_worker = &worker;
    ASSERT_TRUE(worker.Run());
}
