#ifndef STC_TASK_IOSTREAMBYTECONVERTER_H
#define STC_TASK_IOSTREAMBYTECONVERTER_H

#include "Logger.h"
#include <iostream>
#include <concepts>
#include <vector>
#include <variant>
#include <string>
#include <thread>
#include <bitset>
#include <mutex>

// Simple logging
#define LOG(LogLevel, Message) Logger::log(LogLevel, Message)
#define LOG_BINARY_BYTE(ByteValue) \
        LOG(Logger::level_e::Info, "Byte read -> "); \
        std::cout << "0b" << std::bitset<8>{ByteValue} << std::endl

template <class Source, class Sink>
concept IsIOInterfaces = requires (Source source, Sink sink) {
    { source.pushByte(uint8_t{}) } -> std::same_as<void>;
    { source.extractByte() } -> std::same_as<uint8_t>;
    { source.isEmptyBuffer() } -> std::same_as<bool>;
    { source.setBufferMutex(std::add_pointer_t<std::recursive_mutex>{})} -> std::same_as<void>;
    typename Sink::sink_element_t;
    typename Sink::sink_t;
    { sink.push(typename Sink::sink_element_t{}) } -> std::same_as<void>;
    { sink.setSymbolsMutex(std::add_pointer_t<std::recursive_mutex>{})} -> std::same_as<void>;
} && std::same_as<typename Sink::sink_element_t, std::variant<char, std::string>> &&
     std::same_as<typename Sink::sink_t, std::vector<typename Sink::sink_element_t>>;

/// A class representing a converter that reads bytes from Source and converts them to string characters according to
///  the logic from the test task and writes them to Sink
///
/// @tparam Source - An interface that represents an input "stream" of bytes
/// @tparam Sink - An interface that represents the output "stream" of bytes
template <class Source, class Sink> requires IsIOInterfaces<Source, Sink>
class IOStreamByteConverter {
public:
    IOStreamByteConverter(Source& source, Sink& sink) noexcept;

    /// Starts byte reading and byte conversion with writing in a separate thread
    ///
    /// @note When called again, will restart the converter with thread
    void run();

    /// Stops the converter
    void stop();

    ~IOStreamByteConverter() { stop(); }

private:
    std::unique_ptr<std::thread> m_pWorkingThread;
    std::recursive_mutex m_mutex;
    bool m_isRunning{false};

    Source& m_source;
    Sink& m_sink;

    enum class byte_type_e : uint8_t {
        UnsignedNumber = 0b00,
        SignedNumber = 0b01,
        LatinLetter = 0b10
    };

    void workingThread();

    bool convertByte(Sink::sink_element_t& convertedByte, uint8_t byte);
    void convertUnsignedValue(typename Sink::sink_element_t& convertedByte, uint16_t data);
    void convertSignedValue(typename Sink::sink_element_t& convertedByte, int16_t data);
    bool convertLatinLetter(typename Sink::sink_element_t& convertedByte, uint16_t data);
};

template<class Source, class Sink> requires IsIOInterfaces<Source, Sink>
IOStreamByteConverter<Source, Sink>::IOStreamByteConverter(Source& source, Sink& sink) noexcept
    : m_source{source}, m_sink{sink}
{
    m_source.setBufferMutex(&m_mutex);
    m_sink.setSymbolsMutex(&m_mutex);
}

template<class Source, class Sink> requires IsIOInterfaces<Source, Sink>
void IOStreamByteConverter<Source, Sink>::run() {
    if (m_isRunning) {
        LOG(Logger::level_e::Warn, "The converter is already running, it will be restarted\n");
        stop();
    }

    m_isRunning = true;
    m_pWorkingThread = std::make_unique<std::thread>(&IOStreamByteConverter::workingThread, this);

    LOG(Logger::level_e::Info, "The converter is running\n");
}

