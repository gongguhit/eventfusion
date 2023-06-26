#ifndef DV_SDK_STATS_HPP
#define DV_SDK_STATS_HPP

#include "config.hpp"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/rolling_mean.hpp>
#include <boost/accumulators/statistics/rolling_variance.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <chrono>
#include <type_traits>

namespace dv::statistics {

/**
 * Class which provides the functionality to measure statistics for numeric data types.
 * The following statistics are computed and published, along with the most recently added value itself:
 * - mean
 * - variance
 * - minimum
 * - maximum
 * - sample count
 */
template<typename T>
class Stats {
private:
	static_assert(std::is_arithmetic_v<T>, "dv::Statistics::Stats are only supported for arithmetic types.");

protected:
	template<typename... ARGS>
	using accumulator_set = boost::accumulators::accumulator_set<ARGS...>;
	template<typename... ARGS>
	using stats = boost::accumulators::stats<ARGS...>;

	using tag_lazy_rolling_mean     = boost::accumulators::tag::lazy_rolling_mean;
	using tag_lazy_rolling_variance = boost::accumulators::tag::lazy_rolling_variance;
	using tag_min                   = boost::accumulators::tag::min;
	using tag_max                   = boost::accumulators::tag::max;
	using tag_count                 = boost::accumulators::tag::count;

	using accumulator
		= accumulator_set<T, stats<tag_lazy_rolling_mean, tag_lazy_rolling_variance, tag_min, tag_max, tag_count>>;

	static constexpr uint32_t DEFAULT_WINDOW_SIZE{500};
	static constexpr uint32_t PUBLISHING_RATE_MS{300};

	accumulator mAccumulator{boost::accumulators::tag::rolling_window::window_size = DEFAULT_WINDOW_SIZE};
	T mCurrent{};

	uint32_t mWindowSize{DEFAULT_WINDOW_SIZE};
	dv::_RateLimiter mRateLimiter{1, PUBLISHING_RATE_MS};
	std::string mName;
	dv::Config::Node mNode;

public:
	/**
	 * Deleted default constructor
	 */
	Stats() = delete;

	/**
	 * Constructor which selects the default window size @see DEFAULT_WINDOW_SIZE for the rolling mean and variance.
	 *
	 * Creates a node with the name `name` relative to `node`. The stats will be published to the attributes
	 * `current`, `mean`, `var`, `min`, `max` and `count` at this node.
	 *
	 * @param node Parent node relative to which the node for the measured stats should be published
	 * @param name Name of the node
	 */
	Stats(const dv::Config::Node node, const std::string_view name) :
		mName((name.back() == '/') ? name : fmt::format("{:s}/", name)),
		mNode(node.getRelativeNode(mName)) {
		init();
	}

	/**
	 * Constructor which applies a custom window size for the rolling mean and variance.
	 *
	 * Creates a node with the name `name` relative to `node`. The stats will be published to the attributes
	 * `current`, `mean`, `var`, `min`, `max` and `count` at this node.
	 *
	 * @param windowSize size of the accumulator window
	 * @param node Parent node relative to which the node for the measured stats should be published
	 * @param name Name of the node
	 */
	Stats(const uint32_t windowSize, const dv::Config::Node node, const std::string_view name) :
		mAccumulator(boost::accumulators::tag::rolling_window::window_size = windowSize),
		mWindowSize(windowSize),
		mName((name.back() == '/') ? name : fmt::format("{:s}/", name)),
		mNode(node.getRelativeNode(mName)) {
		init();
	}

	/**
	 * Copy - Disallow: since node and name are the same, you'd
	 * suddenly have two objects feeding data into the configuration
	 * backend and overwriting each other. This is not what you want.
	 */
	Stats(const Stats &other)          = delete;
	Stats &operator=(const Stats &rhs) = delete;

	/**
	 * Move
	 */
	Stats(Stats &&other)          = default;
	Stats &operator=(Stats &&rhs) = default;

	/**
	 * Destructor
	 */
	virtual ~Stats() noexcept = default;

	/**
	 * Adds a new value to the underlying accumulator
	 */
	void operator()(const T value) noexcept {
		mCurrent = value;
		mAccumulator(value);

		if (mRateLimiter.pass()) {
			publish();
		}
	}

	/**
	 * Returns the most recently added sample
	 */
	auto currentSample() const noexcept {
		return mCurrent;
	}

	/**
	 * Returns the rolling mean
	 */
	auto mean() const noexcept {
		return boost::accumulators::lazy_rolling_mean(mAccumulator);
	}

	/**
	 * Returns the rolling variance
	 */
	auto var() const noexcept {
		return boost::accumulators::lazy_rolling_variance(mAccumulator);
	}

