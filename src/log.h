#define BOOST_ALL_DYN_LINK 1
#define BOOST_LOG_DYN_LINK 1

#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED

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

using sl = boost::log::trivial::severity_level;
using Logger = boost::log::sources::severity_logger_mt<sl>;

void initLog();

static Logger test_logger;

#define LOG(exp) BOOST_LOG_SEV(test_logger, sl::info) << exp

#endif
