
#include "log.h"

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

void initLog()
{
    /*
    boost::log::add_file_log(boost::log::keywords::file_name = "logs/coffee_%m%d%Y_%H%S.log",
                             boost::log::keywords::format = "[%TimeStamp%](%ThreadID%): %Message%",
                             boost::log::keywords::auto_flush = true);
    */

    auto console_log = boost::log::add_console_log(std::cout,
                             boost::log::keywords::format = "%TimeStamp%: %Message%",
                             boost::log::keywords::auto_flush = true);

    console_log->set_filter
    (
        boost::log::trivial::severity >= boost::log::trivial::info
    );

    boost::log::add_common_attributes();
}
