#ifndef INCLUDED_LX_RED_BLACK_H
#define INCLUDED_LX_RED_BLACK_H
#include "LuxBase.h"
#include "core/lxUtil.h"
#include "core/lxIterator.h"

namespace lux
{
namespace core
{
template <typename T, typename Compare = core::CompareType<T>>
class RedBlackTree
{
public:
	class Node
	{
		friend class RedBlackTree;
	public:
		Node(const T& v) :
			left(nullptr), right(nullptr), parent(nullptr),
			value(v), isRed(true)
		{
		}

		void SetValue(const T& v) { value = v; }

		void SetValue(T&& v) { value = std::move(v); }
		void SetLeft(Node* n)
		{
			left = n;
			if(n)
				n->SetParent(this);
		}

		void SetRight(Node* n)
		{
			right = n;
			if(n)
				n->SetParent(this);
		}

		void SetParent(Node* n)
		{
			parent = n;
		}

		void RemoveFromParent()
		{
			if(!parent)
				return;

			if(parent->right == this)
				parent->right = nullptr;
			else
				parent->left = nullptr;
		}

		const T& GetValue() const { return value; }
		T& GetValue() { return value; }
		Node* GetLeft() const { return left; }
		Node* GetRight() const { return right; }
		Node* GetParent() const { return parent; }

		bool IsRight() const
		{
			return parent != nullptr && parent->GetRight() == this;
		}
		bool IsLeft() const
		{
			return parent != nullptr && parent->GetLeft() == this;
		}
		bool IsRoot() const
		{
			return parent == nullptr;
		}

		Node* GetPred() const
		{
			if(!left)
				return nullptr;
			Node* n = left;
			while(n->GetRight())
				n = n->GetRight();

			return n;
		}

		bool IsLeaf() const { return left == nullptr && right == nullptr; }

		void SetRed() { isRed = true; }
		void SetBlack() { isRed = false; }
		void SetColor(bool b) { isRed = b; }

		bool IsRed() const { return isRed; }
		bool IsBlack() const { return !isRed; }
		bool GetColor() const { return isRed; }

	private:
		Node* left;
		Node* right;
		Node* parent;

		T value;
		bool isRed;
	};

	class Iterator : public core::BaseIterator<core::BidirectionalIteratorTag, T>
	{
		friend class ConstIterator;
	public:
		Iterator() :
			m_Root(nullptr),
			m_Cur(nullptr)
		{
		}

		Iterator(Node* root, Node* cur) :
			m_Root(root),
			m_Cur(cur)
		{
		}

		Iterator(Node* root, bool lowest) :
			m_Root(root)
		{
			m_Cur = lowest ? GetMin(m_Root) : GetMax(m_Root);
		}

		Iterator& operator++()
		{
			lxAssert(m_Cur);

			// Got to the next bigger element

			if(m_Cur->GetRight())
				m_Cur = GetMin(m_Cur->GetRight());
			else if(m_Cur->IsLeft())
				m_Cur = m_Cur->GetParent();
			else {
				while(m_Cur->IsRight())
					m_Cur = m_Cur->GetParent();
				m_Cur = m_Cur->GetParent();
			}

			return *this;
		}

		Iterator& operator--()
		{
			// Got to the next smaller element
			lxAssert(m_Cur);

			if(m_Cur->GetLeft())
				m_Cur = GetMax(m_Cur->GetLeft());
			else if(m_Cur->GetRight())
				m_Cur = m_Cur->GetParent();
			else {
				while(m_Cur->IsLeft())
					m_Cur = m_Cur->GetParent();
				m_Cur = m_Cur->GetParent();
			}

			return *this;
		}

		Iterator operator++(int)
		{
			Iterator tmp = *this;
			++*this;
			return tmp;
		}

		Iterator operator--(int)
		{
			Iterator tmp = *this;
			--*this;
			return tmp;
		}

		bool operator==(const Iterator& other) const
		{
			return m_Root == other.m_Root && m_Cur == other.m_Cur;
		}

