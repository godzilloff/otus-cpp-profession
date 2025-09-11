#include <iostream>
#include "duplicate_finder.h"

/**
 * @brief Main entry point for the duplicate file finder utility
 * 
 * @param argc Count of command line arguments
 * @param argv Array of command line argument strings
 * @return int Exit status code (0 for success, non-zero for errors)
 * 
 * This function implements a command-line utility for finding duplicate files
 * by content comparison using block hashing. It supports various configuration
 * options including directory selection, file filtering, and hash algorithms.
 */
int main(int argc, char* argv[]) {
    std::cout << "argc " << argc << std::endl;
    
    /**
     * @brief Command line options description for the duplicate finder utility
     * 
     * Defines all available command line options with their descriptions
     * and default values where applicable.
     */
    po::options_description desc("Duplicate File Finder");
    desc.add_options()
        ("help,h", "Show help message")
        ("directories,d", po::value<std::vector<std::string>>()->multitoken(), 
         "Directories to search (multiple allowed)")
        ("exclude,e", po::value<std::vector<std::string>>()->multitoken(), 
         "Directories to exclude (multiple allowed)")
        ("level,l", po::value<int>()->default_value(-1), 
         "Scan level (-1 for unlimited, 0 for current directory only)")
        ("min-size,m", po::value<uintmax_t>()->default_value(1), 
         "Minimum file size in bytes")
        ("mask", po::value<std::vector<std::string>>()->multitoken(), 
         "File masks (e.g., '*.txt', '*.jpg')")
        ("block-size,b", po::value<size_t>()->default_value(4096), 
         "Block size for reading files")
        ("algorithm,a", po::value<std::string>()->default_value("crc32"), 
         "Hash algorithm (crc32, md5)");

    po::variables_map vm;
    try {
        // Parse command line arguments and store them in variables map
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } catch (const po::error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << desc << std::endl;
        return 1;
    }

    // Display help message if requested
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }

    // Validate that at least one search directory is specified
    if (!vm.count("directories")) {
        std::cerr << "Error: At least one directory must be specified" << std::endl;
        std::cerr << desc << std::endl;
        return 1;
    }

    /**
     * @brief Parse and validate hash algorithm selection
     * 
     * Converts the string algorithm name to the corresponding enum value
     * and validates that a supported algorithm was specified.
     */
    FileHasher::HashAlgorithm algorithm;
    std::string algo_str = vm["algorithm"].as<std::string>();
    if (algo_str == "crc32") {
        algorithm = FileHasher::HashAlgorithm::CRC32;
    } else if (algo_str == "md5") {
        algorithm = FileHasher::HashAlgorithm::MD5;
    } else {
        std::cerr << "Error: Unknown algorithm. Use 'crc32' or 'md5'" << std::endl;
        return 1;
    }

    /**
     * @brief Create DuplicateFinder instance with configured parameters
     * 
     * Initializes the duplicate finder with user-specified or default values
     * for block size, hash algorithm, minimum file size, and scan level.
     */
    DuplicateFinder finder(
        vm["block-size"].as<size_t>(),
        algorithm,
        vm["min-size"].as<uintmax_t>(),
        vm["level"].as<int>()
    );

    // Add search directories from command line arguments
    for (const auto& dir : vm["directories"].as<std::vector<std::string>>()) {
        finder.add_search_directory(fs::path(dir));
    }

    // Add exclude directories if specified
    if (vm.count("exclude")) {
        for (const auto& dir : vm["exclude"].as<std::vector<std::string>>()) {
            finder.add_exclude_directory(fs::path(dir));
        }
    }

    // Add file masks for filtering if specified
    if (vm.count("mask")) {
        for (const auto& mask : vm["mask"].as<std::vector<std::string>>()) {
            finder.add_file_mask(mask);
        }
    }

    /**
     * @brief Execute the duplicate finding process
     * 
     * This call performs the actual file scanning, hashing, and comparison
     * operations. Results are printed to standard output.
     */
    try {
        finder.find_duplicates();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
