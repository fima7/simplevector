#pragma once

#include "array_ptr.h"

#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <stdexcept>
#include <iostream>

class ReserveProxyObj
{
public:
    explicit ReserveProxyObj(size_t capacity)
        : capacity_(capacity)
    {}
public:
    size_t GetCapacity() const {
        return capacity_;
    }
private:
    size_t capacity_;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    // SimpleVector() noexcept = default;

    SimpleVector() noexcept
        : capacity_(0)
        , size_(0)
        , array_(nullptr)
    {}

    // ������ ������ �� size ���������, ������������������ ��������� �� ���������
    explicit SimpleVector(size_t size)
        : capacity_(size)
        , size_(capacity_)
        , array_(capacity_)
    {
        std::fill(begin(), end(), Type());
    }

    // ������ ������ �� size ���������, ������������������ ��������� value
    SimpleVector(size_t size, const Type& value)
        : capacity_(size)
        , size_(capacity_)
        , array_(capacity_)
    {
        std::fill(begin(), end(), value);
    }

    explicit SimpleVector(ReserveProxyObj reserve)
        : capacity_(reserve.GetCapacity())
        , size_(0)
        , array_(capacity_)
    {}

    // ������ ������ �� std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
        : SimpleVector(init.size())
    {
        std::copy(init.begin(), init.end(), begin());
    }
    
    SimpleVector(const SimpleVector& other) 
        : capacity_(0)
        , size_(0)
        , array_(nullptr)
    {
        // �������� ���� ������������ ��������������
        if (other.size_ > 0) {
            ArrayPtr<Type> tmp(other.size_);
            for (size_t i = 0; i < other.size_; ++i) {
                tmp[i] = other.array_[i];
            }

            size_ = other.size_;
            capacity_ = size_;
            array_.swap(tmp);
        }
    }

    SimpleVector(SimpleVector&& other) noexcept {
        // std::cout << "Move " << std::endl;
        swap(other);
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        // �������� ���� ������������ ��������������
        if(this != &rhs) {
            SimpleVector tmp(rhs);
            swap(tmp);
        }
        return *this;
    }

    // ��������� ������� � ����� �������
    // ��� �������� ����� ����������� ����� ����������� �������
    void PushBack(const Type& item) {
        // �������� ���� ��������������
        if (size_ == capacity_) {
            size_t new_capacity = size_ > 0 ? 2 * capacity_ : 1;
            ArrayPtr<Type> tmp(new_capacity);
            for (size_t i = 0; i < size_; ++i) {
                tmp[i] = array_[i];
            }
            tmp[size_] = item;
            array_.swap(tmp);
            ++size_;
            capacity_ = new_capacity;
        } else {
            array_[size_++] = item;
        }
    }

    void PushBack(Type&& item) {
        // �������� ���� ��������������
        if (size_ == capacity_) {
            size_t new_capacity = size_ > 0 ? 2 * capacity_ : 1;
            ArrayPtr<Type> tmp(new_capacity);
            for (size_t i = 0; i < size_; ++i) {
                tmp[i] = std::move(array_[i]);
            }
            tmp[size_] = std::move(item);
            array_.swap(tmp);
            ++size_;
            capacity_ = new_capacity;
        } else {
            array_[size_++] = std::move(item);
        }
    }


    // ��������� �������� value � ������� pos.
    // ���������� �������� �� ����������� ��������
    // ���� ����� �������� �������� ������ ��� �������� ���������,
    // ����������� ������� ������ ����������� �����, � ��� ������� ������������ 0 ����� ������ 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        size_t npos = pos - begin();

        if (size_ == capacity_) {
            size_t new_capacity = size_ > 0 ? 2 * capacity_ : 1;
            ArrayPtr<Type> tmp(new_capacity);
            for (size_t i = 0; i < npos; ++i) {
                tmp[i] = array_[i];
            }
            tmp[npos] = value;
            for (size_t i = npos + 1; i < size_ + 1; ++i) {
                tmp[i] = array_[i - 1];
            }
            array_.swap(tmp);
            ++size_;
            capacity_ = new_capacity;
        } else {
            for (size_t i = npos; i < size_; ++i) {
                array_[i + 1] = array_[i];
            }
            array_[npos] = value;
            ++size_;
        }

