#include "IOStreamByteConverter.h"
#include "fake_interfaces/FakeSource.h"
#include "fake_interfaces/FakeSink.h"
#include <gtest/gtest.h>

using namespace std::chrono_literals;

FakeSource g_source;
FakeSink g_sink;
IOStreamByteConverter<FakeSource, FakeSink> g_converter{g_source, g_sink};

class Environment : public ::testing::Environment {
public:
    void SetUp() override {
        g_converter.run();
    }

    void TearDown() override {
        g_converter.stop();
    }
};

class EventListener : public ::testing::EmptyTestEventListener {
private:
    void OnTestStart(const ::testing::TestInfo&) override {
        g_sink.clear();
    }
};

// В качестве альтернативы std::this_thread::sleep_for в тестах можно сделать синхронизации source-sink и converter-sink:
// Необходимо, как вариант, вызывать getSymbols в отдельном потоке и иметь рекурсивный мьютекс, взаимодействие с которым описано ниже
// 1) source-sink: в методе pushByte интерфейса Source идёт захват рекурсивного мьютекса, защищающего "выходной буфер" sink'a
// 2) converter-sink: после обработки байтов из source и их записи в "выходной буфер" sink'a идёт разблокировка мьютекса из пункта 1
// Таким образом, вызов getSymbols во время наличия байтов в Source и их обработки в конвертере будет заблокирован

TEST(TypeTest, UnsignedNumber) {
    g_source.pushByte(0b00000000);

    std::this_thread::sleep_for(25ms); // Time that needs converter to process the buffer g_source
    ASSERT_EQ(std::get<char>(g_sink.getSymbols().at(0)), '0');
}

TEST(TypeTest, SignedNumber) {
    g_source.pushByte(0b00000001);

    std::this_thread::sleep_for(25ms); // Time that needs converter to process the buffer g_source
    ASSERT_EQ(std::get<char>(g_sink.getSymbols().at(0)), '0');
}

TEST(TypeTest, SmallLatinLetter) {
    g_source.pushByte(0b00000010);

    std::this_thread::sleep_for(25ms); // Time that needs converter to process the buffer g_source
    ASSERT_EQ(std::get<char>(g_sink.getSymbols().at(0)), 'a');
}

TEST(TypeTest, UnknownByteType) {
    // The test is considered passed if there are no errors in the time the converter is running
    g_source.pushByte(0b00000011);
}

TEST(MultipleBytes, ThreeRandomTypeBytes) {
    g_source.pushByte(0b11111100);
    g_source.pushByte(0b01100110);
    g_source.pushByte(0b01100101);

    std::this_thread::sleep_for(25ms); // Time that needs converter to process the buffer g_source
    ASSERT_EQ(std::get<std::string>(g_sink.getSymbols().at(0)), "63");
    ASSERT_EQ(std::get<char>(g_sink.getSymbols().at(1)), 'z');
    ASSERT_EQ(std::get<std::string>(g_sink.getSymbols().at(2)), "25");
}

TEST(MultipleBytes, ThreeTimesThreeBytes) {
    g_source.pushByte(0b11111100);
    g_source.pushByte(0b01100110);
    g_source.pushByte(0b01100101);

    std::this_thread::sleep_for(25ms); // Time that needs converter to process the buffer g_source
    ASSERT_EQ(std::get<std::string>(g_sink.getSymbols().at(0)), "63");
    ASSERT_EQ(std::get<char>(g_sink.getSymbols().at(1)), 'z');
    ASSERT_EQ(std::get<std::string>(g_sink.getSymbols().at(2)), "25");

    g_source.pushByte(0b00010010);
    g_source.pushByte(0b00100010);
    g_source.pushByte(0b01001110);

    std::this_thread::sleep_for(25ms); // Time that needs converter to process the buffer g_source
    ASSERT_EQ(std::get<char>(g_sink.getSymbols().at(3)), 'e');
    ASSERT_EQ(std::get<char>(g_sink.getSymbols().at(4)), 'i');
    ASSERT_EQ(std::get<char>(g_sink.getSymbols().at(5)), 't');

    g_source.pushByte(0b11100000);
    g_source.pushByte(0b10100101);
    g_source.pushByte(0b01101100);

    std::this_thread::sleep_for(25ms); // Time that needs converter to process the buffer g_source
    ASSERT_EQ(std::get<std::string>(g_sink.getSymbols().at(6)), "56");
    ASSERT_EQ(std::get<std::string>(g_sink.getSymbols().at(7)), "41");
    ASSERT_EQ(std::get<std::string>(g_sink.getSymbols().at(8)), "27");
}

TEST(LetterData, UnknownLetterData) {
    // The test is considered passed if there are no errors in the time the converter is running
    g_source.pushByte(0b11111110);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new Environment);
    ::testing::UnitTest::GetInstance()->listeners().Append(new EventListener);

    return RUN_ALL_TESTS();
}