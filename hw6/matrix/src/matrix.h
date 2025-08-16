/**
 * @file sparse_matrix.h
 * @brief Implementation of an infinite sparse matrix using Proxy pattern
 */

#include <iostream>
#include <unordered_map>
#include <array>
#include <type_traits>

/**
 * @class Matrix
 * @brief N-dimensional sparse matrix with default values
 * @tparam T Type of matrix elements
 * @tparam DefaultValue Default value for unassigned elements
 * @tparam Dimensions Number of dimensions (default: 2)
 */
template<typename T, T DefaultValue, size_t Dimensions = 2>
class Matrix {
private:
    /**
     * @struct Key
     * @brief Represents a multi-dimensional index for matrix access
     */
    struct Key {
        std::array<size_t, Dimensions> indices; ///< Array of indices for each dimension
        
        /**
         * @brief Equality comparison operator
         * @param other Key to compare with
         * @return true if keys are equal, false otherwise
         */
        bool operator==(const Key& other) const {
            return indices == other.indices;
        }
    };

    /**
     * @struct KeyHasher
     * @brief Hash function for Key objects
     */
    struct KeyHasher {
        /**
         * @brief Computes hash value for a Key
         * @param key Key to hash
         * @return Computed hash value
         */
        size_t operator()(const Key& key) const {
            size_t hash = 0;
            for (auto index : key.indices) {
                hash ^= std::hash<size_t>{}(index) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            }
            return hash;
        }
    };

    std::unordered_map<Key, T, KeyHasher> data; ///< Storage for assigned elements
    T defaultValue; ///< Default value for unassigned elements

    /**
     * @class Proxy
     * @brief Intermediate proxy class for implementing multi-dimensional access
     * @tparam CurrentDimension Current dimension being accessed
     * @tparam TotalDimensions Total number of dimensions
     */
    template<size_t CurrentDimension, size_t TotalDimensions>
    class Proxy {
        Matrix& matrix; ///< Reference to the parent matrix
        std::array<size_t, TotalDimensions> indices; ///< Accumulated indices
        size_t current_dim; ///< Current dimension index

    public:
        /**
         * @brief Constructor
         * @param m Reference to parent matrix
         * @param idxs Array of indices
         * @param dim Current dimension index
         */
        Proxy(Matrix& m, std::array<size_t, TotalDimensions> idxs, size_t dim)
            : matrix(m), indices(idxs), current_dim(dim) {}

        /**
         * @brief Array access operator
         * @param index Index for current dimension
         * @return Next level proxy or terminal proxy
         */
        auto operator[](size_t index) {
            indices[current_dim] = index;
            if constexpr (CurrentDimension + 1 < TotalDimensions) {
                return Proxy<CurrentDimension + 1, TotalDimensions>(matrix, indices, current_dim + 1);
            } else {
                return Proxy<TotalDimensions, TotalDimensions>(matrix, indices, current_dim + 1);
            }
        }
    };

    /**
     * @class Proxy<TotalDimensions, TotalDimensions>
     * @brief Terminal proxy class for assignment operations
     * @tparam TotalDimensions Total number of dimensions
     */
    template<size_t TotalDimensions>
    class Proxy<TotalDimensions, TotalDimensions> {
        Matrix& matrix; ///< Reference to parent matrix
        std::array<size_t, TotalDimensions> indices; ///< Complete array of indices

    public:
        /**
         * @brief Constructor
         * @param m Reference to parent matrix
         * @param idxs Complete array of indices
         * @param Unused dimension parameter (ignored)
         */
        Proxy(Matrix& m, std::array<size_t, TotalDimensions> idxs, size_t)
            : matrix(m), indices(idxs) {}

        /**
         * @brief Assignment operator
         * @param value Value to assign
         * @return Reference to this proxy for chaining
         */
        Proxy& operator=(T value) {
            if (value == matrix.defaultValue) {
                matrix.data.erase({indices});
            } else {
                matrix.data[{indices}] = value;
            }
            return *this;
        }

        /**
         * @brief Conversion operator to matrix element type
         * @return Value at current position or default if unassigned
         */
        operator T() const {
            if (auto it = matrix.data.find({indices}); it != matrix.data.end()) {
                return it->second;
            }
            return matrix.defaultValue;
        }

        /**
         * @brief Assignment operator for size_t values
         * @param value Value to assign (automatically converted)
         * @return Reference to this proxy for chaining
         */
        Proxy& operator=(size_t value) {
            return *this = static_cast<T>(value);
        }
    };

public:
    /**
     * @brief Default constructor
     */
    Matrix() : defaultValue(DefaultValue) {}

    /**
     * @brief Array access operator for first dimension
     * @param index Index in first dimension
     * @return Proxy object for chaining
     */
    auto operator[](size_t index) {
        std::array<size_t, Dimensions> indices{};
        indices[0] = index;
        return Proxy<1, Dimensions>(*this, indices, 1);
    }

    /**
     * @brief Get number of assigned elements
     * @return Count of assigned elements
     */
    size_t size() const { return data.size(); }

    /**
     * @brief Check empty
     * @return true if (size == 0)
     */
    bool empty() const { return data.size() == 0; }

    /**
     * @struct Element
     * @brief Represents an assigned matrix element with its indices
     */
    struct Element {
        std::array<size_t, Dimensions> indices; ///< Position indices
        T value; ///< Element value
    };

    /**
     * @class Iterator
     * @brief Iterator over assigned matrix elements
     */
    class Iterator {
        typename std::unordered_map<Key, T, KeyHasher>::const_iterator it; ///< Internal map iterator
        
    public:
        /**
         * @brief Constructor
         * @param it Internal map iterator
         */
        Iterator(typename std::unordered_map<Key, T, KeyHasher>::const_iterator it) : it(it) {}

        /**
         * @brief Inequality operator
         * @param other Iterator to compare with
         * @return true if iterators are not equal
         */
        bool operator!=(const Iterator& other) const { return it != other.it; }
        
        /**
         * @brief Prefix increment operator
         */
        void operator++() { ++it; }
        
        /**
         * @brief Dereference operator
         * @return Element object with indices and value
         */
        Element operator*() const { return {it->first.indices, it->second}; }
    };

    /**
     * @brief Get iterator to first element
     * @return Begin iterator
     */
    Iterator begin() const { return Iterator(data.begin()); }
    
    /**
     * @brief Get iterator past last element
     * @return End iterator
     */
    Iterator end() const { return Iterator(data.end()); }

    /**
     * @brief Print matrix fragment (2D only)
     * @param x1 First dimension start index
     * @param y1 Second dimension start index
     * @param x2 First dimension end index
     * @param y2 Second dimension end index
     */
    void printFragment(size_t x1, size_t y1, size_t x2, size_t y2) {
        static_assert(Dimensions == 2, "printFragment is only available for 2D matrices");
        
        for (size_t i = x1; i <= x2; ++i) {
            for (size_t j = y1; j <= y2; ++j) {
                Key key{{i, j}};
                auto it = data.find(key);
                if (it != data.end()) {
                    std::cout << it->second;
                } else {
                    std::cout << defaultValue;
                }
                
                if (j != y2) {
                    std::cout << " ";
                }
            }
            std::cout << std::endl;
        }
    }
};
