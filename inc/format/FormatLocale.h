#ifndef INCLUDED_FORMAT_FORMAT_LOCALE_H
#define INCLUDED_FORMAT_FORMAT_LOCALE_H
#include "format/FormatConfig.h"
#include "format/Exception.h"
#include "format/Context.h"
#include <string>
#include <type_traits>

namespace format
{

/** \addtogroup Localisation
@{
*/

struct locale_exception : public format_exception
{
	explicit locale_exception(const char* msg) :
		format_exception(msg)
	{
	}
};

//! A facet represents an aspect of localication
/**
Examples for facets how to write number or booleans or dates and so on.
A concrete facets describes how this is handled for some concrete locale.
Facets are identified by their typeid
Each type of facet(number, boolean, etc.) is associated with a single facet type
*/
class Facet
{
public:
	virtual ~Facet() = default;

	//! Create a copy of the facet
	virtual Facet* Clone() const = 0;
};

//! The facet describing the number format
/**
All facet string are null-terminated utf8-strings of any lenght
*/
class Facet_NumericalFormat : public Facet
{
public:
	Facet_NumericalFormat(
		const char* sep,
		const char* com,
		const char* _plus,
		const char* _minus,
		const char* _NaN,
		const char* _Inf,
		int digitCount) :
		Seperator(sep),
		Comma(com),
		Plus(_plus),
		Minus(_minus),
		NaN(_NaN),
		Inf(_Inf),
		DefaultDigitCount(digitCount)
	{
	}

	Facet_NumericalFormat* Clone() const override { return new Facet_NumericalFormat(*this); }

public:
	std::string Seperator; //! Seperator of thousands
	std::string Comma; //! Character used seperate pre-comma and after-comma value
	std::string Plus; //! Mark position values
	std::string Minus; //! Mark negative values
	std::string NaN; //! Name of the not-a-number value
	std::string Inf; //! Name of the infinite value
	int DefaultDigitCount; //! Number of after-comma digit, if nothing is specified
};

extern FORMAT_API Facet_NumericalFormat NumericalFormat_English; //!< Numerical format for english language
extern FORMAT_API Facet_NumericalFormat NumericalFormat_German; //!< Numerical format for german language

//! The facet describing the boolean format
/**
All facet string are null-terminated utf8-strings of any lenght
*/
class Facet_BooleanFormat : public Facet
{
public:
	Facet_BooleanFormat(const char* t, const char* f) :
		True(t),
		False(f)
	{
	}

	Facet_BooleanFormat* Clone() const override { return new Facet_BooleanFormat(*this); }

public:
	std::string True; //!< Null-terminated utf8-string to represent <true>
	std::string False; //!< Null-terminated utf8-string to represent <false>
};

extern FORMAT_API Facet_BooleanFormat BooleanFormat_English; //!< Boolean format for english language
extern FORMAT_API Facet_BooleanFormat BooleanFormat_German; //!< Boolean format for german language
extern FORMAT_API Facet_BooleanFormat BooleanFormat_Digit; //!< Boolean format to write boolean values as digits 0 and 1

class Facet_Functions : public Facet
{
public:
	enum class EValueKind
	{
		Integer,
		String,
		Placeholder,
	};
	struct Value
	{
		int iValue;
		Slice sValue;
		FormatEntry* pValue;
	};

	typedef Value(*FormatFunctionPtr)(Context& ctx, Value* args);

	using InitList = std::initializer_list<std::pair<const char*, FormatFunctionPtr>>;

public:
	explicit Facet_Functions(const InitList& init) :
		Facet_Functions(nullptr, init)
	{
	}
	Facet_Functions(const Facet_Functions* base, const InitList& init);

	virtual ~Facet_Functions() = default;

	// TODO: Faster lookup.
	FormatFunctionPtr GetFunction(
		Slice name,
		int& args,
		const EValueKind*& outArgs,
		EValueKind& outRetType) const;

	Facet_Functions* Clone() const override { return new Facet_Functions(*this); }

private:
	int LookUp(Slice name) const;
	void AddFunction(Slice interface, FormatFunctionPtr function);

private:
	struct SelfData;
	std::shared_ptr<SelfData> self;
};

//! A locale is a collection of facets.
class Locale
{
public:
	FORMAT_API Locale(
		const Facet_NumericalFormat& num = NumericalFormat_English,
		const Facet_BooleanFormat& boolean = BooleanFormat_English);

	FORMAT_API Locale(const Locale& other);
	FORMAT_API ~Locale();

	FORMAT_API Locale& operator=(const Locale& other);

	//! Retrieve a facet from the locale.
	/**
	A locale_exception is thrown if the facet is not available.
	*/
	template <typename T>
	const T& GetFacet() const
	{
		static_assert(std::is_base_of<Facet, T>::value, "T must inherit from Facet");
		return static_cast<const T&>(GetFacet(typeid(T)));
	}
	//! Retrieve a modifiable facet from the locale.
	/**
	A locale_exception is thrown if the facet is not available.
	*/
	template <typename T>
	T& GetFacet()
	{
		static_assert(std::is_base_of<Facet, T>::value, "T must inherit from Facet");
		return static_cast<T&>(GetFacet(typeid(T)));
	}

	//! Change the value of an existing facet, or add a new facet type to the locale
	/**
	The facet is copied.
	*/
	template <typename T>
	void SetFacet(const T& facet)
	{
		static_assert(std::is_base_of<Facet, T>::value, "T must inherit from Facet");
		SetFacet(typeid(T), facet);
	}

	//! These facets are always available.
	FORMAT_API const Facet_NumericalFormat& GetNumericalFacet() const;
	FORMAT_API const Facet_BooleanFormat& GetBooleanFacet() const;
	FORMAT_API const Facet_Functions& GetFunctionsFacet() const;

private:
	FORMAT_API const Facet& GetFacet(const std::type_info& type) const;
	FORMAT_API Facet& GetFacet(const std::type_info& type);

	FORMAT_API void SetFacet(const std::type_info& type, const Facet& f);

private:
	struct SelfData;
	std::unique_ptr<SelfData> self;
};

extern FORMAT_API Locale InvariantLocale; //!< Default locale for the invariant language
extern FORMAT_API Locale English; //!< Default locale for the english language
extern FORMAT_API Locale German; //!< Default locale for the german language

//! Set the default locale for all format calls.
/**
The locale is not copied and must exist until another locale is set.
*/
FORMAT_API void SetLocale(Locale* l);

//! Get the current default locale.
FORMAT_API Locale* GetLocale();

/** @}*/
}

#endif // #ifndef INCLUDED_FORMAT_FORMAT_LOCALE_H
