//
// Created by Administrator on 2021/9/17.
//

#include <memory>
#include <iostream>

class A : public std::enable_shared_from_this<A>
{
public:
    A()
    {
        std::cout << "A constructor" << std::endl;
    }

    ~A()
    {
        std::cout << "A destructor" << std::endl;
    }

    std::shared_ptr<A> getSelf()
    {
        return shared_from_this();
    }
};

void smartPointTest()
{
    // shared_ptr 共享智能指针
    auto *sp1 = new std::shared_ptr<int>(new int(100));
    std::cout << *(*sp1) << std::endl;

    // unique_ptr 独占智能指针
    std::unique_ptr<int> sp2 = std::make_unique<int>(1);

    std::shared_ptr<A> sp3(new A());
    std::shared_ptr<A> sp4 = sp3->getSelf();

    std::cout << "use count: " << sp3.use_count() << std::endl;

    // weak_ptr 弱引用智能指针
    std::weak_ptr<A> weakSp1(sp3);
    std::weak_ptr<A> weakSp2 = sp3;
    std::weak_ptr<A> weakSp3 = weakSp2;
    weakSp3.reset();
}