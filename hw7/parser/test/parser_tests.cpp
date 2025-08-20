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
        observable->subscribe(std::make_shared<FileObserver>());
        
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

// const size_t CNT = 10;

// TEST(Parser, Empty) {
//     // Arrange
//     // Matrix<int,0> matrix_empty;

//     // Act (empty for this test)

//     // Assert
//     ASSERT_EQ(0, static_cast<size_t>(0));
//     ASSERT_TRUE(true);
// }

// struct MatrixFixture : public testing::Test {
//     const size_t cnt = CNT;
//     Matrix<int,0> matrix;

//     // Per-test-suite set-up.
//     static void SetUpTestSuite() {
//         // std::cout << "SetUpTestSuite" << std::endl;
//     }

//     // Per-test-suite tear-down.
//     static void TearDownTestSuite() {
//         // std::cout << "TearDownTestSuite" << std::endl;
//     }

//     // Per-test set-up
//     void SetUp() override {
//         std::cout << "SetUp" << std::endl;

//         // Create list with elementCount elements    // Заполнение главной диагонали
//         for (size_t i = 0; i < CNT; ++i) {
//             matrix[i][i] = i;
//         }

//         // Заполнение второстепенной диагонали
//         for (size_t i = 0; i < CNT; ++i) {
//             matrix[i][9 - i] = 9 - i;
//         }

//     }

//     // You can define per-test tear-down logic as usual.
//     void TearDown() override {
//         // std::cout << "TearDown" << std::endl;
//     }
// };


// TEST(Matrix, Empty) {
//     // Arrange
//     Matrix<int,0> matrix_empty;

//     // Act (empty for this test)

//     // Assert
//     ASSERT_EQ(matrix_empty.size(), static_cast<size_t>(0));
//     ASSERT_TRUE(matrix_empty.empty());
// }

// TEST(Matrix, PushBack) {
//     // Arrange
//     const size_t count = CNT;
//     Matrix<int, 0> matrix;

//     // Act
//     for (size_t i = 0; i < count; ++i)
//         matrix[i][i] = i;

//     // Заполнение второстепенной диагонали
//     for (size_t i = 0; i < count; ++i)
//         matrix[i][9 - i] = 9 - i;

//     // Assert
//     ASSERT_FALSE(matrix.empty());
//     ASSERT_EQ(matrix.size(), static_cast<size_t>(18));
// }


// TEST_F(MatrixFixture, Operator) {
//     // Arrange
//     // from fixture

//     // Act    
//     ((matrix[100][100] = 314) = 0) = 217;
//     std::cout << "matrix[100][100] = " << matrix[100][100] << std::endl;

//     // Assert
//     ASSERT_EQ(matrix.size(), static_cast<size_t>(19));
//     ASSERT_FALSE(matrix.empty());

//     ASSERT_EQ(matrix[100][100], static_cast<size_t>(217));
// }
