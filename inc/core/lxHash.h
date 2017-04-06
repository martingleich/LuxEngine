#ifndef INCLUDED_LXHASH_H
#define INCLUDED_LXHASH_H
#include "core/lxString.h"
#include <unordered_map>

// Spezialisierung der Hash-Funktion für Lux-Strings
template <> inline
size_t stdext::hash_value<lux::string>(const lux::string& s)
{
	return (std::_Hash_seq((const unsigned char*)s.c_str(), s.Length() * sizeof(char)));
}

#endif // !INCLUDED_LXHASH_H