	/**
	 * Returns the minimum value
	 */
	auto min() const noexcept {
		return boost::accumulators::min(mAccumulator);
	}

	/**
	 * Returns the maximum value
	 */
	auto max() const noexcept {
		return boost::accumulators::max(mAccumulator);
	}

	/**
	 * Returns the number of added samples
	 */
	auto count() const noexcept {
		return boost::accumulators::count(mAccumulator);
	}

private:
	/**
	 * Initializes the underlying accumulators to zero, and creates the node attributes to which data will be published.
	 */
	void init() {
		for (uint16_t i = 0; i < mWindowSize; i++) {
			mAccumulator(T{});
		}

		mNode.create<dv::Config::AttributeTypeConverter<T>::type>("current", 0, {0, std::numeric_limits<T>::max()},
			dv::CfgFlags::READ_ONLY | dv::CfgFlags::NO_EXPORT, "Current value");
		mNode.create<dv::Config::AttributeTypeConverter<T>::type>(
			"mean", 0, {0, std::numeric_limits<T>::max()}, dv::CfgFlags::READ_ONLY | dv::CfgFlags::NO_EXPORT, "Mean");
		mNode.create<dv::Config::AttributeTypeConverter<T>::type>("var", 0, {0, std::numeric_limits<T>::max()},
			dv::CfgFlags::READ_ONLY | dv::CfgFlags::NO_EXPORT, "Variance");
		mNode.create<dv::Config::AttributeTypeConverter<T>::type>(
			"min", 0, {0, std::numeric_limits<T>::max()}, dv::CfgFlags::READ_ONLY | dv::CfgFlags::NO_EXPORT, "Min");
		mNode.create<dv::Config::AttributeTypeConverter<T>::type>(
			"max", 0, {0, std::numeric_limits<T>::max()}, dv::CfgFlags::READ_ONLY | dv::CfgFlags::NO_EXPORT, "Max");
		mNode.create<dv::Config::AttributeTypeConverter<T>::type>(
			"count", 0, {0, std::numeric_limits<T>::max()}, dv::CfgFlags::READ_ONLY | dv::CfgFlags::NO_EXPORT, "Count");
	}

	/**
	 * Publishes the statistical data to the node attributes
	 */
	void publish() noexcept {
		mNode.updateReadOnly<dv::Config::AttributeTypeConverter<T>::type>("current", static_cast<T>(currentSample()));
		mNode.updateReadOnly<dv::Config::AttributeTypeConverter<T>::type>("mean", static_cast<T>(mean()));
		mNode.updateReadOnly<dv::Config::AttributeTypeConverter<T>::type>("var", static_cast<T>(var()));
		mNode.updateReadOnly<dv::Config::AttributeTypeConverter<T>::type>("min", static_cast<T>(min()));
		mNode.updateReadOnly<dv::Config::AttributeTypeConverter<T>::type>("max", static_cast<T>(max()));
		mNode.updateReadOnly<dv::Config::AttributeTypeConverter<T>::type>("count", static_cast<T>(count()));
	}
};

/**
 * Class which provides the functionality to measure statistics for throughput related data.
 */
class Throughput : public Stats<float> {
private:
	static constexpr std::chrono::duration DEFAULT_MEASUREMENT_INTERVAL{std::chrono::milliseconds(1)};

	std::chrono::time_point<std::chrono::steady_clock> mStartTime{std::chrono::steady_clock::now()};
	uint64_t mNumElements{0};
	std::chrono::microseconds mMeasurementInterval{DEFAULT_MEASUREMENT_INTERVAL};

public:
	/**
	 * Deleted default constructor
	 */
	Throughput() = delete;

	/**
	 * Constructor which selects the default measurement interval @see DEFAULT_MEASUREMENT_INTERVAL and window size for
	 * the underlying statistics. @see Stats.
	 *
	 * The data will be published to the node `node/name/throughput/` if a name is specified, or `node/throughput/ if
	 * none is specified. @see Stats
	 *
	 * @param node Parent node relative to which the node for the measured stats should be published
	 * @param name Name of the node
	 */
	Throughput(const dv::Config::Node node, const std::string_view name) : Stats(node, name) {
	}

	/**
	 * Constructor which applies a custom measurement interval and the default window size for
	 * the underlying statistics. @see Stats.
	 *
	 * The data will be published to the node `node/name/throughput/` if a name is specified, or `node/throughput/ if
	 * none is specified. @see Stats
	 *
	 * @param measurementInterval The measurement interval with which the throughput shall be evaluated.
	 * @param node Parent node relative to which the node for the measured stats should be published
	 * @param name Name of the node
	 */
	Throughput(
		const std::chrono::microseconds measurementInterval, const dv::Config::Node node, const std::string_view name) :
		Stats(node, name),
		mMeasurementInterval(measurementInterval) {
	}

