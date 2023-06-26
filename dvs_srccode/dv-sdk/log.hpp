#ifndef DV_SDK_LOG_HPP
#define DV_SDK_LOG_HPP

#include "utils.h"

#include <iostream>
#include <sstream>

/**
 * Header file that includes the C++ wrapper for the DV logging facilities.
 */
namespace dv {

/**
 * DV custom log end type. Can be piped into the log to commit it.
 * Eg. `log.info << "Hello " << "world" << dv::logEnd`.
 */
struct logEndType {};

constexpr logEndType logEnd{};

/**
 * LogStream class. To be instantiated only by dv::Logger. Handles the
 * actual logging and provides functions to stream logging data in as well as
 * to format logging strings.
 * @tparam L The log level for which this stream should be configured.
 */
template<dv::logLevel L>
class LogStream {
private:
	/**
	 * internal string stream that gets used to assemble the log message.
	 */
	std::ostringstream stream_;

public:
	/**
	 * Logs the given argument value.
	 * @tparam T the type of the argument value
	 * @param val the value to be logged
	 */
	template<class T>
	void operator()(const T &val) {
		write(val);
		commit();
	}

	/**
	 * Appends the given argument value to the current log message.
	 * To commit a log message (write it out and flush), one can stream in
	 * `dv::logEnd` or `std::endl`.
	 * @tparam T The type of the value to be appended to the log
	 * @param val The value to be appended to the log
	 * @return A reference to the stream, to stream in further values
	 */
	template<class T>
	LogStream &operator<<(const T &val) {
		write(val);
		return (*this);
	}

	/**
	 * Commits a log message upon reception of an object of the type of
	 * `std::endl`. Committing a message means writing it out and flushing
	 * the buffer.
	 * @return a reference to the log stream for the reception of further
	 * values to be logged.
	 */
	LogStream &operator<<(std::ostream &(*) (std::ostream &) ) {
		commit();
		return (*this);
	}

	/**
	 * Commits a log message upon reception of `dv::logEnd`.
	 * Committing a message means writing it out and flushing
	 * the buffer.
	 * @return a reference to the log stream for the reception of further
	 * values to be logged.
	 */
	LogStream &operator<<(logEndType) {
		commit();
		return (*this);
	}

	/**
	 * Appends the given argument value to the current log message.
	 * To commit a log message (write it out and flush), one can use the function
	 * `commit()`
	 * @tparam T The type of the value to be appended to the log
	 * @param val The value to be appended to the log
	 */
	template<class T>
	void write(const T &val) {
		stream_ << val;
	}

	/**
	 * Commits the current log message. Committing means writing the message
	 * out and flushing the buffer.
	 */
	void commit() {
		dvLog(static_cast<enum dvLogLevel>(L), stream_.str().c_str());
		flush();
	}

	/**
	 * Flushes the buffer without writing out the message.
	 */
	void flush() {
		stream_.str(std::string{});
		stream_.clear();
	}

#if defined(FMT_VERSION) && FMT_VERSION >= 80000

	/**
	 * Formats the given format string with the given arguments and writes it
	 * to the log. Takes any `fmt::format` format string.
	 *
	 * @tparam Args The types of the arguments
	 *
	 * @param format The format string
	 * @param args The argument values to be formatted
	 */
	template<typename... Args>
	void format(const fmt::format_string<Args...> format, Args &&...args) {
		this->operator()(fmt::format(format, std::forward<Args>(args)...));
	}

#else

	/**
	 * Formats the given format string with the given arguments and writes it
	 * to the log. Takes any `fmt::format` format string.
	 *
	 * @tparam S The format string type
	 * @tparam Args The types of the arguments
	 *
	 * @param format The format string
	 * @param args The argument values to be formatted
	 */
	template<typename S, typename... Args>
	void format(const S &format, Args &&...args) {
		this->operator()(fmt::format(format, std::forward<Args>(args)...));
	}

#endif
};

/**
 * Logger class. This class provides logging streams for different log
 * levels to log to.
 */
class Logger {
public:
	LogStream<dv::logLevel::DEBUG> debug;
	LogStream<dv::logLevel::INFO> info;
	LogStream<dv::logLevel::WARNING> warning;
	LogStream<dv::logLevel::ERROR> error;
};

} // namespace dv

#endif // DV_SDK_LOG_HPP
