#include "duplicate_finder.h"
#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>

class FileHasherTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Создаем временные файлы для тестирования
        test_dir_ = fs::temp_directory_path() / "duplicate_finder_test";
        fs::create_directories(test_dir_);
        
        // Создаем тестовые файлы
        create_test_file("test1.txt", "Hello World!");
        create_test_file("test2.txt", "Hello World!");
        create_test_file("test3.txt", "Different content");
        create_test_file("empty.txt", "");
    }
    
    void TearDown() override {
        // Удаляем временные файлы
        fs::remove_all(test_dir_);
    }
    
    void create_test_file(const std::string& filename, const std::string& content) {
        fs::path file_path = test_dir_ / filename;
        std::ofstream file(file_path.string()); // Преобразуем path в string
        file << content;
        file.close();
    }
    
    fs::path test_dir_;
};

TEST_F(FileHasherTest, CRC32_IdenticalFiles) {
    FileHasher hasher(FileHasher::HashAlgorithm::CRC32, 4);
    
    auto hashes1 = hasher.compute_file_hashes(test_dir_ / "test1.txt");
    auto hashes2 = hasher.compute_file_hashes(test_dir_ / "test2.txt");
    
    EXPECT_EQ(hashes1, hashes2);
}

TEST_F(FileHasherTest, CRC32_DifferentFiles) {
    FileHasher hasher(FileHasher::HashAlgorithm::CRC32, 4);
    
    auto hashes1 = hasher.compute_file_hashes(test_dir_ / "test1.txt");
    auto hashes2 = hasher.compute_file_hashes(test_dir_ / "test3.txt");
    
    EXPECT_NE(hashes1, hashes2);
}

TEST_F(FileHasherTest, MD5_IdenticalFiles) {
    FileHasher hasher(FileHasher::HashAlgorithm::MD5, 4);
    
    auto hashes1 = hasher.compute_file_hashes(test_dir_ / "test1.txt");
    auto hashes2 = hasher.compute_file_hashes(test_dir_ / "test2.txt");
    
    EXPECT_EQ(hashes1, hashes2);
}

TEST_F(FileHasherTest, EmptyFile) {
    FileHasher hasher(FileHasher::HashAlgorithm::CRC32, 4);
    
    auto hashes = hasher.compute_file_hashes(test_dir_ / "empty.txt");
    
    EXPECT_TRUE(hashes.empty());
}

TEST_F(FileHasherTest, DifferentBlockSizes) {
    FileHasher hasher1(FileHasher::HashAlgorithm::CRC32, 2);
    FileHasher hasher2(FileHasher::HashAlgorithm::CRC32, 8);
    
    auto hashes1 = hasher1.compute_file_hashes(test_dir_ / "test1.txt");
    auto hashes2 = hasher2.compute_file_hashes(test_dir_ / "test1.txt");
    
    EXPECT_NE(hashes1.size(), hashes2.size());
}