	/**
	 * Constructor which applies a custom measurement interval and a custom window size for
	 * the underlying statistics. @see Stats.
	 *
	 * The data will be published to the node `node/name/throughput/` if a name is specified, or `node/throughput/ if
	 * none is specified. @see Stats
	 *
	 * @param windowSize the window size for the statistical accumulators. @See Stats
	 * @param measurementInterval The measurement interval with which the throughput shall be evaluated.
	 * @param node Parent node relative to which the node for the measured stats should be published
	 * @param name Name of the node
	 */
	Throughput(const uint32_t windowSize, const std::chrono::microseconds measurementInterval,
		const dv::Config::Node node, const std::string_view name) :
		Stats(windowSize, node, name),
		mMeasurementInterval(measurementInterval) {
	}

	/**
	 * Copy - Disallow: since node and name are the same, you'd
	 * suddenly have two objects feeding data into the configuration
	 * backend and overwriting each other. This is not what you want.
	 */
	Throughput(const Throughput &other)          = delete;
	Throughput &operator=(const Throughput &rhs) = delete;

	/**
	 * Move
	 */
	Throughput(Throughput &&other)          = default;
	Throughput &operator=(Throughput &&rhs) = default;

	/**
	 * Destructor
	 */
	~Throughput() noexcept = default;

	/**
	 * Updates the statistics by adding elements to the number of elements already received within this interval.
	 *
	 * If an interval has finished since we previously called this method, the throughput is computed and added to the
	 * underlying statistical accumulators. @see Stats.
	 *
	 * @param elements The number of elements to be added to the throughput computation
	 */
	inline void add(const uint64_t elements) noexcept {
		if (const auto now = std::chrono::steady_clock::now(); now >= mStartTime + mMeasurementInterval) {
			const float elementsPerS
				= static_cast<float>(mNumElements * 1000000)
				/ static_cast<float>(std::chrono::duration_cast<std::chrono::microseconds>(now - mStartTime).count());

			Stats::operator()(elementsPerS);

			mNumElements = 0;
			mStartTime   = now;
		}

		mNumElements += elements;
	}
};

/**
 * Class which provides the functionality to measure statistics for cycle time related data.
 */
class CycleTime : public Stats<float> {
private:
	std::chrono::time_point<std::chrono::steady_clock> mStartTime{std::chrono::steady_clock::now()};

public:
	/**
	 * Deleted default constructor
	 */
	CycleTime() = delete;

	/**
	 * Constructor which selects the default window size for the underlying statistics. @see Stats.
	 *
	 * The data will be published to the node `node/name/cycleTime/` if a name is specified, or `node/cycleTime/ if
	 * none is specified. @see Stats
	 *
	 * @param node Parent node relative to which the node for the measured stats should be published
	 * @param name Name of the node
	 */
	CycleTime(const dv::Config::Node node, const std::string_view name) : Stats(node, name) {
	}

	/**
	 * Constructor which applies the a custom window size for the underlying statistics. @see Stats.
	 *
	 * The data will be published to the node `node/name/cycleTime/` if a name is specified, or `node/cycleTime/ if
	 * none is specified. @see Stats
	 *
	 * @param node Parent node relative to which the node for the measured stats should be published
	 * @param name Name of the node
	 */
	CycleTime(const uint32_t windowSize, const dv::Config::Node node, const std::string_view name) :
		Stats(windowSize, node, name) {
	}

	/**
	 * Copy - Disallow: since node and name are the same, you'd
	 * suddenly have two objects feeding data into the configuration
	 * backend and overwriting each other. This is not what you want.
	 */
	CycleTime(const CycleTime &other)          = delete;
	CycleTime &operator=(const CycleTime &rhs) = delete;

	/**
	 * Move
	 */
	CycleTime(CycleTime &&other)          = default;
	CycleTime &operator=(CycleTime &&rhs) = default;

	/**
	 * Destructor
	 */
	~CycleTime() noexcept = default;

	/**
	 * Starts a measurement
	 */
	inline void start() noexcept {
		mStartTime = std::chrono::steady_clock::now();
	}

	/**
	 * Finishes a measurement and adds it to the underlying statistical accumulators
	 */
	inline void finish() noexcept {
		const auto now = std::chrono::steady_clock::now();

		const int64_t cycleTime = std::chrono::duration_cast<std::chrono::microseconds>(now - mStartTime).count();

		Stats::operator()(static_cast<float>(cycleTime));
	}
};

} // namespace dv::statistics

#endif // DV_SDK_STATS_HPP
