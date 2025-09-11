#include "duplicate_finder.h"

/**
 * @brief Constructs a FileHasher with specified algorithm and block size
 * @param algorithm The hash algorithm to use (CRC32 or MD5)
 * @param block_size The size of blocks to read and hash
 */
FileHasher::FileHasher(HashAlgorithm algorithm, size_t block_size)
    : algorithm_(algorithm), block_size_(block_size) {}

/**
 * @brief Computes block hashes for a given file
 * @param file_path Path to the file to process
 * @return Vector of hash values for each block in the file
 * 
 * The file is read in blocks of specified size. If the file size is not
 * divisible by block size, the last block is padded with zeros.
 * Uses memory-mapped files for efficient reading.
 */
std::vector<uint32_t> FileHasher::compute_file_hashes(const fs::path& file_path) {
    std::vector<uint32_t> hashes;
    
    io::mapped_file_source file;
    try {
        file.open(file_path);
        if (!file.is_open()) {
            return hashes;
        }

        const char* data = file.data();
        size_t file_size = file.size();
        size_t pos = 0;

        // Process file in blocks of specified size
        while (pos < file_size) {
            size_t chunk_size = std::min(block_size_, file_size - pos);
            uint32_t hash = compute_hash(data + pos, chunk_size);
            hashes.push_back(hash);
            pos += chunk_size;
        }

        // Handle padding for last block if needed
        if (file_size > 0 && file_size % block_size_ != 0) {
            std::vector<char> padding(block_size_ - (file_size % block_size_), '\0');
            uint32_t hash = compute_hash(padding.data(), padding.size());
            hashes.push_back(hash);
        }

    } catch (const std::exception& e) {
        std::cerr << "Error processing file " << file_path << ": " << e.what() << std::endl;
    }

    return hashes;
}

/**
 * @brief Computes hash for a data block using selected algorithm
 * @param data Pointer to the data to hash
 * @param size Size of the data block in bytes
 * @return Hash value of the data block
 * @throws std::runtime_error if unsupported algorithm is specified
 */
uint32_t FileHasher::compute_hash(const char* data, size_t size) {
    switch (algorithm_) {
        case HashAlgorithm::CRC32: {
            boost::crc_32_type result;
            result.process_bytes(data, size);
            return result.checksum();
        }
        case HashAlgorithm::MD5: {
            // Для MD5 используем boost::hash
            boost::hash<std::string_view> hasher;
            return static_cast<uint32_t>(hasher(std::string_view(data, size)));
        }
        default:
            throw std::runtime_error("Unsupported hash algorithm");
    }
}

// Реализация DuplicateFinder

/**
 * @brief Constructs a DuplicateFinder with specified parameters
 * @param block_size Size of blocks for reading and comparison
 * @param algorithm Hash algorithm to use for block comparison
 * @param min_size Minimum file size to consider (default: 1 byte)
 * @param scan_level Recursion depth level (-1 for unlimited, 0 for current dir only)
 */
DuplicateFinder::DuplicateFinder(size_t block_size, FileHasher::HashAlgorithm algorithm,
                               uintmax_t min_size, int scan_level)
    : hasher_(algorithm, block_size), min_size_(min_size), scan_level_(scan_level) {}

/**
 * @brief Adds a directory to search for files
 * @param dir Path to the directory to include in search
 */
void DuplicateFinder::add_search_directory(const fs::path& dir) {
    search_dirs_.push_back(dir);
}

/**
 * @brief Adds a directory to exclude from search
 * @param dir Path to the directory to exclude from search
 */
void DuplicateFinder::add_exclude_directory(const fs::path& dir) {
    exclude_dirs_.insert(dir);
}

/**
 * @brief Adds a file mask pattern for filtering files
 * @param mask File mask pattern (case-insensitive, e.g., "*.txt", "*.jpg")
 */
void DuplicateFinder::add_file_mask(const std::string& mask) {
    file_masks_.push_back(boost::algorithm::to_lower_copy(mask));
}

/**
 * @brief Returns the found duplicate groups
 * @return Vector of duplicate file groups, each group contains paths to identical files
 */
std::vector<std::vector<fs::path>> DuplicateFinder::get_duplicate_groups() const {
    return duplicate_groups_;
}

/**
 * @brief Main method to find duplicate files
 * 
 * Executes the complete duplicate finding process:
 * 1. Collects files from specified directories
 * 2. Groups files by content using block hashing
 * 3. Prints results to standard output
 */
void DuplicateFinder::find_duplicates() {
    collect_files();
    group_duplicates();
    print_results();
}

/**
 * @brief Collects files from all search directories
 * 
 * Iterates through all added search directories and collects
 * files that match the specified criteria (size, masks, etc.)
 */
void DuplicateFinder::collect_files() {
    for (const auto& dir : search_dirs_) {
        if (!fs::exists(dir) || !fs::is_directory(dir)) {
            std::cerr << "Warning: Directory not found: " << dir << std::endl;
            continue;
        }

        int current_level = 0;
        collect_files_recursive(dir, current_level);
    }
}

/**
 * @brief Recursively collects files from a directory
 * @param dir Directory to scan
 * @param current_level Current recursion depth level
 * 
 * Respects scan level limits and exclude directories.
 * Processes subdirectories recursively if within level limits.
 */
