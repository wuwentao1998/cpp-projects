// STL的list是采用双向循环链表实现的

#include <iostream>
#include <algorithm>
#include <assert.h>
#include <crtdbg.h>

using namespace std;

#define DEBUG

template <typename T>
struct Node
{
    T val;
    Node* prev;
    Node* next;

    Node(): prev(nullptr), next(nullptr) {}
    Node(const T& Val): val(Val), prev(nullptr), next(nullptr) {}
};


// 预先声明
template <typename T>
class List;


template <typename T>
class List_iterator
{
    friend class List<T>;
public:
    typedef Node<T>* Ptr;
    typedef List_iterator<T> Self;

    List_iterator(): cur(nullptr) {}
    List_iterator(Ptr x) : cur(x) {}
    List_iterator(const Self& s): cur(s.cur) {}

    // list迭代器不是随机访问迭代器，不支持加减操作
    T* operator->();
    T& operator*();
    Self& operator++();
    Self operator++(int);
    bool operator==(const Self& other) const { return cur == other.cur; }
    bool operator!=(const Self& other) const { return cur != other.cur; }

private:
    Ptr cur;
};


template <typename T>
class List
{
public:
    typedef Node<T> Node;
    typedef Node* Ptr;
    // 因为前面已经把Node<T>取别名为Node, 此时再取 Node<T>*,实际上类型为Node<T><T>*
    // 最好不要让别名和原类型的一部分一样，容易出错
    typedef List_iterator<T> iterator;
    typedef const List_iterator<T> const_iterator;

    List() { createHead(); }
    List(int num, T val);
    List(const List& other);
    List& operator=(const List& other);
    ~List();

    // 因为是循环链表，所以需要标记链表头，head节点便是这样的节点
    // 因此head节点本身不含有效val，head节点的下一个节点才是真正的头节点，而head节点本身来表示end节点也符合前闭后开原则
    iterator begin() {return iterator(head->next);}
    const_iterator begin() const {return iterator(head->next);}
    iterator end() {return iterator(head);}
    const_iterator end() const {return iterator(head);}
    bool empty() {return head->next == head;}

    void push_back(T val);
    void pop_back();
    iterator insert(iterator pos, T val);
    iterator erase(iterator pos);
    void clear();
private:
    void createHead();

    Ptr head;
};


/* List_iterator公有成员函数 */

template <typename T>
T& List_iterator<T>::operator*()
{
    return cur->val;
}

template <typename T>
T* List_iterator<T>::operator->()
{
    return &(cur->val);
}

template <typename T>
List_iterator<T>& List_iterator<T>::operator++()
{
    cur = cur->next;
    return *this; // 注意不是return cur
}

template <typename T>
List_iterator<T> List_iterator<T>::operator++(int)
{
    Self temp(*this);
    cur = cur->next;
    return temp;
}


/* List公有成员函数 */

template <typename T>
List<T>::List(int num, T val)
{
    createHead();
    for (int i = 0; i < num; i++)
        push_back(std::move(val));
}

template <typename T>
List<T>::List(const List& other)
{
    createHead();
    iterator it = other.begin();
    iterator endIt = other.end();
    while (it != endIt)
    {
        push_back(*it);
        it++;
    }
}

template <typename T>
List<T>& List<T>::operator=(const List& other)
{
    if (&other != this)
    {
        if (begin() != end())
            clear();

        iterator it = other.begin();
        iterator endIt = other.end();
        while (it != endIt)
        {
            push_back(*it);
            it++;
        }
    }

    return *this;
}

template <typename T>
List<T>::~List()
{
    if (begin() != end())
        clear();

    delete head;
    head = nullptr;
}

template <typename T>
void List<T>::push_back(T val)
{
    Ptr tail = head->prev;
    Ptr newNode = new Node(std::move(val));
    newNode->prev = tail;
    newNode->next = head;
    tail->next = newNode;
    head->prev = newNode;
}

template <typename T>
void List<T>::pop_back()
{
    if (empty())
        return;

    Ptr tail = head->prev;
    Ptr prevTail = tail->prev;
    prevTail->next = head;
    head->prev = prevTail;
    delete tail;
    tail = nullptr;
}

template <typename T>
typename List<T>::iterator List<T>::insert(iterator pos, T val)
{
    Ptr newNode = new Node(std::move(val));
    Ptr curNode = pos.cur;
    Ptr prevNode = curNode->prev;
    prevNode->next = newNode;
    newNode->prev = prevNode;
    newNode->next = curNode;
    curNode->prev = newNode;

    return iterator(newNode);
}

template <typename T>
typename List<T>::iterator List<T>::erase(iterator pos)
{
    if (empty())
        return iterator();

    Ptr& curNode = pos.cur;
    Ptr prevNode = curNode->prev;
    Ptr nextNode = curNode->next;
    prevNode->next = nextNode;
    nextNode->prev = prevNode;
    delete curNode;
    curNode = nullptr;

    return iterator(nextNode);
}

template <typename T>
void List<T>::clear()
{
    iterator it = begin();
    iterator endIt = end();
    while (it != endIt)
    {
        Ptr temp = it.cur;
        it++;
        delete temp;
        temp = nullptr;
    }

    head->next = head;
    head->prev = head;
}

/* List私有成员函数 */

template <typename T>
void List<T>::createHead()
{
    head = new Node;
    head->next = head;
    head->prev = head;
}



/* 测试代码 */

#ifdef DEBUG

int main(int argc, char* argv[])
{
    List<int> a;
    List<int> b(3, 5);
    List<int> c(b);
    List<int> d(4,6);
    d = c;

    a.push_back(9);
    a.push_back(0);
    a.pop_back();
    a.insert(a.begin(), 8);
    a.erase(++a.begin());
    a.clear();

}

#endif // DEBUG

