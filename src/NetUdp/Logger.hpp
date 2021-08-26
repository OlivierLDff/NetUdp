// Copyright 2019 - 2021 Olivier Le Doeuff
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright noticeand this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef __NETUDP_UTILS_LOGGER_HPP__
#define __NETUDP_UTILS_LOGGER_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Library Headers
#include <NetUdp/Export.hpp>

// Dependencies Headers
#include <spdlog/spdlog.h>
#include <spdlog/sinks/sink.h>

// Stl Headers
#include <memory>
#include <set>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

namespace net {
namespace udp {

// ─────────────────────────────────────────────────────────────
//                  CLASS
// ─────────────────────────────────────────────────────────────

/**
 * Define static logger that library use.
 * You need to install sink on them
 */
class NETUDP_API_ Logger
{
    // ─────── TYPES ─────────
public:
    using Log = spdlog::logger;
    using LogPtr = std::shared_ptr<Log>;
    using LogList = std::set<LogPtr>;
    using Sink = spdlog::sinks::sink;
    using SinkPtr = std::shared_ptr<Sink>;

    // ─────── LOGGERS NAME ─────────
public:
    static const char* const WORKER_NAME;
    static const char* const SERVER_NAME;
    static const char* const UTILS_NAME;

    // ─────── LOGGERS ─────────
public:
    static const LogPtr WORKER;
    static const LogPtr SERVER;
    static const LogPtr UTILS;

    // ─────── LIST OF ALL LOGGERS ─────────
public:
    // Loggers
    static const LogList LOGGERS;

    // ─────── API ─────────
public:
    static void registerSink(const SinkPtr& sink);
    static void unRegisterSink(const SinkPtr& sink);
};

}
}

#endif
