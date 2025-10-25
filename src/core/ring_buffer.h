/**
 * @file ring_buffer.h
 * @brief Generic ring buffer for time-series telemetry data
 */

#ifndef CORE_RING_BUFFER_H
#define CORE_RING_BUFFER_H

#include <deque>
#include <limits>

/**
 * @brief Generic time-series ring buffer with automatic pruning
 *
 * **Web Analogy:**
 * Like a JavaScript array with automatic size management:
 * ```javascript
 * const buffer = new RingBuffer(maxAge: 60.0); // Keep last 60 seconds
 * buffer.push({time: t, value: x});
 * // Old samples automatically removed
 * ```
 *
 * **Use Cases:**
 * - Attitude history (quaternions, Euler angles)
 * - Sensor data (gyro, accelerometer, magnetometer)
 * - Rotor telemetry (RPM, thrust, power, temperature)
 * - State estimates (position, velocity, covariance)
 *
 * @tparam T Sample data type (e.g., AttitudeSample, RotorSample)
 */
template<typename T>
struct RingBuffer {
    std::deque<T> samples;                  ///< Time-ordered samples (oldest at front)
    double window_seconds{15.0};            ///< Time window to retain (seconds)
    double sample_interval{0.05};           ///< Desired sampling period (seconds, 20 Hz default)
    double last_sample_time{-std::numeric_limits<double>::infinity()}; ///< Timestamp of last captured sample

    /**
     * @brief Add sample if enough time has passed since last sample
     * @param sample New sample to add
     * @param current_time Current simulation time (seconds)
     * @return true if sample was added, false if skipped (too soon)
     */
    bool tryPush(const T& sample, double current_time) {
        // Check if enough time has passed since last sample
        if (current_time - last_sample_time < sample_interval) {
            return false;  // Skip sample (too soon)
        }

        samples.push_back(sample);
        last_sample_time = current_time;

        // Prune old samples (keep only samples within time window)
        prune(current_time);

        return true;
    }

    /**
     * @brief Force add sample regardless of timing
     * @param sample Sample to add
     * @param current_time Current simulation time
     */
    void push(const T& sample, double current_time) {
        samples.push_back(sample);
        last_sample_time = current_time;
        prune(current_time);
    }

    /**
     * @brief Remove samples older than time window
     * @param current_time Current simulation time (seconds)
     */
    void prune(double current_time) {
        // Remove samples older than window_seconds
        while (!samples.empty()) {
            // Assuming T has a 'timestamp' member
            double sample_age = current_time - samples.front().timestamp;
            if (sample_age > window_seconds) {
                samples.pop_front();  // Remove oldest sample
            } else {
                break;  // Rest of samples are within window
            }
        }
    }

    /**
     * @brief Clear all samples and reset timing
     */
    void clear() {
        samples.clear();
        last_sample_time = -std::numeric_limits<double>::infinity();
    }

    /**
     * @brief Get number of samples currently stored
     */
    size_t size() const {
        return samples.size();
    }

    /**
     * @brief Check if buffer is empty
     */
    bool empty() const {
        return samples.empty();
    }
};

#endif // CORE_RING_BUFFER_H
