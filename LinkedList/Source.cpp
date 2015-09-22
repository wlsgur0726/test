#include <stdio.h>
#include <cassert>

template <typename T>
class List
{
public:
	typedef T value_type;
	typedef size_t size_type;

private:
	struct Node
	{
		Node* m_prev = nullptr;
		Node* m_next = nullptr;

		Node() {}
		Node(Node* prev, Node* next)
			: m_prev(prev), m_next(next) {}

		virtual value_type* Data() { return nullptr; }
		virtual ~Node() {}
	};

	struct DataNode : public Node
	{
		value_type m_data;
		DataNode(const value_type& val) : m_data(val) {}
		virtual value_type* Data() override { return &m_data; }
	};

	Node m_base = Node(&m_base, &m_base);
	size_type m_count = 0;

public:
	class iterator
	{
		friend class List;
		Node* m_node = nullptr;
		iterator(Node* init) {
			assert(init != nullptr);
			m_node = init;
		}

	public:
		iterator() {}

		iterator& operator ++ () {
			m_node = m_node->m_next;
			return *this;
		}

		iterator& operator -- () {
			m_node = m_node->m_prev;
			return *this;
		}

		T& operator * () {
			return *m_node->Data();
		}

		bool operator == (const iterator& i) const {
			return m_node == i.m_node;
		}

		bool operator != (const iterator& i) const {
			return m_node != i.m_node;
		}
	};


	size_type size() const
	{
		return m_count;
	}

	iterator begin()
	{
		return iterator(m_base.m_next);
	}

	iterator end()
	{
		return iterator(&m_base);
	}

	iterator insert(iterator pos, const value_type& data)
	{
		Node* base = pos.m_node;
		assert(base != nullptr); // 초기화되지 않은 이터레이터
		Node* newNode = new DataNode(data);
		Node* next = base->m_next;

		base->m_next = newNode;
		newNode->m_prev = base;
		newNode->m_next = next;
		assert(next->m_prev == base);
		next->m_prev = newNode;

		++m_count;
		return iterator(newNode);
	}

	void push_front(const value_type& data)
	{
		insert(iterator(&m_base), data);
	}

	void push_back(const value_type& data)
	{
		insert(iterator(m_base.m_prev), data);
	}

	iterator erase(iterator i)
	{
		Node* cur = i.m_node;
		assert(cur != nullptr);
		Node* prev = i.m_node->m_prev;
		Node* next = i.m_node->m_next;

		if (cur == &m_base)
			return end();

		assert(prev != nullptr);
		assert(prev->m_next == cur);
		prev->m_next = next;

		assert(next != nullptr);
		assert(next->m_prev == cur);
		next->m_prev = prev;

		delete cur;
		--m_count;

		return iterator(next);
	}

	void clear()
	{
		for (auto i=begin(); i!=end(); i=erase(i));
		assert(size() == 0);
	}

	~List()
	{
		clear();
	}
};



struct Int
{
	int val = 0;
	Int(int n) : val(n) {
		printf("[%p] constructor %d\n", this, val);
	}
	~Int() {
		printf("[%p] destructor %d\n", this, val);
	}
	operator int() const {
		return val;
	}
};


int main()
{
	List<Int> lst;
	auto Print = [&]()
	{
		printf("Print :");
		for (auto& n : lst) {
			printf(" %d", n);
		}
		printf("\n");
	};

	for (int i=0; i<10; ++i) {
		if (i%2 == 0)
			lst.push_back(i);
		else
			lst.push_front(i);
	}
	Print();

	auto zero = lst.begin();
	for (; zero!=lst.end() && *zero!=0; ++zero);
	assert(zero != lst.end());

	for (int i=100; i<500; i+=100) {
		lst.insert(zero, i);
	}
	Print();
	return 0;
}
