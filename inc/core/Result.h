#ifndef INCLUDED_RESULT_H
#define INCLUDED_RESULT_H

namespace lux
{

enum class EResult
{
	Aborted = 1,
	Succeeded = 0,
	Failed = -1,
	NotImplemented = -2,
};

inline bool Succeeded(EResult r)
{
	return (int)r >= 0;
}

inline bool Failed(EResult r)
{
	return !Succeeded(r);
}
}

#endif // !INCLUDED_RESULT_H