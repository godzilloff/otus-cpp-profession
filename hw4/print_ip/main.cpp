#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <tuple>
#include <type_traits>
#include <cstddef>
#include <cstdint>

/**
 * @brief Checks if type T is a string (std::string, char[], const char*)
 * @tparam T Type to check
 */
template<typename T>
struct is_string : std::false_type {};

/// Specialization for std::string
template<> struct is_string<std::string> : std::true_type {};
/// Specialization for string literals (char[N])
template<size_t N> struct is_string<char[N]> : std::true_type {};
/// Specialization for const char*
template<> struct is_string<const char*> : std::true_type {};

/**
 * @brief Helper template variable for is_string
 * @tparam T Type to check
 */
template<typename T>
constexpr bool is_string_v = is_string<std::decay_t<T>>::value;

/**
 * @brief Checks if type T is integral and not bool
 * @tparam T Type to check
 */
template<typename T>
constexpr bool is_integral_not_bool_v = std::is_integral_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>;

/**
 * @brief Checks if type T is a vector or list
 * @tparam T Type to check
 */
template<typename T> struct is_vector_or_list : std::false_type {};
/// Specialization for std::vector
template<typename T, typename A> struct is_vector_or_list<std::vector<T, A>> : std::true_type {};
/// Specialization for std::list
template<typename T, typename A> struct is_vector_or_list<std::list<T, A>> : std::true_type {};

/**
 * @brief Helper template variable for is_vector_or_list
 * @tparam T Type to check
 */
template<typename T>
constexpr bool is_vector_or_list_v = is_vector_or_list<std::decay_t<T>>::value;

/**
 * @brief Checks if all types in Ts list are the same
 * @tparam Ts List of types to check
 */
template<typename... Ts>
struct are_all_same : std::false_type {};

/// Specialization for empty type list
template<>
struct are_all_same<> : std::true_type {};

/// Specialization for single type
template<typename T>
struct are_all_same<T> : std::true_type {};

/// Specialization for multiple types
template<typename T, typename... Rest>
struct are_all_same<T, Rest...> : std::conjunction<std::is_same<T, Rest>...> {};

/**
 * @brief Checks if tuple is homogeneous (all elements of same type)
 * @tparam T Tuple type to check
 */
template<typename T>
struct is_homogeneous_tuple : std::false_type {};

/// Specialization for std::tuple
template<typename... Ts>
struct is_homogeneous_tuple<std::tuple<Ts...>> : are_all_same<Ts...> {};

/**
 * @brief Helper template variable for is_homogeneous_tuple
 * @tparam T Tuple type to check
 */
template<typename T>
constexpr bool is_homogeneous_tuple_v = is_homogeneous_tuple<std::decay_t<T>>::value;

/**
 * @brief Prints tuple elements separated by dots
 * @tparam T Tuple type
 * @tparam Is Index sequence
 * @param t Tuple to print
 * @param Index sequence (index_sequence)
 */
template<typename T, size_t... Is>
void print_tuple(const T& t, std::index_sequence<Is...>) {
    ((std::cout << (Is == 0 ? "" : ".") << std::get<Is>(t)), ...);
    std::cout << std::endl;
}

/**
 * @brief Overload for integral types (except bool)
 * @details Prints value as sequence of bytes separated by dots
 * @tparam T Integral type (checked via is_integral_not_bool_v)
 * @param value Value to print
 */
template<typename T>
std::enable_if_t<is_integral_not_bool_v<T>>
print_ip(const T& value) {
    constexpr size_t size = sizeof(T);
    unsigned char bytes[size];
    T temp = value;
    for (int i = size - 1; i >= 0; --i) {
        bytes[i] = static_cast<unsigned char>(temp & 0xFF);
        temp >>= 8;
    }
    for (size_t i = 0; i < size; ++i) {
        if (i != 0) std::cout << ".";
        std::cout << static_cast<unsigned int>(bytes[i]);
    }
    std::cout << std::endl;
}

/**
 * @brief Overload for string types
 * @details Simply prints the string as-is
 * @tparam T String type (checked via is_string_v)
 * @param value String to print
 */
template<typename T>
std::enable_if_t<is_string_v<T>>
print_ip(const T& value) {
    std::cout << value << std::endl;
}

/**
 * @brief Overload for vectors and lists
 * @details Prints container elements separated by dots
 * @tparam T Container type (checked via is_vector_or_list_v)
 * @param container Container to print
 */
template<typename T>
std::enable_if_t<is_vector_or_list_v<T>>
print_ip(const T& container) {
    bool first = true;
    for (const auto& elem : container) {
        if (!first) std::cout << ".";
        std::cout << elem;
        first = false;
    }
    std::cout << std::endl;
}

/**
 * @brief Overload for homogeneous tuples
 * @details Prints tuple elements separated by dots
 * @tparam T Tuple type (checked via is_homogeneous_tuple_v)
 * @param t Tuple to print
 */
template<typename T>
std::enable_if_t<is_homogeneous_tuple_v<T>>
print_ip(const T& t) {
    constexpr size_t size = std::tuple_size_v<std::decay_t<T>>;
    print_tuple(t, std::make_index_sequence<size>{});
}

/**
 * @brief Main program function
 * @details Demonstrates print_ip function with different data types
 * @return 0 on success
 */
int main() {
    print_ip(int8_t{-1}); // 255
    print_ip(int16_t{0}); // 0.0
    print_ip(int32_t{2130706433}); // 127.0.0.1
    print_ip(int64_t{8875824491850138409}); // 123.45.67.89.101.112.131.41
    print_ip(std::string{"Hello, World!"}); // Hello, World!
    print_ip(std::vector<int>{100, 200, 300, 400}); // 100.200.300.400
    print_ip(std::list<short>{400, 300, 200, 100}); // 400.300.200.100
    print_ip(std::make_tuple(123, 456, 789, 0)); // 123.456.789.0

    // error
    // print_ip(std::make_tuple(1, 2.0));

    return 0;
}