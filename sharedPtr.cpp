#include <iostream>
#include <crtdbg.h>
#include <assert.h>

#define DEBUG

using namespace std;

template<typename T>
class sharedPtr
{
	T* pt;
	size_t* count;
	//由于多个sharedPtr实例会共享一个对象，当其中一个实例析构时，如果使用size_t而不是指针
	//那么其余的实例无法感知count的变动

public:
	sharedPtr() : pt(nullptr), count(new size_t(1)) {}

	explicit sharedPtr(T* t) : pt(t), count(new size_t(1)) {} //不允许用T* t隐式初始化

	sharedPtr(const sharedPtr& p) : pt(p.pt), count(p.count) { ++(*count); } //允许用拷贝构造函数隐式初始化

	~sharedPtr();

	sharedPtr& operator=(const sharedPtr& p);

	T& operator*();

	T* operator->();

	operator bool() const { return (pt != nullptr); }

	void make_unique();

	bool unique() const { return (*count) == 1; }

	size_t use_count() const { return (*count); }

	T* get() { return pt; }

};

/*相关操作的函数*/

template<typename T>
sharedPtr<T>& make_shared(T args)
{
	return sharedPtr<T>(&args);
}

template<typename T>
sharedPtr<T>& make_shard()
{
	return sharedPtr<T>();
}


/*辅助函数*/

//对于含有clone函数的对象，我们调用这个函数
template<typename T>
T* clone(const T* pt)
{
	return pt->clone();
}


//偏特化
//对于不含有clone函数的对象，我们调用这个函数
//以Vec<char>类型为例
template<>
int* clone(const int* pt)
{
	return new int(*pt);
}


/*成员函数的实现*/

template<typename T>
sharedPtr<T>::~sharedPtr()
{

	if (--(*count) == 0)
	{
#ifdef DEBUG
		cout << "done" << endl;
#endif // DEBUG

		delete pt;
		pt = nullptr;
		delete count;
		count = nullptr;
	}
}


template<typename T>
sharedPtr<T>& sharedPtr<T>::operator=(const sharedPtr& p)
{
	if (&p == this)
		return *this;

	if (--(*count) == 0)
	{
		delete pt;
		delete count;
#ifdef DEBUG
		cout << "done" << endl;
#endif // DEBUG
	}

	++(*p.count); //改变的是p.count指向的对象，而不是p的成员，所以可以用const

	pt = p.pt;
	count = p.count;

	return *this;
}


template<typename T>
T& sharedPtr<T>::operator*()
{
	if (pt == nullptr)
		throw runtime_error("unbund sharedPtr\n");
	else
		return *pt;
}


template<typename T>
T* sharedPtr<T>::operator->()
{
	if (pt == nullptr)
		throw runtime_error("unbund sharedPtr\n");
	else
		return pt;
}


template<typename T>
void sharedPtr<T>::make_unique()
{
	if ((*count) > 1)
	{
		--(*count);
		pt = pt ? clone(pt) : 0;
		count = new size_t(1);
	}
}


/* 测试代码 */
#ifdef DEBUG

int main(int argc, char* argv[])
{
	{
		//_CrtSetBreakAlloc(159);
		sharedPtr<int> p1(new int(0));
		assert(p1.unique() == true);
		assert(p1.use_count() == 1);
		p1 = p1;

		sharedPtr<int> p2(p1);
		assert(p1.unique() == false);
		assert(p2.use_count() == 2);

		sharedPtr<string> p3(new string("hello world"));
		string a = (*p3);
		size_t b = p3->size();
		assert(a == "hello world");
		assert(b == 11);

		sharedPtr<char> p4(new char('a'));
		char* c = p4.get();
		assert(*c == 'a');

		sharedPtr<int> p5(new int(5));
		p5 = p1;

		{
			sharedPtr<int> p6(new int(9));
		}
	}

	_CrtDumpMemoryLeaks();
	//由于该检测手法实在main函数推出前检测，而此时各个类实例还没出作用域
	//所以一直会存在内存泄漏，解决方法就是多加一层括号
}

#endif // DEBUG

