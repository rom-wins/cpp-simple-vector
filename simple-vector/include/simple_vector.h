#pragma once
#include "array_ptr.h"
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <algorithm>


class ReserveProxyObj
{
public:
    ReserveProxyObj(size_t capacity_to_reserve) : capacity_to_reserve_(capacity_to_reserve){};
    size_t capacity_to_reserve_ = 0;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve);

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) {
        if (size == 0)
        {
            return;
        }
        ArrayPtr<Type> new_ptr(size);
        
        for (size_t i = 0; i < size; ++i)
        {
            new_ptr[i] = std::move(Type{});
        }
        
        ptr_.swap(new_ptr);
        size_ = size;
        capasity_ = size;
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    template<class Item>
    SimpleVector(size_t size, Item&& value) {
       
        if (size == 0)
        {
            return;
        }
        ArrayPtr<Type> new_ptr(size);
        
        for (size_t i = 0; i < size; ++i)
        {
            new_ptr[i] = std::move(value);
        }
        
        ptr_.swap(new_ptr);
        size_ = size;
        capasity_ = size;
 
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) {
        CreateBySizeAndIter(init.size(), init.begin(), init.end());
    }
    
    SimpleVector(const SimpleVector& other) {
        CreateBySizeAndIter(other.GetSize(), other.begin(), other.end());
    }
    
    SimpleVector(SimpleVector&& other) {
        CreateBySizeAndIter(other.GetSize(), std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()));
        other.size_ = 0;
    }
    
    SimpleVector(ReserveProxyObj rpobj)
    {
        Reserve(rpobj.capacity_to_reserve_);
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this == &rhs)
        {
            return *this;
        }

        SimpleVector<Type> copy_rhs = SimpleVector<Type>(rhs);
        this->swap(copy_rhs);
        return *this;
    }
    
    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    template<class Item>
    void PushBack(Item&& item) {
        if (size_ == capasity_)
        {
            size_t new_capasity = std::max(capasity_ * 2, size_ + 1);
            
            SimpleVector<Type> new_vector(new_capasity);
            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), new_vector.begin());
            
            new_vector[size_] = std::move(item);
            ptr_.swap(new_vector.ptr_);
            capasity_ = new_capasity;
        }
        else
        {
            ptr_[size_] = std::move(item);
        }
        ++size_;
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    template<class Item>
    Iterator Insert(ConstIterator cpos, Item&& value) {

        auto pos = const_cast<Type*>(cpos);
        size_t pos_index = std::distance(begin(), pos);

        if (size_ == capasity_)
        {
            size_t new_capasity = std::max(capasity_ * 2, size_ + 1); 
            SimpleVector<Type> new_vector(new_capasity);

            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(pos), new_vector.begin());
            std::copy(std::make_move_iterator(pos), std::make_move_iterator(end()), new_vector.begin() + pos_index + 1);

            new_vector[pos_index] = std::move(value); 
            ptr_.swap(new_vector.ptr_);
            capasity_ = new_capasity;
        }
        else
        {
            std::copy(std::make_move_iterator(pos), std::make_move_iterator(end()), begin() + pos_index); 
            ptr_[pos_index] = std::move(value);
        }
        ++size_;

        return ptr_.Get() + pos_index;
    } 
    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator cpos) {
        Iterator pos = const_cast<Type*>(cpos);
        std::copy(std::make_move_iterator(pos + 1), std::make_move_iterator(end()), pos);
        --size_;
        return pos;
    }
    
    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        --size_;
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        ptr_.swap(other.ptr_);
        std::swap(size_, other.size_);
        std::swap(capasity_, other.capasity_);
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capasity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0u;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return ptr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return ptr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_)
        {
            using namespace std::string_literals;
            throw std::out_of_range("Индекс за пределами диапазона"s);
        }
        return (*this)[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_)
        {
            using namespace std::string_literals;
            throw std::out_of_range("Индекс за пределами диапазона"s);
        }
        return (*this)[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        Resize(0u);
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size <= size_)
        {
            size_ = new_size;
            return;
        }
       
        size_t new_capasity = std::max(capasity_ * 2, new_size);
        
        SimpleVector<Type> new_vector(new_capasity);
        std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), new_vector.begin());
        
        ptr_.swap(new_vector.ptr_);
        size_ = new_size;
        capasity_ = new_capasity;
    }
    
    void Reserve(size_t new_capacity)
    {
        if (new_capacity > capasity_)
        {
            auto new_ptr = ArrayPtr<Type>(new_capacity);
            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), new_ptr.Get());
            ptr_.swap(new_ptr);
            std::swap(capasity_, new_capacity);
        }
    }
    
    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return ptr_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return ptr_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return ptr_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return ptr_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return begin();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return end();
    }
private:
    size_t size_ = 0;
    size_t capasity_ = 0;
    ArrayPtr<Type> ptr_;

    template<class Iter>
    void CreateBySizeAndIter(size_t size, Iter start, Iter end)
    {
        if (size == 0)
        {
            return;
        }
        ArrayPtr<Type> new_ptr(size);

        std::copy(start, end, new_ptr.Get());
        ptr_.swap(new_ptr);
        size_ = size;
        capasity_ = size;
    }
};


template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lhs.GetSize() == rhs.GetSize() && std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
} 
