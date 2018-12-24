#include <iostream>

using namespace std;

template<typename T>
class sharedPtr
{
    T* pt;
    size_t* count;
    //由于多个sharedPtr实例会共享一个对象，当其中一个实例析构时，如果使用size_t而不是指针
    //那么其余的实例无法感知count的变动

public:
    sharedPtr(): pt(nullptr), count(new size_t(1)) {}

    sharedPtr(T* t): pt(t), count(new size_t(1)) {}

    sharedPtr(const sharedPtr& p): pt(p.pt), count(p.count) {(*count)++;}

    ~sharedPtr();

    sharedPtr& operator=(const sharedPtr& p);

    T& operator*();

    T* operator->();

    operator bool() const {return (pt != nullptr);}

    void make_unique();
};

/*辅助全局函数*/

//对于含有clone函数的对象，我们调用这个函数
template<typename T>
T* clone(const T* pt)
{
    return pt->clone();
}


//偏特化
//对于不含有clone函数的对象，我们调用这个函数
template<>
Vec<char>* clone(const Vec<char>* vp)
{
    return new Vec<char>(vp);
}


/*成员函数的实现*/

template<typename T>
sharedPtr<T>::~sharedPtr()
{
    if ((*count)-- == 0)
    {
        delete pt;
        delete count;
    }
}


template<typename T>
sharedPtr<T>& sharedPtr<T>::operator=(const sharedPtr& p)
{
    if (&p == this)
        return *this;

    if ((*count)-- == 0)
    {
        delete pt;
        delete count;
    }

    (*p.count)++; //改变的是p.count指向的对象，而不是p的成员，所以可以用const

    pt = p.pt;
    count = p.count;

    return *this;
}


template<typename T>
T& sharedPtr<T>::operator*()
{
    if(pt == nullptr)
        throw runtime_error("unbund sharedPtr\n");
    else
        return *pt;
}


template<typename T>
T* sharedPtr<T>::operator->()
{
    if(pt == nullptr)
        throw runtime_error("unbund sharedPtr\n");
    else
        return pt;
}


template<typename T>
void sharedPtr<T>::make_unique()
{
    if ((*count) > 1)
    {
        (*count)--;
        pt = pt ? clone(pt) : 0;
        count = new size_t(1);
    }
}



