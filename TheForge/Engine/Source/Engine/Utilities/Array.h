#include <vector>
#include <algorithm>

template <typename T>
class Array
{
public:
    std::vector<T> data;

    // Check if the vector contains a specific value
    bool contains(const T& value) const
    {
        return std::find(data.begin(), data.end(), value) != data.end();
    }

    // Add a unique value to the vector
    void add_unique(const T& value)
    {
        if (!contains(value))
        {
            data.push_back(value);
        }
    }

    // Forwarding functions to behave like std::vector
    void push_back(const T& value) { data.push_back(value); }
    void pop_back() { data.pop_back(); }
    void clear() { data.clear(); }
    bool empty() const { return data.empty(); }
    size_t size() const { return data.size(); }
    size_t capacity() const { return data.capacity(); }
    void reserve(size_t new_cap) { data.reserve(new_cap); }
    void resize(size_t new_size) { data.resize(new_size); }
    void shrink_to_fit() { data.shrink_to_fit(); }
    
    void erase(const T& value) { data.erase(std::remove(data.begin(), data.end(), value), data.end()); }
    typename std::vector<T>::iterator erase(typename std::vector<T>::iterator pos) { return data.erase(pos); }
    typename std::vector<T>::iterator erase(typename std::vector<T>::iterator first, typename std::vector<T>::iterator last) { return data.erase(first, last); }
    
    typename std::vector<T>::iterator insert(typename std::vector<T>::iterator pos, const T& value) { return data.insert(pos, value); }
    void insert(typename std::vector<T>::iterator pos, size_t count, const T& value) { data.insert(pos, count, value); }

    T& front() { return data.front(); }
    const T& front() const { return data.front(); }
    T& back() { return data.back(); }
    const T& back() const { return data.back(); }

    typename std::vector<T>::iterator begin() { return data.begin(); }
    typename std::vector<T>::iterator end() { return data.end(); }
    typename std::vector<T>::const_iterator begin() const { return data.begin(); }
    typename std::vector<T>::const_iterator end() const { return data.end(); }

    T& operator[](size_t index) { return data[index]; }
    const T& operator[](size_t index) const { return data[index]; }
};
