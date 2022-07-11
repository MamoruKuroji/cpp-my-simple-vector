#pragma once

#include "array_ptr.h"

#include <cassert>
#include <initializer_list>
#include <exception>
#include <stdexcept>
#include <algorithm>
#include <utility>

struct ReserveProxyObj {
    ReserveProxyObj() = default;
    ReserveProxyObj(size_t capacity_to_reserve) : capacity(capacity_to_reserve)
    {
    }

    size_t capacity;
};

template <typename Type>
class SimpleVector {
public:

    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) : size_(size),
        capacity_(size),
        vector_(size) {
    }

    explicit SimpleVector(ReserveProxyObj size) : size_res_(size)
    {
        size_t capacity = size_res_.capacity;
        capacity_ = capacity;
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) : size_(size),
        capacity_(size),
        vector_(size) {
        for (size_t i = 0; i < size_; ++i) {
            vector_[i] = std::move(value);
        }
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) : size_(init.size()),
        capacity_(init.size()),
        vector_(init.size()) {
        std::copy( init.begin(), init.end(), &vector_[0]);
    }

    void Reserve(size_t new_capacity) {
        if (capacity_ < new_capacity) {
            ArrayPtr<Type> new_vector(new_capacity);
            for (size_t i = 0; i < size_; ++i) {
                new_vector[i] = std::move(vector_[i]);
            }
            Type check = {};
            for (size_t i = size_; i < new_capacity; ++i) {
                new_vector[i] = std::exchange(check, {});
            }
            vector_ = std::move(new_vector);

            capacity_ = new_capacity;
        }
    }

    SimpleVector(const SimpleVector& other) {
        if (this != &other) {
            size_t size_new = other.GetSize();
            size_t capacity_new = other.GetCapacity();
            ArrayPtr<Type> new_vector(capacity_new);
            for (size_t i = 0; i < size_new; ++i) {
                new_vector[i] = other[i];
            }
            vector_ = std::move(new_vector);
            size_ = size_new;
            capacity_ = capacity_new;
        }
    }

    // Move перемещение
    SimpleVector(SimpleVector&& other) {
        if (this != &other) {

            vector_ = std::move(other.vector_);
            size_ = std::exchange(other.size_, 0);
            capacity_ = std::exchange(other.capacity_, 0);
        }
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector buffer(rhs);
            this->swap(buffer);
        }
        return *this;
    }

    // Move перемещение
    SimpleVector& operator=(SimpleVector&& rhs) {
        if (this != &rhs) {
            SimpleVector buffer(std::move(rhs));
            this->swap(buffer);
        }
        return *this;
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return(size_ == 0);
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return vector_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return vector_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index < size_) {
            return vector_[index];
        }
        else {
            throw std::out_of_range("More than vector size");
        }
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index < size_) {
            return vector_[index];
        }
        else {
            throw std::out_of_range("More than vector size");
        }
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
        }
        else if (new_size <= capacity_) {
            Type check = {};
            for (size_t i = size_; i < new_size; ++i) {
                vector_[i] = std::exchange(check, {});
            }
            size_ = new_size;
        }
        else {
            
            size_t new_capacity = capacity_;
            if (new_capacity == 0) {
                new_capacity = 1;
            }
            while (new_capacity < new_size) {
                new_capacity *= 2;
            }
            ArrayPtr<Type> new_vector(new_capacity);
            for (size_t i = 0; i < size_; ++i) {
                new_vector[i] = std::move(vector_[i]);
            }
            Type check = {};
            for (size_t i = size_; i < new_size; ++i) {
                new_vector[i] = std::exchange(check, {});
            }
            vector_ = std::move(new_vector);

            capacity_ = new_capacity;
            size_ = new_size;

        }

    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return &vector_[0];
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return &vector_[size_];
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return &vector_[0];
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return &vector_[size_];
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return &vector_[0];
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return &vector_[size_];
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        size_t new_size = size_ + 1;
        if (new_size > capacity_) {
            size_t new_capacity = capacity_;
            if (new_capacity == 0) {
                new_capacity = 1;
            }
            while (new_capacity < new_size) {
                new_capacity *= 2;
            }
            ArrayPtr<Type> new_vector(new_capacity);
            for (size_t i = 0; i < size_; ++i) {
                new_vector[i] = vector_[i];
            }
            new_vector[size_] = item;
            vector_ = std::move(new_vector);
            capacity_ = new_capacity;
            size_ = new_size;
        }
        else {
            vector_[size_] = item;
            size_ = new_size;
        }
    }

    void PushBack(Type&& item) {
        size_t new_size = size_ + 1;
        if (new_size > capacity_) {
            size_t new_capacity = capacity_;
            if (new_capacity == 0) {
                new_capacity = 1;
            }
            while (new_capacity < new_size) {
                new_capacity *= 2;
            }
            ArrayPtr<Type> new_vector(new_capacity);
            for (size_t i = 0; i < size_; ++i) {
                new_vector[i] = std::move(vector_[i]);
            }
            new_vector[size_] = std::move(item);
            vector_ = std::move(new_vector);
            capacity_ = new_capacity;
            size_ = new_size;
        }
        else {
            vector_[size_] = std::move(item);
            size_ = new_size;
        }
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= begin() && pos <= end());
        size_t counter = static_cast<size_t>(pos - begin());

        size_t new_size = size_ + 1;
        size_t new_capacity = capacity_;
        if (new_capacity == 0) {
            new_capacity = 1;
        }
        while (new_capacity < new_size) {
                new_capacity *= 2;
        }
        ArrayPtr<Type> new_vector(new_capacity);
        for (size_t i = 0; i < counter; ++i) {
            new_vector[i] = vector_[i];
        }
        new_vector[counter] = value;
        for (size_t i = (counter); i < size_; ++i) {
            new_vector[i + 1] = vector_[i];
        }
        vector_.swap(new_vector);
        size_++;
        capacity_ = new_capacity;
        


        return &vector_[counter];
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());
        size_t counter = static_cast<size_t>(pos - begin());

        size_t new_size = size_ + 1;
        size_t new_capacity = capacity_;
        if (new_capacity == 0) {
            new_capacity = 1;
        }
        while (new_capacity < new_size) {
                new_capacity *= 2;
        }
        ArrayPtr<Type> new_vector(new_capacity);
        for (size_t i = 0; i < counter; ++i) {
            new_vector[i] = std::move(vector_[i]);
        }
        new_vector[counter] = std::move(value);;
        for (size_t i = (counter); i < size_; ++i) {
            new_vector[i + 1] = std::move(vector_[i]);
        }
        vector_ = std::move(new_vector);
        size_ = new_size;
        capacity_ = new_capacity;


        return &vector_[counter];
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if (size_ != 0) {
            --size_;
        }
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos <= end());
        size_t counter = static_cast<size_t>(pos - begin());
        if (pos != nullptr && capacity_ > 0 && size_ > 0) {
            for (size_t i = counter; i < size_; ++i) {
                size_t j = i + 1;
                vector_[i] = std::move(vector_[j]);
            }
            --size_;

        }
        return &vector_[counter];
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        
        swap(size_, other.size_);
        swap(capacity_, other.capacity_);

        vector_.swap(other.vector_);
    }

private:

    size_t size_ = 0;
    size_t capacity_ = 0;

    ArrayPtr<Type> vector_;

    ReserveProxyObj size_res_;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (rhs.GetSize() == lhs.GetSize()) {
        auto it_lhc = lhs.begin();
        auto it_rhs = rhs.begin();
        for (size_t i = 0; i < rhs.GetSize(); ++i) {
            if (*(it_lhc + i) != *(it_rhs + i)) {
                return false;
            };
        }
        return true;
    }
    else {
        return false;
    }
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (lhs == rhs) {
        return false;
    }
    else {
        return true;
    }
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());

}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs > rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(rhs.begin(), rhs.end(), lhs.begin(), lhs.end());

}

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}
