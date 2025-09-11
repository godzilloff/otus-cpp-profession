#ifndef DUPLICATE_FINDER_H
#define DUPLICATE_FINDER_H

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/crc.hpp>
#include <boost/program_options.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/functional/hash.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/unordered_set.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <optional>
#include <fstream>
#include <functional>

namespace fs = boost::filesystem;
namespace po = boost::program_options;
namespace io = boost::iostreams;

/**
 * @brief Hash functor for boost::filesystem::path to use in unordered containers
 */
struct PathHash {
    /**
     * @brief Computes hash value for a filesystem path
     * @param path The path to compute hash for
     * @return Hash value of the path
     */
    std::size_t operator()(const fs::path& path) const {
        return fs::hash_value(path);
    }
};

/**
 * @brief Equality functor for boost::filesystem::path to use in unordered containers
 */
struct PathEqual {
    /**
     * @brief Compares two filesystem paths for equality
     * @param lhs Left-hand side path
     * @param rhs Right-hand side path
     * @return true if paths are equal, false otherwise
     */
    bool operator()(const fs::path& lhs, const fs::path& rhs) const {
        return lhs == rhs;
    }
};

/**
 * @brief Class responsible for computing file block hashes using various algorithms
 */
class FileHasher {
public:
    /**
     * @brief Supported hash algorithms
     */
    enum class HashAlgorithm {
        CRC32,  ///< CRC32 checksum algorithm
        MD5     ///< MD5 hash algorithm (using boost::hash approximation)
    };

    /**
     * @brief Constructs a FileHasher with specified algorithm and block size
     * @param algorithm The hash algorithm to use
     * @param block_size The size of blocks to read and hash
     */
    FileHasher(HashAlgorithm algorithm, size_t block_size);

    /**
     * @brief Computes block hashes for a given file
     * @param file_path Path to the file to process
     * @return Vector of hash values for each block in the file
     */
    std::vector<uint32_t> compute_file_hashes(const fs::path& file_path);

private:
    /**
     * @brief Computes hash for a data block using selected algorithm
     * @param data Pointer to the data to hash
     * @param size Size of the data block in bytes
     * @return Hash value of the data block
     */
    uint32_t compute_hash(const char* data, size_t size);
    
    HashAlgorithm algorithm_;  ///< Selected hash algorithm
    size_t block_size_;       ///< Size of blocks for reading and hashing
};

/**
 * @brief Main class for finding duplicate files by content comparison
 */
class DuplicateFinder {
public:
    /**
     * @brief Structure storing information about a processed file
     */
    struct FileInfo {
        fs::path path;                       ///< Full path to the file
        uintmax_t size;                      ///< File size in bytes
        std::vector<uint32_t> block_hashes;  ///< Computed block hashes
        bool fully_processed = false;        ///< Flag indicating if hashes were computed
    };

    /**
     * @brief Constructs a DuplicateFinder with specified parameters
     * @param block_size Size of blocks for reading and comparison
     * @param algorithm Hash algorithm to use for block comparison
     * @param min_size Minimum file size to consider (default: 1 byte)
     * @param scan_level Recursion depth level (-1 for unlimited, 0 for current dir only)
     */
    DuplicateFinder(size_t block_size, FileHasher::HashAlgorithm algorithm,
                   uintmax_t min_size = 1, int scan_level = -1);

    /**
     * @brief Adds a directory to search for files
     * @param dir Path to the directory to include in search
     */
    void add_search_directory(const fs::path& dir);

    /**
     * @brief Adds a directory to exclude from search
     * @param dir Path to the directory to exclude from search
     */
    void add_exclude_directory(const fs::path& dir);

    /**
     * @brief Adds a file mask pattern for filtering files
     * @param mask File mask pattern (case-insensitive)
     */
    void add_file_mask(const std::string& mask);

    /**
     * @brief Executes the duplicate file finding process
     */
    void find_duplicates();
    
    /**
     * @brief Returns the found duplicate groups
     * @return Vector of duplicate file groups
     */
    std::vector<std::vector<fs::path>> get_duplicate_groups() const;

private:
    /**
     * @brief Collects files from all search directories
     */
    void collect_files();

    /**
     * @brief Recursively collects files from a directory
     * @param dir Directory to scan
     * @param current_level Current recursion depth level
     */
    void collect_files_recursive(const fs::path& dir, int current_level);

    /**
     * @brief Processes a single file and adds it to collection if it matches criteria
     * @param file_path Path to the file to process
     */
    void process_file(const fs::path& file_path);

    /**
     * @brief Checks if filename matches any of the specified masks
     * @param filename Filename to check
     * @return true if filename matches any mask or no masks are specified
     */
    bool matches_file_mask(const std::string& filename);

    /**
     * @brief Groups collected files by content to find duplicates
     */
    void group_duplicates();

    /**
     * @brief Compares two files for identical content
     * @param file1 First file to compare
     * @param file2 Second file to compare
     * @return true if files have identical content, false otherwise
     */
    bool are_files_identical(FileInfo& file1, FileInfo& file2);

    /**
     * @brief Prints duplicate groups to standard output
     */
    void print_results();

    FileHasher hasher_;                                  ///< File hasher instance
    uintmax_t min_size_;                                 ///< Minimum file size to process
    int scan_level_;                                     ///< Maximum recursion depth
    std::vector<fs::path> search_dirs_;                  ///< Directories to search
    boost::unordered_set<fs::path, PathHash, PathEqual> exclude_dirs_;  ///< Directories to exclude
    std::vector<std::string> file_masks_;                ///< File mask patterns
    std::vector<FileInfo> files_;                        ///< Collected files information
    std::vector<std::vector<fs::path>> duplicate_groups_; ///< Found duplicate groups
};

#endif // DUPLICATE_FINDER_H
