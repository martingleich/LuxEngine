#ifndef INCLUDED_LUX_HELPER_TEMPLATES_H
#define INCLUDED_LUX_HELPER_TEMPLATES_H

namespace lux
{
namespace core
{

template <bool flag, class IsTrue, class IsFalse>
struct Choose;

template <class IsTrue, class IsFalse>
struct Choose<true, IsTrue, IsFalse>
{
	typedef IsTrue type;
};

template <class IsTrue, class IsFalse>
struct Choose<false, IsTrue, IsFalse>
{
	typedef IsFalse type;
};

class Uncopyable
{
protected:
	Uncopyable() = default;
	Uncopyable(const Uncopyable&) = delete;
	Uncopyable(Uncopyable&&) = delete;
	Uncopyable& operator=(const Uncopyable&) = delete;
	Uncopyable& operator=(Uncopyable&&) = delete;
};

} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_LUX_HELPER_TEMPLATES_H