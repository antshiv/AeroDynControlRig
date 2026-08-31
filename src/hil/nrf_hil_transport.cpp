#include "hil/nrf_hil_transport.h"

#include <cerrno>
#include <chrono>
#include <cstring>
#include <limits>

#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#include <unistd.h>

namespace {
std::uint64_t monotonicMilliseconds()
{
    using clock = std::chrono::steady_clock;
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            clock::now().time_since_epoch()).count());
}

std::uint64_t monotonicMicroseconds()
{
    using clock = std::chrono::steady_clock;
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(
            clock::now().time_since_epoch()).count());
}

int remainingMilliseconds(std::uint64_t deadline_ms)
{
    const std::uint64_t now = monotonicMilliseconds();
    if (now >= deadline_ms) {
        return 0;
    }
    const std::uint64_t remaining = deadline_ms - now;
    return remaining > static_cast<std::uint64_t>(std::numeric_limits<int>::max())
        ? std::numeric_limits<int>::max()
        : static_cast<int>(remaining);
}

bool waitForDescriptor(int descriptor, short events, std::uint64_t deadline_ms,
                       const char* operation, std::string& error)
{
    for (;;) {
        const int remaining = remainingMilliseconds(deadline_ms);
        if (remaining <= 0) {
            error = std::string("nRF HIL ") + operation + " timed out";
            return false;
        }
        pollfd wait{descriptor, events, 0};
        const int result = poll(&wait, 1, remaining);
        if (result < 0 && errno == EINTR) {
            continue;
        }
        if (result <= 0) {
            error = std::string("nRF HIL ") + operation + " timed out";
            return false;
        }
        if ((wait.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
            error = std::string("nRF HIL ") + operation + " endpoint failed";
            return false;
        }
        if ((wait.revents & events) != 0) {
            return true;
        }
    }
}
}

NrfHilTransport::~NrfHilTransport()
{
    closeDevice();
}

bool NrfHilTransport::openDevice(const std::string& path, std::string& error)
{
    closeDevice();
    if (path.empty()) {
        error = "ASR_FC_HIL_DEVICE is empty";
        return false;
    }
    descriptor_ = ::open(path.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (descriptor_ < 0) {
        error = "cannot open " + path + ": " + std::strerror(errno);
        return false;
    }

    termios config{};
    if (tcgetattr(descriptor_, &config) != 0) {
        error = "cannot read serial configuration: " +
                std::string(std::strerror(errno));
        closeDevice();
        return false;
    }
    cfmakeraw(&config);
    cfsetispeed(&config, B115200);
    cfsetospeed(&config, B115200);
    config.c_cflag |= CLOCAL | CREAD;
    config.c_cflag &= static_cast<tcflag_t>(~(CSTOPB | CRTSCTS | PARENB));
    config.c_cflag = static_cast<tcflag_t>(
        (config.c_cflag & ~CSIZE) | CS8);
    if (tcsetattr(descriptor_, TCSANOW, &config) != 0 ||
        tcflush(descriptor_, TCIOFLUSH) != 0) {
        error = "cannot configure serial endpoint: " +
                std::string(std::strerror(errno));
        closeDevice();
        return false;
    }

    device_ = path;
    sequence_ = 0;
    last_round_trip_us_ = 0;
    asr_fc_hil_parser_init(&parser_);
    error.clear();
    return true;
}

void NrfHilTransport::closeDevice()
{
    if (descriptor_ >= 0) {
        ::close(descriptor_);
    }
    descriptor_ = -1;
    device_.clear();
    sequence_ = 0;
    last_round_trip_us_ = 0;
    asr_fc_hil_parser_init(&parser_);
}

bool NrfHilTransport::writeAll(const std::uint8_t* data, std::size_t size,
                               std::uint64_t deadline_ms, std::string& error)
{
    std::size_t written = 0;
    while (written < size) {
        if (!waitForDescriptor(descriptor_, POLLOUT, deadline_ms, "write", error)) {
            return false;
        }
        const ssize_t count = ::write(descriptor_, data + written, size - written);
        if (count < 0) {
            if (errno == EAGAIN || errno == EINTR) {
                continue;
            }
            error = "nRF HIL write failed: " + std::string(std::strerror(errno));
            return false;
        }
        written += static_cast<std::size_t>(count);
    }
    return true;
}

bool NrfHilTransport::exchange(const asr_fc_hil_sensor_guidance_t& request,
                               asr_fc_hil_flight_output_t& response,
                               std::uint32_t timeout_ms, std::string& error)
{
    if (!available()) {
        error = "nRF HIL serial endpoint is not open";
        return false;
    }
    if (timeout_ms == 0u) {
        error = "nRF HIL exchange timeout must be non-zero";
        return false;
    }
    const std::uint64_t deadline_ms = monotonicMilliseconds() + timeout_ms;
    const std::uint64_t started_us = monotonicMicroseconds();
    const std::uint32_t expected_sequence = ++sequence_;
    std::uint8_t encoded[ASR_FC_HIL_MAX_FRAME_SIZE]{};
    std::size_t encoded_size = 0;
    if (asr_fc_hil_encode_sensor_guidance(
            expected_sequence, &request, encoded, sizeof(encoded),
            &encoded_size) != ASR_FC_HIL_OK ||
        !writeAll(encoded, encoded_size, deadline_ms, error)) {
        return false;
    }

    std::uint8_t input[ASR_FC_HIL_MAX_FRAME_SIZE]{};
    bool corrupt_response_seen = false;
    for (;;) {
        if (!waitForDescriptor(descriptor_, POLLIN, deadline_ms,
                               "response", error)) {
            asr_fc_hil_parser_init(&parser_);
            tcflush(descriptor_, TCIFLUSH);
            if (corrupt_response_seen) {
                error = "nRF HIL response was corrupt and no valid response arrived";
            }
            return false;
        }
        const ssize_t count = ::read(descriptor_, input, sizeof(input));
        if (count < 0) {
            if (errno == EAGAIN || errno == EINTR) {
                continue;
            }
            error = "nRF HIL read failed: " + std::string(std::strerror(errno));
            asr_fc_hil_parser_init(&parser_);
            return false;
        }
        if (count == 0) {
            error = "nRF HIL endpoint closed";
            return false;
        }
        for (ssize_t index = 0; index < count; ++index) {
            std::uint8_t frame[ASR_FC_HIL_MAX_FRAME_SIZE]{};
            std::size_t frame_size = 0;
            bool ready = false;
            if (asr_fc_hil_parser_push(&parser_, input[index], frame,
                                       sizeof(frame), &frame_size,
                                       &ready) != ASR_FC_HIL_OK) {
                // The parser resynchronizes itself. Keep scanning this response
                // window so one damaged or stale fragment cannot hide a valid
                // frame that follows it.
                corrupt_response_seen = true;
                continue;
            }
            if (!ready) {
                continue;
            }
            std::uint32_t sequence = 0;
            if (asr_fc_hil_decode_flight_output(
                    frame, frame_size, &sequence, &response) != ASR_FC_HIL_OK ||
                sequence != expected_sequence ||
                response.acknowledged_sequence != expected_sequence ||
                response.session_id != request.session_id) {
                error = "nRF HIL response identity mismatch";
                asr_fc_hil_parser_init(&parser_);
                tcflush(descriptor_, TCIFLUSH);
                return false;
            }
            error.clear();
            last_round_trip_us_ = monotonicMicroseconds() - started_us;
            return true;
        }
    }
}
