#ifndef INCLUDED_LXTREENODE_H
#define INCLUDED_LXTREENODE_H
#include "core/lxIterator.h"

namespace lux
{
namespace core
{

class TreeNode
{
public:
	typedef bool(*Removecallback)(TreeNode*);

	template <typename T>
	class _ConstIterator;

	template <typename T>
	class _Iterator : BaseIterator<BidirectionalIteratorTag, T>
	{
	public:
		_Iterator() : m_Current(nullptr)
		{
		}

		_Iterator<T>& operator++()
		{
			m_Current = m_Current->m_Sibling; return *this;
		}
		_Iterator<T>  operator++(int)
		{
			_Iterator<T> Temp = *this; m_Current = m_Current->m_Sibling; return Temp;
		}

		_Iterator<T>& operator+=(unsigned int num)
		{
			while(num-- && this->m_Current != 0) ++(*this);

			return *this;
		}

		_Iterator<T>  operator+ (unsigned int num) const
		{
			_Iterator temp = *this; return temp += num;
		}

		bool operator==(const _Iterator<T>&        other) const
		{
			return m_Current == other.m_Current;
		}
		bool operator!=(const _Iterator<T>&        other) const
		{
			return m_Current != other.m_Current;
		}
		bool operator==(const _ConstIterator<T>& other) const
		{
			return m_Current == other.m_Current;
		}
		bool operator!=(const _ConstIterator<T>& other) const
		{
			return m_Current != other.m_Current;
		}

		T* operator*()
		{
			return ((T*)m_Current);
		}
		T* operator->()
		{
			return (T*)m_Current;
		}

		T* Pointer()
		{
			return static_cast<T*>(m_Current);
		}

	private:
		explicit _Iterator(TreeNode* begin) : m_Current(begin)
		{
		}
		friend class TreeNode;
		friend class _ConstIterator<T>;

	private:
		TreeNode* m_Current;
	};

	template <typename T>
	class _ConstIterator : BaseIterator<BidirectionalIteratorTag, T>
	{
	public:
		_ConstIterator() : m_Current(nullptr)
		{
		}
		_ConstIterator(const _Iterator<T>& iter) : m_Current(iter.m_Current)
		{
		}

		_ConstIterator<T>& operator++()
		{
			m_Current = m_Current->m_Sibling; return *this;
		}
		_ConstIterator<T>  operator++(int)
		{
			_ConstIterator<T> Temp = *this; m_Current = m_Current->m_Sibling; return Temp;
		}

		_ConstIterator<T>& operator+=(unsigned int num)
		{
			while(num-- && this->m_Current != nullptr) ++(*this);

			return *this;
		}

		_ConstIterator<T>  operator+ (unsigned int num) const
		{
			_ConstIterator temp = *this; return temp += num;
		}

		bool operator==(const _Iterator<T>&         other) const
		{
			return m_Current == other.m_Current;
		}
		bool operator!=(const _Iterator<T>&         other) const
		{
			return m_Current != other.m_Current;
		}
		bool operator==(const _ConstIterator<T>& other) const
		{
			return m_Current == other.m_Current;
		}

		bool operator!=(const _ConstIterator<T>& other) const
		{
			return m_Current != other.m_Current;
		}

		const T& operator*()
		{
			return *m_Current;
		}
		const T* operator->()
		{
			return m_Current;
		}

		_ConstIterator<T>& operator=(const _Iterator<T>& iter)
		{
			m_Current = iter.m_Current; return *this;
		}
		T* Pointer() const
		{
			return static_cast<T*>(m_Current);
		}

	private:
		explicit _ConstIterator(TreeNode* begin) : m_Current(begin)
		{
		}

		friend class _Iterator<T>;
		friend class TreeNode;

	private:
		TreeNode* m_Current;
	};

	TreeNode() : m_Parent(nullptr), m_Sibling(nullptr), m_Child(nullptr)
	{
	}

	virtual ~TreeNode()
	{
	}

	inline void _AddChild(TreeNode* node)
	{
		if(!node)
			return;

		if(node->m_Parent)
			node->m_Parent->_RemoveChild(node);

		if(m_Child) {
			TreeNode* tmp = m_Child;
			m_Child = node;
			node->m_Sibling = tmp;
		} else {
			m_Child = node;
		}

		node->m_Parent = this;
	}

	inline bool _RemoveChild(TreeNode* node, Removecallback callback = nullptr)
	{
		if(!node)
			return false;

		TreeNode* last = nullptr;
		TreeNode* m_Current = m_Child;
		while(m_Current) {
			if(m_Current == node) {
				if(callback && callback(node) == false)
					return false;

				if(last)
					last->m_Sibling = m_Current->m_Sibling;
				else
					m_Child = m_Current->m_Sibling;

				node->m_Parent = nullptr;
				node->m_Sibling = nullptr;
				return true;
			}

			last = m_Current;
			m_Current = m_Current->m_Sibling;
		}

		return false;
	}

	inline void _RemoveFromParent()
	{
		if(m_Parent) {
			m_Parent->_RemoveChild(this);
			m_Sibling = nullptr;
		}
	}

	inline TreeNode* _GetParent() const
	{
		return m_Parent;
	}

	template <typename T>
	inline _Iterator<T> _GetChildrenFirst()
	{
		return _Iterator<T>(m_Child);
	}

	template <typename T>
	inline _ConstIterator<T> _GetChildrenFirst() const
	{
		return _ConstIterator<T>(m_Child);
	}

	template <typename T>
	inline _Iterator<T> _GetChildrenEnd()
	{
		return _Iterator<T>(nullptr);
	}

	template <typename T>
	inline _ConstIterator<T> _GetChildrenEnd() const
	{
		return _ConstIterator<T>(nullptr);
	}

private:
	TreeNode* m_Parent;
	TreeNode* m_Sibling;
	TreeNode* m_Child;
};

} // !namespace core
} // !namespace lux

#endif // !INCLUDED_TREENODE_H