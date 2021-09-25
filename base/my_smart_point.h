//
// Created by Administrator on 2021/9/25.
//

#ifndef NETWORK_MY_SMART_POINT_H
#define NETWORK_MY_SMART_POINT_H

#include <algorithm>

template<typename T>
class MySharedPtr
{
public:
    explicit MySharedPtr(T *ptr = nullptr) : ptr_(ptr)
    {}

    ~MySharedPtr()
    {
        delete ptr_;
    }

    T *get() const
    {
        return ptr_;
    }

    T &operator*() const
    {
        return *ptr_;
    }

    T *operator->() const
    {
        return ptr_;
    }

    explicit operator bool() const
    {
        return ptr_;
    }

    MySharedPtr(const MySharedPtr &) = delete;

    MySharedPtr &operator=(const MySharedPtr &) = delete;

private:
    T *ptr_;
};

template<typename T>
class AutoPtr
{
public:
    explicit AutoPtr(T *ptr = nullptr) noexcept: ptr_(ptr)
    {}

    ~AutoPtr() noexcept
    {
        delete ptr_;
    }

    // 返回值为T&，允许*ptr=10操作
    T &operator*() const noexcept
    {
        return *ptr_;
    }

    T *operator->() const noexcept
    {
        return ptr_;
    }

    operator bool() const noexcept
    {
        return ptr_;
    }

    T *get() const noexcept
    {
        return ptr_;
    }

    // 拷贝构造,被复制放释放原来指针的所有权,交给复制方
    AutoPtr(AutoPtr &other) noexcept
    {
        ptr_ = other.release();
    }

    // copy and swap
    AutoPtr &operator=(AutoPtr &rhs) noexcept
    {
        auto_ptr(rhs.release()).swap(*this);
        return *this;
    }

    // 原来的指针释放所有权
    T *release() noexcept
    {
        T *ptr = ptr_;
        ptr_ = nullptr;
        return ptr;
    }

    void swap(AutoPtr &rhs) noexcept
    {
        // 转移指针所有权
        std::swap(ptr_, rhs.ptr_);
    }

private:
    T *ptr_;
};

template<typename T>
void swap(AutoPtr<T> &lhs, AutoPtr<T> &rhs) noexcept
{
    lhs.swap(rhs);
}

#endif //NETWORK_MY_SMART_POINT_H