		bool operator!=(const Iterator& other) const
		{
			return !(*this == other);
		}

		T* operator->()
		{
			return &m_Cur->GetValue();
		}

		T& operator*()
		{
			return m_Cur->GetValue();
		}

		Node* GetNode() const
		{
			return m_Cur;
		}
		Node* GetRoot() const
		{
			return m_Root;
		}
	private:
		Node* GetMin(Node* n)
		{
			while(n && n->GetLeft())
				n = n->GetLeft();
			return n;
		}

		Node* GetMax(Node* n)
		{
			while(n && n->GetRight())
				n = n->GetRight();
			return n;
		}
	private:
		Node* m_Root;
		Node* m_Cur;
	};

	class ConstIterator : public core::BaseIterator<core::BidirectionalIteratorTag, T>
	{
	public:
		ConstIterator() :
			m_Root(nullptr),
			m_Cur(nullptr)
		{
		}

		ConstIterator(const Node* root, const Node* cur) :
			m_Root(root),
			m_Cur(cur)
		{
		}

		ConstIterator(const Node* root, bool lowest) :
			m_Root(root)
		{
			m_Cur = lowest ? GetMin(m_Root) : GetMax(m_Root);
		}

		ConstIterator(const ConstIterator& other) :
			m_Root(other.m_Root),
			m_Cur(other.m_Cur)
		{
		}

		ConstIterator(const Iterator& other) :
			m_Root(other.m_Root),
			m_Cur(other.m_Cur)
		{
		}
		ConstIterator& operator++()
		{
			lxAssert(m_Cur);

			// Got to the next bigger element

			if(m_Cur->GetRight())
				m_Cur = GetMin(m_Cur->GetRight());
			else if(m_Cur->IsLeft())
				m_Cur = m_Cur->GetParent();
			else {
				while(m_Cur->IsRight())
					m_Cur = m_Cur->GetParent();
				m_Cur = m_Cur->GetParent();
			}

			return *this;
		}

		ConstIterator& operator--()
		{
			// Got to the next smaller element
			lxAssert(m_Cur);

			if(m_Cur->GetLeft())
				m_Cur = GetMax(m_Cur->GetLeft());
			else if(m_Cur->GetRight())
				m_Cur = m_Cur->GetParent();
			else {
				while(m_Cur->IsLeft())
					m_Cur = m_Cur->GetParent();
				m_Cur = m_Cur->GetParent();
			}

			return *this;
		}

		ConstIterator operator++(int)
		{
			ConstIterator tmp = *this;
			++*this;
			return tmp;
		}

		ConstIterator operator--(int)
		{
			Iterator tmp = *this;
			--*this;
			return tmp;
		}

		bool operator==(const Iterator& other) const
		{
			return m_Root == other.m_Root && m_Cur == other.m_Cur;
		}

		bool operator!=(const Iterator& other) const
		{
			return !(*this == other);
		}

		bool operator==(const ConstIterator& other) const
		{
			return m_Root == other.m_Root && m_Cur == other.m_Cur;
		}

		bool operator!=(const ConstIterator& other) const
		{
			return !(*this == other);
		}

		const T* operator->()
		{
			return &m_Cur->GetValue();
		}

		const T& operator*()
		{
			return m_Cur->GetValue();
		}

	private:
		const Node* GetMin(const Node* n)
		{
			while(n && n->GetLeft())
				n = n->GetLeft();
			return n;
		}

		const Node* GetMax(const Node* n)
		{
			while(n && n->GetRight())
				n = n->GetRight();
			return n;
		}
	private:
		const Node* m_Root;
		const Node* m_Cur;
	};

public:
	RedBlackTree() :
		m_Root(nullptr),
		m_Size(0)
	{
	}

	RedBlackTree(const RedBlackTree& other) :
		m_Root(nullptr),
		m_Size(0)
	{
		*this = other;
	}

