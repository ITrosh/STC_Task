#ifndef STC_TASK_FAKESOURCE_H
#define STC_TASK_FAKESOURCE_H

#include <cassert>
#include <cstdint>
#include <queue>
#include <mutex>

// Before starting Source it is necessary to set BufferMutex via the setBufferMutex method,
// where BufferMutex is in byte converter class
class FakeSource {
public:
    void pushByte(uint8_t byte);

    uint8_t extractByte();

    bool isEmptyBuffer() const;

    void setBufferMutex(std::recursive_mutex* pBufferMutex);

private:
    std::queue<uint8_t> m_buffer;
    std::recursive_mutex* m_pBufferMutex{nullptr}; // The original of this mutex is in byte converter class
};

void FakeSource::pushByte(uint8_t byte) {
    assert(m_pBufferMutex != nullptr && "The pointer of m_pBufferMutex is null!");
    const std::lock_guard locker(*m_pBufferMutex);

    m_buffer.emplace(byte);
}

uint8_t FakeSource::extractByte() {
    assert(m_pBufferMutex != nullptr && "The pointer of m_pBufferMutex is null!");
    const std::lock_guard locker(*m_pBufferMutex);

    const auto byte = m_buffer.front();
    m_buffer.pop();
    return byte;
}

bool FakeSource::isEmptyBuffer() const {
    assert(m_pBufferMutex != nullptr && "The pointer of m_pBufferMutex is null!");
    const std::lock_guard locker(*m_pBufferMutex);

    return m_buffer.empty();
}

void FakeSource::setBufferMutex(std::recursive_mutex* pBufferMutex) {
    m_pBufferMutex = pBufferMutex;
}

#endif //STC_TASK_FAKESOURCE_H
