/**
 * @file LinkedList.h
 * @brief This file contains the implementation of the LinkedList class.
 * @details This file contains the implementation of the LinkedList class
 * @author Higor Grigorio <higorgrigorio@gmail.com>
 * @version 1.0.0
 * @date 2023-07-10
 * 
*/
#ifndef ESP8266_LINKED_LIST
#define ESP8266_LINKED_LIST

namespace utility
{
    template <typename T>
    class LinkedListNode
    {
        T _value;

    public:
        LinkedListNode<T> *next;
        LinkedListNode(const T val) : _value(val), next(nullptr) {}
        ~LinkedListNode() {}
        const T &value() const { return _value; };
        T &value() { return _value; }
    };

    template <typename T, template <typename> class Item = LinkedListNode>
    class LinkedList
    {
    public:
        typedef Item<T> ItemType;
        typedef bool (*Predicate)(const T &);

    private:
        ItemType *_root;
        ItemType *_end;

        class Iterator
        {
            ItemType *_node;
            ItemType *_nextNode = nullptr;

        public:
            Iterator(ItemType *current = nullptr) : _node(current)
            {
                if (_node != nullptr)
                {
                    _nextNode = current->next;
                }
            }
            Iterator(const Iterator &i) : _node(i._node) {}
            Iterator &operator++()
            {
                _node = _nextNode;
                _nextNode = _node != nullptr ? _node->next : nullptr;
                return *this;
            }
            bool operator!=(const Iterator &i) const { return _node != i._node; }
            const T &operator*() const { return _node->value(); }
            const T *operator->() const { return &_node->value(); }
        };

    public:
        typedef const Iterator ConstIterator;
        ConstIterator constBegin() const { return ConstIterator(_root); }
        ConstIterator constEnd() const { return ConstIterator(nullptr); }

        Iterator begin() const { return Iterator(_root); }
        Iterator end() const { return Iterator(nullptr); }

        LinkedList()
            : _root(nullptr),
              _end(nullptr) {};
        ~LinkedList() {}

        void add(T &&t)
        {
            this->add(t);
        }

        void add(T &t)
        {
            auto value = new ItemType(t);
            if (!_root)
            {
                _root = _end = value;
            }
            else
            {
                _end = _end->next = value;
            }
        }

        void add(T *array, int size)
        {
            for (int i = 0; i < size; ++i)
            {
                add(array[i]);
            }
        }

        T &front() const
        {
            return _root->value();
        }

        T &back() const
        {
            return _end->value();
        }

        bool isEmpty() const
        {
            return _root == nullptr;
        }

        int length() const
        {
            int i = 0;
            auto it = _root;
            while (it)
            {
                i++;
                it = it->next;
            }
            return i;
        }

        int countIf(Predicate predicate) const
        {
            int i = 0;
            auto it = _root;
            while (it)
            {
                if (!predicate)
                {
                    i++;
                }
                else if (predicate(it->value()))
                {
                    i++;
                }
                it = it->next;
            }
            return i;
        }

        const T *at(int pos) const
        {
            int i = 0;
            auto it = _root;
            while (it)
            {
                if (i++ == pos)
                    return &(it->value());
                it = it->next;
            }
            return nullptr;
        }

        bool remove(const T &t)
        {
            auto it = _root;
            auto pit = _root;
            while (it)
            {
                if (it->value() == t)
                {
                    if (it == _root)
                    {
                        _root = _root->next;
                    }
                    else
                    {
                        pit->next = it->next;
                    }

                    if (it == _end)
                    {
                        _end = pit;
                    }

                    delete it;
                    return true;
                }
                pit = it;
                it = it->next;
            }
            return false;
        }

        bool removeFirst(Predicate predicate)
        {
            auto it = _root;
            auto pit = _root;
            while (it)
            {
                if (predicate(it->value()))
                {
                    if (it == _root)
                    {
                        _root = _root->next;
                    }
                    else
                    {
                        pit->next = it->next;
                    }

                    delete it;
                    return true;
                }
                pit = it;
                it = it->next;
            }
            return false;
        }

        int removeIf(Predicate predicate)
        {
            ItemType *it = _root;
            ItemType *pit = nullptr;
            auto count = 0;

            while (it)
            {
                if (predicate(it->value()))
                {
                    if (it == _root)
                    {
                        _root = _root->next;
                    }
                    else
                    {
                        if (it == _end)
                        { // end element
                            pit->next = nullptr;
                            _end = pit;
                        }
                        else
                        { // middle element
                            pit->next = it->next;
                        }
                    }
                    count++;
                    delete it;
                    it = pit->next;
                    continue;
                }
                pit = it;
                it = it->next;
            }

            return count;
        }

        int indexOf(const T &value)
        {
            auto it = _root;
            auto index = 0;

            while (it)
            {
                if (value == it->value())
                {
                    return index;
                }
                it = it->next;
                index++;
            }

            return -1;
        }

        LinkedList filter(Predicate predicate)
        {
            auto list = LinkedList<T>();
            auto it = _root;

            while (it)
            {
                T value = it->value();

                if (predicate(value))
                {
                    list.add(value);
                }
                it = it->next;
            }

            return list;
        }

        void free()
        {
            while (_root != nullptr)
            {
                auto it = _root;
                _root = _root->next;
                delete it;
            }
            _root = _end = nullptr;
        }
    };
}

template <typename T>
using LL = utility::LinkedList<T>;

#endif // ! ESP8266_LINKED_LIST