template<class Source, class Sink> requires IsIOInterfaces<Source, Sink>
void IOStreamByteConverter<Source, Sink>::stop() {
    m_isRunning = false;

    if (m_pWorkingThread) {
        m_pWorkingThread->join();
        m_pWorkingThread.reset();
        LOG(Logger::level_e::Info, "Converter stopped\n");
    }
}

template<class Source, class Sink> requires IsIOInterfaces<Source, Sink>
void IOStreamByteConverter<Source, Sink>::workingThread() {
    static constexpr int64_t OPTIMIZATION_SLEEP_TIME_MS = 15;

    while (m_isRunning) {
        // It is assumed that only one consumer is working with the source at a time (like one TCP connection
        // and reading bytes from a socket, for example)

        if (!m_source.isEmptyBuffer()) {
            const std::lock_guard locker(m_mutex);
            typename Sink::sink_element_t convertedByte;

            while (!m_source.isEmptyBuffer()) {
                const auto byte = m_source.extractByte();
                LOG_BINARY_BYTE(byte);

                if (!convertByte(convertedByte, byte)) continue;
                m_sink.push(std::move(convertedByte));
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(OPTIMIZATION_SLEEP_TIME_MS)); // Optimization for idle loop
    }
}

template<class Source, class Sink> requires IsIOInterfaces<Source, Sink>
bool IOStreamByteConverter<Source, Sink>::convertByte(typename Sink::sink_element_t& convertedByte, uint8_t byte) {
    const std::bitset<2> type = byte & 0b00000011;
    const auto data = (byte & 0b11111100) >> 2;

    if (type == static_cast<uint8_t>(byte_type_e::UnsignedNumber)) {
        LOG(Logger::level_e::Info, "Got unsigned number\n");
        convertUnsignedValue(convertedByte, data);
    }
    else if (type == static_cast<uint8_t>(byte_type_e::SignedNumber)) {
        LOG(Logger::level_e::Info, "Got signed number\n");
        convertSignedValue(convertedByte, data);
    }
    else if (type == static_cast<uint8_t>(byte_type_e::LatinLetter)) {
        LOG(Logger::level_e::Info, "Got latin letter\n");
        return convertLatinLetter(convertedByte, data);
    }
    else {
        LOG(Logger::level_e::Warn, "Byte skipped due to unknown type!\n");
        return false;
    }

    return true;
}

template<class Source, class Sink> requires IsIOInterfaces<Source, Sink>
void IOStreamByteConverter<Source, Sink>::convertUnsignedValue(typename Sink::sink_element_t& convertedByte, uint16_t data) {
    if (data >= 0 && data <= 9) {
        convertedByte = static_cast<char>(data + '0');
    } else {
        convertedByte = std::to_string(data);
    }
}

template<class Source, class Sink> requires IsIOInterfaces<Source, Sink>
void IOStreamByteConverter<Source, Sink>::convertSignedValue(typename Sink::sink_element_t& convertedByte, int16_t data) {
    if (data >= 0 && data <= 9) {
        convertedByte = static_cast<char>(data + '0');
    } else {
        convertedByte = std::to_string(data);
    }
}

template<class Source, class Sink> requires IsIOInterfaces<Source, Sink>
bool IOStreamByteConverter<Source, Sink>::convertLatinLetter(typename Sink::sink_element_t& convertedByte, uint16_t data) {
    static constexpr uint16_t firstLatinLetterNumber = 0; // a = 0b00000010
    static constexpr uint16_t lastLatinLetterNumber = 25; // z = 0b01100110

    if (data >= firstLatinLetterNumber && data <= lastLatinLetterNumber) {
        char latinLetter = static_cast<char>(data);
        latinLetter += 'a';

        convertedByte = latinLetter;
        return true;
    } else {
        LOG(Logger::level_e::Warn, "Byte skipped due to unknown letter data!\n");
        return false;
    }
}

#endif //STC_TASK_IOSTREAMBYTECONVERTER_H
