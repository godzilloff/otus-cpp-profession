#include "duplicate_finder.h"
#include <gtest/gtest.h>
#include <fstream>
#include <sstream>

class DuplicateFinderTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir_ = fs::temp_directory_path() / "duplicate_finder_test";
        // Очищаем директорию перед созданием
        fs::remove_all(test_dir_);
        fs::create_directories(test_dir_);
        
        // Создаем структуру тестовых файлов
        create_test_files();
    }
    
    void TearDown() override {
        // Даем время файлам закрыться перед удалением
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        fs::remove_all(test_dir_);
    }
    
    void create_test_files() {
        // Создаем идентичные файлы
        create_test_file("dir1/file1.txt", "Same content 123");
        create_test_file("dir1/file2.txt", "Same content 123"); 
        create_test_file("dir2/file3.txt", "Same content 123");
        
        // Создаем разные файлы
        create_test_file("dir1/unique.txt", "Unique content");
        create_test_file("dir2/different.txt", "Different content");
        
        // Создаем маленький файл
        create_test_file("dir1/small.txt", "X");
        
        // Создаем файлы с разными масками
        create_test_file("dir1/image.jpg", "Image content 123");
        create_test_file("dir2/image.jpg", "Image content 123");
        create_test_file("dir1/document.pdf", "PDF content");
    }
    
    void create_test_file(const std::string& rel_path, const std::string& content) {
        fs::path full_path = test_dir_ / rel_path;
        fs::create_directories(full_path.parent_path());
        
        std::ofstream file(full_path.string(), std::ios::binary);
        file << content;
        file.close();
    }
    
    fs::path test_dir_;
};

TEST_F(DuplicateFinderTest, FindBasicDuplicates) {
    DuplicateFinder finder(4, FileHasher::HashAlgorithm::CRC32, 1, -1);
    finder.add_search_directory(test_dir_);
    
    finder.find_duplicates();
    auto duplicates = finder.get_duplicate_groups();
    
    // Должно найти 3 идентичных txt-файла и 2 идентичных jpg-файла
    EXPECT_EQ(duplicates.size(), 2);
    EXPECT_EQ(duplicates[0].size(), 3);
}

TEST_F(DuplicateFinderTest, FindBasicDuplicatesGuaranteed) {
    // Второй вариант создания файлов
    // Создаем гарантированно одинаковые файлы
    fs::path file1 = test_dir_ / "test1.txt";
    fs::path file2 = test_dir_ / "test2.txt";
    fs::path file3 = test_dir_ / "test3.txt";
    
    std::string identical_content = "Exactly the same content for all three files with some more text to make it longer";
    
    {
        std::ofstream f1(file1.string());
        std::ofstream f2(file2.string());
        std::ofstream f3(file3.string());
        f1 << identical_content;
        f2 << identical_content;
        f3 << identical_content;
    }
    
    DuplicateFinder finder(8, FileHasher::HashAlgorithm::CRC32, 1, 0);
    finder.add_search_directory(test_dir_);
    
    finder.find_duplicates();
    auto duplicates = finder.get_duplicate_groups();
    
    // Должно найти 3 идентичных файла
    EXPECT_EQ(duplicates.size(), 1);
    if (duplicates.size() == 1) {
        EXPECT_EQ(duplicates[0].size(), 3);
    }
}

TEST_F(DuplicateFinderTest, ExcludeDirectory) {
    DuplicateFinder finder(4, FileHasher::HashAlgorithm::CRC32, 1, -1);
    finder.add_search_directory(test_dir_);
    finder.add_exclude_directory(test_dir_ / "dir2");
    
    finder.find_duplicates();
    auto duplicates = finder.get_duplicate_groups();
    
    // Должен найти только 2 файла в dir1
    EXPECT_EQ(duplicates.size(), 1);
    EXPECT_EQ(duplicates[0].size(), 2);
}

TEST_F(DuplicateFinderTest, FileMaskFiltering) {
    DuplicateFinder finder(4, FileHasher::HashAlgorithm::CRC32, 1, -1);
    finder.add_search_directory(test_dir_);
    finder.add_file_mask("*.jpg");
    
    finder.find_duplicates();
    auto duplicates = finder.get_duplicate_groups();
    
    // Должен найти только jpg файлы
    EXPECT_EQ(duplicates.size(), 1);
    EXPECT_EQ(duplicates[0].size(), 2);
    
    for (const auto& path : duplicates[0]) {
        EXPECT_EQ(path.extension().string(), ".jpg");
    }
}

TEST_F(DuplicateFinderTest, MinSizeFilter) {
    DuplicateFinder finder(4, FileHasher::HashAlgorithm::CRC32, 10, -1);
    finder.add_search_directory(test_dir_);
    
    finder.find_duplicates();
    auto duplicates = finder.get_duplicate_groups();
    
    // Маленький файл должен быть отфильтрован
    bool found_small = false;
    for (const auto& group : duplicates) {
        for (const auto& path : group) {
            if (path.filename() == "small.txt") {
                found_small = true;
                break;
            }
        }
    }
    EXPECT_FALSE(found_small);
}

TEST_F(DuplicateFinderTest, ScanLevelLimit) {
    DuplicateFinder finder(4, FileHasher::HashAlgorithm::CRC32, 1, 0);
    finder.add_search_directory(test_dir_);
    
    finder.find_duplicates();
    auto duplicates = finder.get_duplicate_groups();
    
    // На уровне 0 не должно найти файлов в поддиректориях
    EXPECT_TRUE(duplicates.empty());
}
