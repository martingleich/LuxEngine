#pragma once
#include "Exception.h"
#include <map>
#include <memory>

namespace format
{
namespace locale
{
struct FacetType
{
public:
	FacetType()
	{
	}

	FacetType(const FacetType&) = delete;
	FacetType& operator=(const FacetType&) = delete;
};

extern FacetType NumericalFormat;
extern FacetType BooleanFormat;

class Facet
{
public:
	Facet(const FacetType& type) :
		m_Type(type)
	{
	}

	virtual ~Facet() {}

	template <typename T>
	T& As()
	{
		T* out = dynamic_cast<T*>(this);
		if(!out)
			throw bad_cast_exception("Bad locale facet cast");

		return *out;
	}

	template <typename T>
	const T& As() const
	{
		const T* out = dynamic_cast<const T*>(this);
		if(!out)
			throw bad_cast_exception("Bad locale facet cast");

		return *out;
	}

	const FacetType& GetType() const
	{
		return m_Type;
	}

	virtual Facet* Clone() const = 0;

private:
	const FacetType& m_Type;
};

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
		Inf(_Inf)
	{
	}

	virtual Facet_NumericalFormat* Clone() const
	{
		return new Facet_NumericalFormat(*this);
	}

public:
	const char* Seperator;
	const char* Comma;
	const char* Plus;
	const char* Minus;
	const char* NaN;
	const char* Inf;
};

extern Facet_NumericalFormat NumericalFormat_English;
extern Facet_NumericalFormat NumericalFormat_German;

class Facet_BooleanFormat : public Facet
{
public:
	// Returns utf-8 string for boolean value b
	typedef const char* (*GetterT)(bool b);

public:
	static const char* EnglishGetter(bool b)
	{
		return b ? "true" : "false";
	}

	static const char* GermanGetter(bool b)
	{
		return b ? "wahr" : "falsch";
	}

	static const char* DigitGetter(bool b)
	{
		return b ? "1" : "0";
	}

	Facet_BooleanFormat() :
		Facet(BooleanFormat),
		GetString(&EnglishGetter)
	{
	}

	Facet_BooleanFormat(GetterT g) :
		Facet(BooleanFormat),
		GetString(g)
	{
	}

	virtual Facet_BooleanFormat* Clone() const
	{
		return new Facet_BooleanFormat(*this);
	}

public:
	GetterT GetString;
};

extern Facet_BooleanFormat BooleanFormat_English;
extern Facet_BooleanFormat BooleanFormat_German;
extern Facet_BooleanFormat BooleanFormat_Digit;

class Locale
{
public:
	Locale(
		const Facet_NumericalFormat& num = NumericalFormat_English,
		const Facet_BooleanFormat& boolean = BooleanFormat_English)
	{
		m_Numerical = num.Clone();
		SetFacet(BooleanFormat, boolean);
	}

	Locale(const Locale& other) :
		m_Numerical(other.m_Numerical->Clone()),
		m_Facets(other.m_Facets)
	{
		for(auto it = m_Facets.begin(); it != m_Facets.end(); ++it)
			it->second = it->second->Clone();
	}

	~Locale()
	{
		for(auto it = m_Facets.begin(); it != m_Facets.end(); ++it)
			delete it->second;
	}

	Locale& operator=(const Locale& other)
	{
		for(auto it = m_Facets.begin(); it != m_Facets.end(); ++it)
			delete it->second;

		m_Facets = other.m_Facets;

		for(auto it = m_Facets.begin(); it != m_Facets.end(); ++it)
			it->second = it->second->Clone();

		return *this;
	}

	const Facet& GetFacet(const FacetType& type) const
	{
		if(&type == &NumericalFormat)
			return *m_Numerical;
			
		auto it = m_Facets.find(&type);
		if(it == m_Facets.end())
			throw bad_locale_exception("Unknown locale-facet requested.");
		return *it->second;
	}

	Facet& GetFacet(const FacetType& type)
	{
		if(&type == &NumericalFormat)
			return *m_Numerical;

		auto it = m_Facets.find(&type);
		if(it == m_Facets.end())
			throw bad_locale_exception("Unknown locale-facet requested.");
		return *it->second;
	}

	void SetFacet(const FacetType& type, const Facet& f)
	{
		if(&type == &NumericalFormat) {
			delete m_Numerical;
			m_Numerical = dynamic_cast<Facet_NumericalFormat*>(f.Clone());
			return;
		}

		auto it = m_Facets.find(&type);
		if(it != m_Facets.end()) {
			delete it->second;
			it->second = f.Clone();
		} else {
			m_Facets.emplace(&type, f.Clone());
		}
	}

	const Facet_NumericalFormat& GetNumericalFacet() const
	{
		return *m_Numerical;
	}

	const Facet_BooleanFormat& GetBooleanFacet() const
	{
		return GetFacet(BooleanFormat).As<Facet_BooleanFormat>();
	}

	Facet_NumericalFormat& GetNumericalFacet()
	{
		return *m_Numerical;
	}

	Facet_BooleanFormat& GetBooleanFacet()
	{
		return GetFacet(BooleanFormat).As<Facet_BooleanFormat>();
	}
private:
	Facet_NumericalFormat* m_Numerical;
	std::map<const FacetType*, Facet*> m_Facets;
};

extern Locale English;
extern Locale German;

void SetLocale(Locale* l);
Locale* GetLocale();

}
}

