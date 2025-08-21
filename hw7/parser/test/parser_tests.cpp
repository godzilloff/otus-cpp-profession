#include <gtest/gtest.h>
#include <sstream>
#include <iostream>
#include "parse.h"

// Класс фикстуры для перенаправления cout
class ParseFixture : public ::testing::Test {
protected:
    // Сохраняем оригинальный буфер cout
    std::streambuf* original_cout_buf;
    // Поток для захвата вывода
    std::ostringstream cout_buffer;

    size_t bulk_size = 3;
    
    CommandProcessor* processor;
    std::shared_ptr<Observable> observable;

    void SetUp() override {
        // Перенаправляем cout в наш буфер перед каждым тестом
        original_cout_buf = std::cout.rdbuf();
        std::cout.rdbuf(cout_buffer.rdbuf());

        processor = new CommandProcessor(bulk_size);
        
        observable = std::make_shared<Observable>();
        observable->subscribe(std::make_shared<ConsoleObserver>());
        //observable->subscribe(std::make_shared<FileObserver>());
        
        processor->set_observable(observable);
    }

    void TearDown() override {
        // Восстанавливаем оригинальный буфер cout после каждого теста
        std::cout.rdbuf(original_cout_buf);
    }

    // Метод для получения захваченного вывода
    std::string GetCapturedOutput() {
        return cout_buffer.str();
    }

    // Метод для очистки буфера (если нужно тестировать несколько выводов)
    void ClearOutputBuffer() {
        cout_buffer.str("");
        cout_buffer.clear();
    }
};

TEST_F(ParseFixture, Simple) {
    processor->process_command("cmd1");
    processor->process_command("cmd2");
    processor->process_command("cmd3");
    EXPECT_EQ( GetCapturedOutput(), std::string("bulk: cmd1, cmd2, cmd3\n") );
}

TEST_F(ParseFixture, Nested) {
    processor->process_command("cmd1");
    processor->process_command("{");
    processor->process_command("cmd21");
    processor->process_command("cmd22");
    processor->process_command("}");
    EXPECT_EQ( GetCapturedOutput(), std::string("bulk: cmd1\nbulk: cmd21, cmd22\n") );
}
