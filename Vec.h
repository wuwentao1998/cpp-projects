#include <memory>
#include <iostream>
#include <algorithm>
#include <assert.h>

using namespace std;

//#define DEBUG

template<typename T>
class Vec
{
public:
    typedef T* iterator;
    typedef const T* const_iterator;
    typedef size_t size_type;
    typedef T& ref;
    typedef const T& const_ref;

private:
    iterator base;
    iterator limit;
    iterator avail;

    allocator<T> alloc; //由于new在分配内存的同时还执行了多余的默认初始化操作，因此我们用allocator类代替

public:
    Vec() { create();}
    explicit Vec(size_type n, const_ref val = T()) { create(n, val); }
    Vec(const Vec& v) { create(v.begin(), v.end());}
    Vec& operator=(const Vec& v);
    ~Vec();

    bool empty() const { return base == avail;}
    iterator begin() { return base; }
    const_iterator begin() const { return base; }
    iterator end() { return avail; }
    const_iterator end() const { return avail; }
    size_type size() const { return avail - base; }
    size_type capacity() const { return limit - base; }
    const_ref front() const { return *base; }
    const_ref back() const { return *(avail- 1); }


    void push_back(const_ref val);
    void clear();
    iterator insert(iterator pos, const_ref val);
    template<typename In>
    iterator insert(iterator pos, In first, In last);
    iterator erase(iterator pos);
    iterator erase(iterator first, iterator last);
    ref at(size_type n);

    ref operator[](size_type n) {return base[n]; }
    const_ref operator[](size_type n) const { return base[n]; }
    bool operator==(const Vec& v);



private:
    void create(size_type n = 0, const_ref val = T());
    void create(const_iterator begin, const_iterator end);
    void del();
    void grow();
};



/* 公有成员函数的实现 */

template<typename T>
Vec<T>& Vec<T>::operator=(const Vec& v) //类的作用域运算符之后才不用显式声明<T>
{
    if( &v != this)
    {
        del(); //如果不加判断，那么当传入对象是自身的时候，在执行删除操作后，传入的v其实也被删除了
        create(v.begin(), v.end());
    }

    return *this;
}


template<typename T>
Vec<T>::~Vec()
{
    del();
}


template<typename T>
void Vec<T>::push_back(const_ref val)
{
    if (avail + 1 > limit)
        grow();

    alloc.construct(avail++, val); //avail指向最后一个元素的后一个地址
}


template<typename T>
void Vec<T>::clear()
{
    auto it = base;
    while(it != avail)
        alloc.destroy(it--);

    avail = base;
}


template<typename T>
typename Vec<T>::iterator Vec<T>::insert(iterator pos, const_ref val)
{
    if (pos == avail)
        throw "illegal input iterator";

    if (avail + 1 > limit)
        grow();

    if (pos + 1 == avail)
    {
        alloc.construct(avail++, val);
        return avail;
    }

    auto temp = back();
    alloc.construct(avail++, temp);

    for(auto i = avail - 3; i >= pos; --i)
        base[i+1] = base[i];

    base[pos] = val;
    return pos + 1;
}


template<typename T>
template<typename In>
typename Vec<T>::iterator Vec<T>::insert(iterator pos, In first, In last)
{
    if (pos == avail || last <= first)
        throw "illegal input iterator";

    size_type add = last - first;

    if (avail + add > limit)
        grow();

    if (pos + 1 == avail)
    {
        copy(first, last, pos);
        avail += add;
        return avail;
    }

    auto temp = back();
    avail += add;
    alloc.construct(avail, temp);

    for(auto i = avail - add - 2; i >= pos + add; --i)
        base[i+1] = base[i];

    copy(first, last, pos);
    return pos + add;
}


template<typename T>
typename Vec<T>::iterator Vec<T>::erase(iterator pos)
{
    alloc.destory(pos);
    for(auto it = pos + 1 ; it < avail; ++it)
        base[it - 1] = base[it];

    --avail;

    return pos;
}


template<typename T>
typename Vec<T>::iterator Vec<T>::erase(iterator first, iterator last)
{
    if (!(first > base && last < avail))
        throw "illegal input iterator";

    for(auto it = first; it != last;++it)
        alloc.destory(it);

    size_type minus = last - first;

    for (auto it = last; it < avail; ++it)
        base[it - minus] = base[it];

    avail -= minus;

    return first;
}


template<typename T>
typename Vec<T>::ref Vec<T>::at(size_type n)
{
    if(base + n >= avail)
        throw "illegal position";

    return base[n];
}


template<typename T>
bool Vec<T>::operator==(const Vec& v)
{
    if (size() != v.size())
    return false;

    for(size_t i = 0; i < size(); ++i)
    {
        if(base[i] != v[i])
            return false;
    }

    return true;
}



/* 私有成员函数的实现 */

template<typename T>
void Vec<T>::create(size_type n , const_ref val)
{
    if (n == 0)
        base = limit = avail = nullptr;
    else
    {
        base = alloc.allocate(n); //因为alloc本身已经声明过类型了(T)，所以这里只需用n而不是n*sizeof(T)
        limit = avail = base + n;
        uninitialized_fill(base, limit, val);
    }
}


template<typename T>
void Vec<T>::create(const_iterator begin, const_iterator end)
{
    base = alloc.allocate(end - begin);
    limit = avail = uninitialized_copy(begin, end, base);
}


template<typename T>
void Vec<T>::del()
{
    if(base != nullptr)
    {
        iterator it = limit;
        while(it != base)
            alloc.destory(--it); //destory实际上就是运行了类的析构函数，如果类中存在指针，那么缺少这一步会造成内存泄漏

        alloc.deallocate(base, limit - base);
        base = limit = avail = nullptr;
    }
}


template<typename T>
void Vec<T>::grow()
{
    size_type new_size = (base == limit) ? 1 : 2*(limit - base);
    size_type new_base = alloc.allocate(new_size);
    size_type new_avail = uninitialized_copy(base, avail, new_base);

    del();

    base = new_base;
    avail = new_avail;
    limit = base + new_size;
}


/* 测试代码 */

#ifdef DEBUG

int main(int argc, char* argv[])
{
	{

	}

	_CrtDumpMemoryLeaks();
	//由于该检测手法实在main函数推出前检测，而此时各个类实例还没出作用域
	//所以一直会存在内存泄漏，解决方法就是多加一层括号
}

#endif // DEBUG