	RedBlackTree(RedBlackTree&& old) :
		m_Root(old.m_Root),
		m_Size(old.m_Size)
	{
		old.m_Root = nullptr;
		old.m_Size = 0;
	}

	~RedBlackTree()
	{
		Clear();
	}

	RedBlackTree& operator=(const RedBlackTree& other)
	{
		Clear();
		if(other.m_Size == 0)
			return *this;

		CopyTree(m_Root, other.m_Root);
		m_Size = other.m_Size;

		return *this;
	}

	RedBlackTree& operator=(RedBlackTree&& old)
	{
		Clear();
		m_Root = old.m_Root;
		m_Size = old.m_Size;
		old.m_Root = nullptr;
		old.m_Size = 0;

		return *this;
	}

	bool operator==(const RedBlackTree& other) const
	{
		if(Size() != other.Size())
			return false;

		ConstIterator it = ConstIterator(m_Root, true);
		ConstIterator jt = ConstIterator(other.m_Root, true);

		ConstIterator end = ConstIterator(m_Root, nullptr);
		while(it != end) {
			if(!m_Compare.Equal(*it, *jt))
				return false;
			++it;
			++jt;
		}
		return true;
	}

	bool operator!=(const RedBlackTree& other) const
	{
		return !(*this == other);
	}

	bool Insert(const T& v, Node** node = nullptr)
	{
		if(!m_Root) {
			lxAssert(m_Size == 0);
			SetRoot(CreateEntry(v));
			m_Size = 1;
			if(node)
				*node = m_Root;
			return true;
		}

		Node* n = m_Root;
		while(true) {
			if(m_Compare.Equal(v, n->GetValue())) {
				n->SetValue(v);
				if(node)
					*node = n;
				return false;
			} else if(m_Compare.Smaller(v, n->GetValue())) {
				if(n->GetLeft() == nullptr) {
					Node* newNode = CreateEntry(v);
					n->SetLeft(newNode);
					AdjustInsertion(newNode);
					n = newNode;
					break;
				}
				n = n->GetLeft();
			} else {
				if(n->GetRight() == nullptr) {
					Node* newNode = CreateEntry(v);
					n->SetRight(newNode);
					AdjustInsertion(newNode);
					n = newNode;
					break;
				}
				n = n->GetRight();
			}
		}

		++m_Size;
		if(node)
			*node = n;
		return true;
	}

	void Clear()
	{
		if(m_Size == 0)
			return;

		Node* n = m_Root;

		// Go to the furthest down node
		while(n && (n->GetLeft() || n->GetRight())) {
			if(n->GetLeft())
				n = n->GetLeft();
			else
				n = n->GetRight();
		}

		// Iterator over the nodes, bottom first order, from left to right
		while(n) {
			Node* toDelete = n; // Remember the node to delete


			// If there is a right sibling, remove it first
			Node* sibling = n->IsLeft() ? n->GetParent()->GetRight() : nullptr;
			if(sibling) {
				// Go as far down it's branch as posible
				n = sibling;
				while(n && (n->GetLeft() || n->GetRight())) {
					if(n->GetLeft())
						n = n->GetLeft();
					else
						n = n->GetRight();
				}
			} else {
				// Handled all the childs of this parent, go up one layer
				n = n->GetParent();
			}

			// Delete the node
			toDelete->RemoveFromParent();

			LUX_FREE(toDelete);

			--m_Size;
		}

		lxAssert(m_Size == 0);
		m_Root = nullptr;
	}

	Node* Find(const T& v) const
	{
		Node* n = m_Root;

		// Simple binary search
		while(n) {
			if(m_Compare.Equal(v, n->GetValue()))
				return n;
			else if(m_Compare.Smaller(v, n->GetValue()))
				n = n->GetLeft();
			else
				n = n->GetRight();
		}

		return nullptr;
	}

	bool Erase(const T& v)
	{
		Node* n = Find(v);
		if(!n)
			return false;

		Erase(n);
		return true;
	}

