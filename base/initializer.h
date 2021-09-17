//
// Created by Administrator on 2021/9/17.
//

#include <initializer_list>

template<typename T, int N>
class Arrays
{
private:
    T data[N];

public:
    explicit Arrays() = default;

    ~Arrays() = default;

    Arrays(std::initializer_list<T> list)
    {
        int index = 0;
        for (auto &temp: list)
        {
            data[index++] = temp;
        }
    }

    T operator[](int index)
    {
        return this->data[index];
    }

    T* begin()
    {
        return this->data;
    }

    T* end()
    {
        return this->data + N;
    }
};

void initializerTest()
{
    Arrays<int, 5> arr = {1, 2, 3, 4, 5};

    for (const auto &item: arr)
    {
        std::cout << item << std::endl;
    }
}