        return begin() + npos;
    }

    Iterator Insert(Iterator pos, Type&& value) {
        size_t npos = pos - begin();

        if (size_ == capacity_) {
            size_t new_capacity = size_ > 0 ? 2 * capacity_ : 1;
            ArrayPtr<Type> tmp(new_capacity);
            for (size_t i = 0; i < npos; ++i) {
                tmp[i] = std::move(array_[i]);
            }
            tmp[npos] = std::move(value);
            for (size_t i = npos + 1; i < size_ + 1; ++i) {
                tmp[i] = std::move(array_[i - 1]);
            }
            array_.swap(tmp);
            ++size_;
            capacity_ = new_capacity;
        } else {
            for (size_t i = npos; i < size_; ++i) {
                auto tmp = std::move(array_[i]);
                array_[i + 1] = std::move(tmp);
            }
            array_[npos] = std::move(value);
            ++size_;
        }

        return begin() + npos;
    }

    // "�������" ��������� ������� �������. ������ �� ������ ���� ������
    void PopBack() noexcept {
        // �������� ���� ��������������
        --size_;
    }

    // ������� ������� ������� � ��������� �������
    Iterator Erase(ConstIterator pos) {
        // �������� ���� ��������������
        size_t npos = pos - begin();

        for (size_t i = npos; i < size_ - 1; ++i) {
            array_[i] = std::move(array_[i + 1]);
        }

        --size_;
        
        return begin() + npos;
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> tmp(new_capacity);
            for (size_t i = 0; i < size_; ++i) {
                tmp[i] = array_[i];
            }
            array_.swap(tmp);
            capacity_ = new_capacity;
        }
    }

    // ���������� �������� � ������ ��������
    void swap(SimpleVector& other) noexcept {
        // �������� ���� ��������������
        if (this != &other) {
            array_.swap(other.array_);
            std::swap(capacity_, other.capacity_);
            std::swap(size_, other.size_);
        }
    }


    // ���������� ���������� ��������� � �������
    size_t GetSize() const noexcept {
        return size_;
    }

    // ���������� ����������� �������
    size_t GetCapacity() const noexcept {
        // �������� ���� ��������������
        return capacity_;
    }

    // ��������, ������ �� ������
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // ���������� ������ �� ������� � �������� index
    Type& operator[](size_t index) noexcept {
        // exception
        return array_[index];
    }

    // ���������� ����������� ������ �� ������� � �������� index
    const Type& operator[](size_t index) const noexcept {
        // exception
        return array_[index];
    }

    // ���������� ����������� ������ �� ������� � �������� index
    // ����������� ���������� std::out_of_range, ���� index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("index");
        }
        return array_[index];
    }

    // ���������� ����������� ������ �� ������� � �������� index
    // ����������� ���������� std::out_of_range, ���� index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("index");
        }
        return array_[index];
    }

    // �������� ������ �������, �� ������� ��� �����������
    void Clear() noexcept {
        // �������� ���� ��������������
        size_ = 0;
    }

    // �������� ������ �������.
    // ��� ���������� ������� ����� �������� �������� �������� �� ��������� ��� ���� Type
    void Resize(size_t new_size) {
        // �������� ���� ��������������
        if (new_size < size_) {
            size_ = new_size;
            return;
        }

        if (new_size <= capacity_) {
            for (size_t i = size_; i < new_size; ++i) {
                array_[i] = Type();
            }
            size_ = new_size;
        }
        else {
            ArrayPtr<Type> new_array(new_size);
            for (size_t i = 0; i < size_; ++i) {
                new_array[i] = array_[i];
            }
            for (size_t i = size_; i < new_size; ++i) {
                new_array[i] = Type();
            }
            array_.swap(new_array);
            size_ = new_size;
            capacity_ = size_;
        }
    }

    // ���������� �������� �� ������ �������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    Iterator begin() noexcept {
        return array_.Get();// const_cast<Iterator>(cbegin());
    }

    // ���������� �������� �� �������, ��������� �� ���������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    Iterator end() noexcept {
        return array_.Get() + size_;// const_cast<Iterator>(cend());
    }

    // ���������� ����������� �������� �� ������ �������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    ConstIterator begin() const noexcept {
        return array_.Get();// cbegin(); // array_.Get();
    }

    // ���������� �������� �� �������, ��������� �� ���������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    ConstIterator end() const noexcept {
        return array_.Get() + size_;// cend(); // array_.Get() + size_;
    }

    // ���������� ����������� �������� �� ������ �������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    ConstIterator cbegin() const noexcept {
        return array_.Get();
    }

    // ���������� �������� �� �������, ��������� �� ���������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    ConstIterator cend() const noexcept {
        return array_.Get() + size_;
    }
private:
    size_t capacity_;
    size_t size_;
    ArrayPtr<Type> array_;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return &lhs == &rhs || std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
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
    // ��������. �������� ���� ��������������
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    // ��������. �������� ���� ��������������
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}








