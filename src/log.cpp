#include "log.hpp"

namespace atoc::log {
	boost::log::formatting_ostream& operator<<(boost::log::formatting_ostream& strm, boost::log::to_log_manip<Severity,tag::severity> const& manip) {
		static const char* strings[] = {
			"trace", "debug", "warng", "error", "infrm", "fatal"
		};
		Severity level = manip.get();
		if(static_cast<std::size_t>(level) < sizeof(strings) / sizeof(*strings))
			strm << strings[level];
		else
			strm << static_cast<int>(level);
		return strm;
	}

	void init(Severity sev, std::string file) {
		if(ready) return;
		ready = true;

		boost::log::add_common_attributes();

		typedef boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend> text_sink;

		auto form = boost::log::expressions::stream << "[" << boost::log::expressions::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S") << "][" << boost::log::expressions::attr<std::string>("Channel") << "][" << boost::log::expressions::attr<Severity, tag::severity>("Severity") << "] " << boost::log::expressions::smessage;
		boost::shared_ptr<text_sink> sink = boost::make_shared<text_sink>();
		sink->locked_backend()->add_stream(boost::make_shared<std::ofstream>(file.c_str()));
		sink->set_formatter(form);
		boost::log::core::get()->add_sink(sink);

		sink = boost::make_shared<text_sink>();
		sink->locked_backend()->add_stream(boost::shared_ptr<std::ostream>(&std::cout, boost::null_deleter()));
		sink->locked_backend()->auto_flush(true);
		sink->set_filter(boost::log::expressions::attr<Severity>("Severity") >= sev);
		sink->set_formatter(form);
		boost::log::core::get()->add_sink(sink);
	}
}
