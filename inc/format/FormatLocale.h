#ifndef INCLUDED_FORMAT_FORMAT_LOCALE_H
#define INCLUDED_FORMAT_FORMAT_LOCALE_H
#include "core/LuxBase.h"
#include "format/Exception.h"

namespace format
{

/** \addtogroup Localisation
@{
*/

//! Marker type for diffrent facet-types
/**
You should never need to create an instance of this class
*/
struct FacetType
{
public:
	FacetType()
	{
	}

	FacetType(const FacetType&) = delete;
	FacetType& operator=(const FacetType&) = delete;
};

extern LUX_API FacetType NumericalFormat; //!< The marker of the numerical facet
extern LUX_API FacetType BooleanFormat; //!< The marker of the boolean facet

//! A facet represents an aspect of localication
/**
For example how to write number or booleans or dates and so on.
A concrete facets describes how this is handled for some concrete language.
Each type of facet(number, boolean, etc.) is assosiated with a single facet type
*/
class Facet
{
public:
	Facet(const FacetType& type) :
		m_Type(type)
	{
	}

	virtual ~Facet() {}

	//! Cast the facet to a given type, and throw an exception if it fails
	template <typename T>
	T& As()
	{
		T* out = dynamic_cast<T*>(this);
		if(!out)
			throw bad_cast_exception("Bad locale facet cast");

		return *out;
	}

	//! Cast the facet to a given type, and throw an exception if it fails
	template <typename T>
	const T& As() const
	{
		const T* out = dynamic_cast<const T*>(this);
		if(!out)
			throw bad_cast_exception("Bad locale facet cast");

		return *out;
	}

	//! Get the type of the facet
	const FacetType& GetType() const
	{
		return m_Type;
	}

	//! Create a copy of the facet
	virtual Facet* Clone() const = 0;

private:
	const FacetType& m_Type;
};

//! The facet describing the number format
/**
All facet string are null-terminated utf8-strings of any lenght
*/
class Facet_NumericalFormat : public Facet
{
public:
	Facet_NumericalFormat() :
		Facet(NumericalFormat)
	{
	}

	Facet_NumericalFormat(
		const char* sep = "\'",
		const char* com = ".",
		const char* _plus = "+",
		const char* _minus = "-",
		const char* _NaN = "nan",
		const char* _Inf = "inf") :
		Facet(NumericalFormat),
		Seperator(sep),
		Comma(com),
		Plus(_plus),
		Minus(_minus),
		NaN(_NaN),
		Inf(_Inf),
		DefaultDigitCount(4)
	{
	}

	virtual Facet_NumericalFormat* Clone() const
	{
		return new Facet_NumericalFormat(*this);
	}

public:
	const char* Seperator; //! Seperator of thousands
	const char* Comma; //! Character used seperate pre-comma and after-comma value
	const char* Plus; //! Mark position values
	const char* Minus; //! Mark negative values
	const char* NaN; //! Name of the not-a-number value
	const char* Inf; //! Name of the infinite value
	int DefaultDigitCount; //! Number of after-comma digit, if nothing is specified
};

extern LUX_API Facet_NumericalFormat NumericalFormat_English; //!< Numerical format for english language
extern LUX_API Facet_NumericalFormat NumericalFormat_German; //!< Numerical format for german language

//! The facet describing the boolean format
/**
All facet string are null-terminated utf8-strings of any lenght
*/
class Facet_BooleanFormat : public Facet
{
public:
	Facet_BooleanFormat() :
		Facet(BooleanFormat),
		True("true"),
		False("false")
	{
	}

	Facet_BooleanFormat(const char* t, const char* f) :
		Facet(BooleanFormat),
		True(t),
		False(f)
	{
	}

	virtual Facet_BooleanFormat* Clone() const
	{
		return new Facet_BooleanFormat(*this);
	}

public:
	const char* True; //!< Null-terminated utf8-string to represent <true>
	const char* False; //!< Null-terminated utf8-string to represent <false>
};

extern LUX_API Facet_BooleanFormat BooleanFormat_English; //!< Boolean format for english language
extern LUX_API Facet_BooleanFormat BooleanFormat_German; //!< Boolean format for german language
extern LUX_API Facet_BooleanFormat BooleanFormat_Digit; //!< Boolean format to write boolean values as digits 0 and 1

//! A locale is a collection of facets, describing a whole language
class Locale
{
public:
	LUX_API Locale(
		const Facet_NumericalFormat& num = NumericalFormat_English,
		const Facet_BooleanFormat& boolean = BooleanFormat_English);

	LUX_API Locale(const Locale& other);
	LUX_API ~Locale();

	LUX_API Locale& operator=(const Locale& other);
	LUX_API const Facet& GetFacet(const FacetType& type) const;
	LUX_API Facet& GetFacet(const FacetType& type);

	//! Change the value of an existing facet, or add a new facet type to the locale
	/**
	The value of the facet is copied from the passed facet
	*/
	LUX_API void SetFacet(const FacetType& type, const Facet& f);

	LUX_API const Facet_NumericalFormat& GetNumericalFacet() const;
	LUX_API const Facet_BooleanFormat& GetBooleanFacet() const;

	LUX_API Facet_NumericalFormat& GetNumericalFacet();
	LUX_API Facet_BooleanFormat& GetBooleanFacet();

private:
	struct SelfData;
	SelfData* self;
};

extern Locale English; //!< Default locale for english language
extern Locale German; //!< Default locale for german language

//! Set the default locale for format calls
/**
The locale is not copied and must exist until another locale is set
*/
LUX_API void SetLocale(Locale* l);

//! Get the current default locale
LUX_API Locale* GetLocale();

/** @}*/
}

#endif // #ifndef INCLUDED_FORMAT_FORMAT_LOCALE_H
