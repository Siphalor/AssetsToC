#ifndef LOG_HPP
#define LOG_HPP
#include <string>
#include <iostream>
#include <fstream>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/expressions/formatters/date_time.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

namespace atoc::log {
	static bool ready = false;

	enum Severity { trace, debug, warning, error, info, fatal };
	BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "AssetsToCSeverity", Severity)
	boost::log::formatting_ostream& operator<<(boost::log::formatting_ostream& strm, boost::log::to_log_manip<Severity,tag::severity> const& manip);
	typedef boost::log::sources::severity_channel_logger<Severity, std::string> logger;

	BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT(log_main, logger) { return atoc::log::logger(boost::log::keywords::channel = "atoc"); }
	BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT(log_parser, logger) { return atoc::log::logger(boost::log::keywords::channel = "pars"); }
	BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT(log_generator, logger) { return atoc::log::logger(boost::log::keywords::channel = "gent"); }

	void init(Severity sev, std::string file);
}

#define ATOC_LOG(Channel, Sev) BOOST_LOG_SEV(atoc::log::log_##Channel::get(), atoc::log::Severity::Sev)

#endif