void DuplicateFinder::collect_files_recursive(const fs::path& dir, int current_level) {
    if (scan_level_ >= 0 && current_level > scan_level_) {
        return;
    }

    if (exclude_dirs_.find(dir) != exclude_dirs_.end()) {
        return;
    }

    try {
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (fs::is_directory(entry)) {
                collect_files_recursive(entry.path(), current_level + 1);
            } else if (fs::is_regular_file(entry)) {
                process_file(entry.path());
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error accessing directory " << dir << ": " << e.what() << std::endl;
    }
}

/**
 * @brief Processes a single file and adds it to collection if it matches criteria
 * @param file_path Path to the file to process
 * 
 * Checks file against masks and minimum size requirements.
 * Only adds files that pass all filters to the processing list.
 */
void DuplicateFinder::process_file(const fs::path& file_path) {
    try {
        if (!matches_file_mask(file_path.filename().string())) {
            return;
        }

        uintmax_t file_size = fs::file_size(file_path);
        if (file_size < min_size_) {
            return;
        }

        FileInfo file_info;
        file_info.path = file_path;
        file_info.size = file_size;
        
        files_.push_back(std::move(file_info));
        
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error processing file " << file_path << ": " << e.what() << std::endl;
    }
}

/**
 * @brief Checks if filename matches any of the specified masks
 * @param filename Filename to check
 * @return true if filename matches any mask or no masks are specified
 * 
 * Performs case-insensitive comparison. Supports wildcard patterns:
 * - "*" matches all files
 * - ".*" matches all files with extensions
 * - "*.ext" matches files with specific extension
 */
bool DuplicateFinder::matches_file_mask(const std::string& filename) {
    if (file_masks_.empty()) {
        return true;
    }

    std::string lower_filename = boost::algorithm::to_lower_copy(filename);
    for (const auto& mask : file_masks_) {
        // Простое сравнение масок
        if (mask == "*") return true;
        if (mask == ".*") return true;
        
        if (mask.size() > 1 && mask[0] == '*' && mask[1] == '.') {
            std::string extension = mask.substr(1);
            if (boost::algorithm::ends_with(lower_filename, extension)) {
                return true;
            }
        }
        
        if (lower_filename == mask) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Groups collected files by content to find duplicates
 * 
 * Uses a two-phase approach:
 * 1. First groups files by size (quick elimination)
 * 2. Then compares files of same size block-by-block
 * 
 * Employs lazy hash computation - hashes are only computed
 * when files need to be compared.
 */
void DuplicateFinder::group_duplicates() {
    // Группируем по размеру файла
    std::unordered_map<uintmax_t, std::vector<size_t>> size_groups;
    for (size_t i = 0; i < files_.size(); ++i) {
        size_groups[files_[i].size].push_back(i);
    }

    // Для каждой группы по размеру сравниваем файлы
    for (auto& [size, indices] : size_groups) {
        if (indices.size() < 2) continue;

        std::vector<std::vector<size_t>> duplicate_groups;
        std::vector<bool> processed(indices.size(), false);

        for (size_t i = 0; i < indices.size(); ++i) {
            if (processed[i]) continue;

            std::vector<size_t> current_group = {indices[i]};
            processed[i] = true;

            for (size_t j = i + 1; j < indices.size(); ++j) {
                if (processed[j]) continue;

                if (are_files_identical(files_[indices[i]], files_[indices[j]])) {
                    current_group.push_back(indices[j]);
                    processed[j] = true;
                }
            }

            if (current_group.size() > 1) {
                duplicate_groups.push_back(std::move(current_group));
            }
        }

        // Сохраняем группы дубликатов
        for (const auto& group : duplicate_groups) {
            std::vector<fs::path> group_paths;
            for (size_t idx : group) {
                group_paths.push_back(files_[idx].path);
            }
            duplicate_groups_.push_back(std::move(group_paths));
        }
    }
}

/**
 * @brief Compares two files for identical content
 * @param file1 First file to compare
 * @param file2 Second file to compare
 * @return true if files have identical content, false otherwise
 * 
 * Uses lazy hash computation - hashes are only computed when needed.
 * Compares files block by block using the specified hash algorithm.
 */
bool DuplicateFinder::are_files_identical(FileInfo& file1, FileInfo& file2) {
    // Ленивое вычисление хэшей
    if (!file1.fully_processed) {
        file1.block_hashes = hasher_.compute_file_hashes(file1.path);
        file1.fully_processed = true;
    }

    if (!file2.fully_processed) {
        file2.block_hashes = hasher_.compute_file_hashes(file2.path);
        file2.fully_processed = true;
    }

    if (file1.block_hashes.size() != file2.block_hashes.size()) {
        return false;
    }

    for (size_t i = 0; i < file1.block_hashes.size(); ++i) {
        if (file1.block_hashes[i] != file2.block_hashes[i]) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Prints duplicate groups to standard output
 * 
 * Output format: each group of identical files is printed together,
 * with one file path per line. Groups are separated by empty lines.
 */
void DuplicateFinder::print_results() {
    for (const auto& group : duplicate_groups_) {
        for (const auto& path : group) {
            std::cout << path.string() << std::endl;
        }
        std::cout << std::endl;
    }
}
