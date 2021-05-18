// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Library Headers
#include <Net/Udp/Logger.hpp>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

using namespace net::udp;

template<typename... Args>
static Logger::LogPtr makeLog(Args&&... args)
{
    return std::make_shared<Logger::Log>(std::forward<Args>(args)...);
}

const char* const Logger::WORKER_NAME = "net.udp.worker";
const char* const Logger::SERVER_NAME = "net.udp.socket";
const char* const Logger::UTILS_NAME = "net.udp.utils";

const Logger::LogPtr Logger::WORKER = makeLog(WORKER_NAME);
const Logger::LogPtr Logger::SERVER = makeLog(SERVER_NAME);
const Logger::LogPtr Logger::UTILS = makeLog(UTILS_NAME);

const Logger::LogList Logger::LOGGERS = {WORKER, SERVER, UTILS};

// ─────────────────────────────────────────────────────────────
//                  FUNCTIONS
// ─────────────────────────────────────────────────────────────

void Logger::registerSink(const SinkPtr& sink)
{
    for(const auto& it: LOGGERS)
        it->sinks().emplace_back(sink);
}

void Logger::unRegisterSink(const SinkPtr& sink)
{
    for(const auto& it: LOGGERS)
    {
        auto& sinks = it->sinks();

        auto sinkIt = sinks.begin();
        while(sinkIt != sinks.end())
        {
            const auto& s = *sinkIt;
            if(s == sink)
            {
                sinks.erase(sinkIt);
                break;
            }
        }
    }
}