	Node* Erase(Node* n)
	{
		lxAssert(n);

		// Get the next node in order and return it
		Iterator it(m_Root, n);
		++it;

		if(n->GetLeft() && n->GetRight()) {
			// n has two children, move predecessor data in
			Node* pred = n->GetPred();
			n->SetValue(std::move(pred->GetValue()));
			n = pred;
		}

		Node* pullUp = n->GetLeft() ? n->GetLeft() : n->GetRight();
		if(pullUp) {
			if(n == m_Root)
				SetRoot(pullUp);
			else if(n->IsLeft())
				n->GetParent()->SetLeft(pullUp);
			else
				n->GetParent()->SetRight(pullUp);

			if(n->IsBlack())
				AdjustRemoval(pullUp);
		} else if(n == m_Root) {
			SetRoot(nullptr);
		} else {
			if(n->IsBlack())
				AdjustRemoval(n);
			n->RemoveFromParent();
		}

		// n isn't pointed at by any one, so we can remove it
		--m_Size;

		delete n;

		return it.GetNode();
	}

	size_t Size() const
	{
		return m_Size;
	}

	bool IsEmpty() const
	{
		return (Size() == 0);
	}

	Node* GetRoot()
	{
		return m_Root;
	}

	const Node* GetRoot() const
	{
		return m_Root;
	}

	template <typename IterT>
	static void FromOrdered(IterT first, IterT end, RedBlackTree* out)
	{
		lxAssert(out);
		lxAssert(out->m_Size == 0);
		lxAssert(out->m_Root == 0);

		out->m_Size = core::IteratorDistance(first, end);
		out->m_Root = out->FromOrderedRec(&first, out->m_Size, false);
	}

	const Compare& GetCompare() const
	{
		return m_Compare;
	}

private:
	void CopyTree(Node*& dst, Node* src)
	{
		if(!src)
			return;
		dst = CreateEntry(src->GetValue());
		dst->SetColor(src->GetColor());

		CopyTree(dst->left, src->left);
		if(dst->left)
			dst->left->parent = dst;
		CopyTree(dst->right, src->right);
		if(dst->right)
			dst->right->parent = dst;
	}

	template <typename IterT>
	Node* FromOrderedRec(IterT* first, size_t count, bool color)
	{
		if(count == 0)
			return nullptr;

		Node* left = FromOrderedRec(first, count / 2, !color);
		Node* root = CreateEntry(**first);
		root->SetColor(color);
		root->SetLeft(left);

		auto it = *first;
		++it;
		*first = it;

		root->SetRight(FromOrderedRec(first, count - count / 2 - 1, !color));

		return root;
	}

	Node* CreateEntry(const T& v)
	{
		return LUX_NEW(Node)(v);
	}

	void AdjustInsertion(Node* n)
	{
		lxAssert(n);

		// Balance the tree
		while(!n->IsRoot() && n->GetParent()->IsRed()) {
			if(n->GetParent()->IsLeft()) {
				// node is a left child -> get right uncle
				Node* uncle = n->GetParent()->GetParent()->GetRight();
				if(IsRed(uncle)) {
					// Change colors
					n->GetParent()->SetBlack();
					uncle->SetBlack();
					n->GetParent()->GetParent()->SetRed();

					// Fix the upper part of the tree
					n = n->GetParent()->GetParent();
				} else {
					// The uncle is black
					if(n->IsRight()) {
						n = n->GetParent();
						RotateLeft(n);
					}

					n->GetParent()->SetBlack();
					n->GetParent()->GetParent()->SetRed();
					RotateRight(n->GetParent()->GetParent());
				}
			} else {
				// node is a right child -> get left uncle
				Node* uncle = n->GetParent()->GetParent()->GetLeft();
				if(IsRed(uncle)) {
					// Change colors
					n->GetParent()->SetBlack();
					uncle->SetBlack();
					n->GetParent()->GetParent()->SetRed();

					// Fix the upper part of the tree
					n = n->GetParent()->GetParent();
				} else {
					// uncle is black
					if(n->IsLeft()) {
						n = n->GetParent();
						RotateRight(n);
					}

					n->GetParent()->SetBlack();
					n->GetParent()->GetParent()->SetRed();
					RotateLeft(n->GetParent()->GetParent());
				}
			}
		}

		m_Root->SetBlack();
	}

