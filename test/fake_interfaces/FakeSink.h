#ifndef STC_TASK_FAKESINK_H
#define STC_TASK_FAKESINK_H

#include <vector>
#include <variant>
#include <string>

// Before starting Sink, SymbolsMutex must be set via the setSymbolsMutex method,
// where SymbolsMutex is in byte converter class
class FakeSink {
public:
    using sink_element_t = std::variant<char, std::string>;
    using sink_t = std::vector<sink_element_t>;

    void push(sink_element_t symbol);

    const sink_t& getSymbols() const;

    void setSymbolsMutex(std::recursive_mutex* pSymbolsMutex);

    void clear();

private:
    sink_t m_symbols;
    std::recursive_mutex* m_pSymbolsMutex{nullptr}; // The original of this mutex is in byte converter class
};

void FakeSink::push(FakeSink::sink_element_t symbol) {
    assert(m_pSymbolsMutex != nullptr && "The pointer of m_pSymbolsMutex is null!");
    const std::lock_guard locker(*m_pSymbolsMutex);

    m_symbols.emplace_back(std::move(symbol));
}

const FakeSink::sink_t& FakeSink::getSymbols() const {
    assert(m_pSymbolsMutex != nullptr && "The pointer of m_pSymbolsMutex is null!");
    const std::lock_guard locker(*m_pSymbolsMutex);

    return m_symbols;
}

void FakeSink::setSymbolsMutex(std::recursive_mutex* pSymbolsMutex) {
    m_pSymbolsMutex = pSymbolsMutex;
}

void FakeSink::clear() {
    m_symbols.clear();
}

#endif //STC_TASK_FAKESINK_H