	void AdjustRemoval(Node* n)
	{
		while(n != m_Root && n->IsBlack()) {
			if(n->IsLeft()) {
				Node* sibling = n->GetParent()->GetRight();
				if(IsRed(sibling)) {
					sibling->SetBlack();
					n->GetParent()->SetRed();
					RotateLeft(n->GetParent());
					sibling = n->GetParent()->GetRight();
				}
				if(IsBlack(LeftOf(sibling)) && IsBlack(RightOf(sibling))) {
					SetRed(sibling);
					n = n->GetParent();
				} else {
					if(IsBlack(RightOf(sibling))) {
						SetBlack(LeftOf(sibling));
						SetRed(sibling);
						RotateRight(sibling);
						sibling = n->GetParent()->GetRight();
					}
					if(sibling)
						sibling->SetColor(n->GetParent()->GetColor());
					n->GetParent()->SetBlack();
					SetBlack(RightOf(sibling));
					RotateLeft(n->GetParent());
					n = m_Root;
				}
			} else {
				Node* sibling = n->GetParent()->GetLeft();
				if(IsRed(sibling)) {
					sibling->SetBlack();
					n->GetParent()->SetRed();
					RotateRight(n->GetParent());
					sibling = n->GetParent()->GetLeft();
				}
				if(IsBlack(LeftOf(sibling)) && IsBlack(RightOf(sibling))) {
					sibling->SetRed();
					n = n->GetParent();
				} else {
					if(IsBlack(LeftOf(sibling))) {
						SetBlack(RightOf(sibling));
						SetRed(sibling);
						RotateLeft(sibling);
						sibling = n->GetParent()->GetLeft();
					}

					if(sibling)
						sibling->SetColor(n->GetParent()->GetColor());

					n->GetParent()->SetBlack();
					SetBlack(LeftOf(sibling));
					RotateRight(n->GetParent());
					n = m_Root;
				}
			}
		}

		n->SetBlack();
	}

	void RotateLeft(Node* n)
	{
		Node* right = n->GetRight();
		n->SetRight(right->GetLeft());

		if(n->IsLeft())
			n->GetParent()->SetLeft(right);
		else if(n->IsRight())
			n->GetParent()->SetRight(right);
		else
			SetRoot(right);

		right->SetLeft(n);
	}

	void RotateRight(Node* n)
	{
		Node* left = n->GetLeft();
		n->SetLeft(left->GetRight());

		if(n->IsRight())
			n->GetParent()->SetRight(left);
		else if(n->IsLeft())
			n->GetParent()->SetLeft(left);
		else
			SetRoot(left);

		left->SetRight(n);
	}

	void SetRoot(Node* n)
	{
		m_Root = n;
		if(m_Root) {
			m_Root->SetParent(nullptr);
			m_Root->SetBlack();
		}
	}

	static bool IsRed(Node* n)
	{
		return n && n->IsRed();
	}

	static bool IsBlack(Node* n)
	{
		return !n || n->IsBlack();
	}

	static void SetRed(Node* n)
	{
		if(n)
			n->SetRed();
	}

	static void SetBlack(Node* n)
	{
		if(n)
			n->SetBlack();
	}

	static Node* LeftOf(Node* n)
	{
		return n ? n->GetLeft() : nullptr;
	}

	static Node* RightOf(Node* n)
	{
		return n ? n->GetRight() : nullptr;
	}

private:
	Node* m_Root;
	size_t m_Size;
	Compare m_Compare;
};

} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_LX_RED_BLACK_H