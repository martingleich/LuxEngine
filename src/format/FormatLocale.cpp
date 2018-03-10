#include "format/FormatLocale.h"
#include <map>

namespace format
{
FacetType NumericalFormat;
FacetType BooleanFormat;

Facet_NumericalFormat NumericalFormat_English("\'", ".", "+", "-", "nan", "inf");
Facet_NumericalFormat NumericalFormat_German(".", ",", "+", "-", "nan", "inf");

Facet_BooleanFormat BooleanFormat_English("true", "false");
Facet_BooleanFormat BooleanFormat_German("wahr", "falsch");
Facet_BooleanFormat BooleanFormat_Digit("1", "0");

Locale English(NumericalFormat_English, BooleanFormat_English);
Locale German(NumericalFormat_German, BooleanFormat_German);

Locale* g_Locale = &format::English;

void SetLocale(Locale* l)
{
	g_Locale = l;
}

Locale* GetLocale()
{
	return g_Locale;
}

struct Locale::SelfData
{
	Facet_NumericalFormat* numerical;
	std::map<const FacetType*, Facet*> facets;
};

Locale::Locale(
	const Facet_NumericalFormat& num,
	const Facet_BooleanFormat& boolean) :
	self(new SelfData)
{
	self->numerical = num.Clone();
	SetFacet(BooleanFormat, boolean);
}

Locale::Locale(const Locale& other) :
	self(new SelfData)
{
	self->numerical = other.self->numerical->Clone();
	self->facets = other.self->facets;

	for(auto it = self->facets.begin(); it != self->facets.end(); ++it)
		it->second = it->second->Clone();
}

Locale::~Locale()
{
	for(auto it = self->facets.begin(); it != self->facets.end(); ++it)
		delete it->second;
	delete self;
}

Locale& Locale::operator=(const Locale& other)
{
	for(auto it = self->facets.begin(); it != self->facets.end(); ++it)
		delete it->second;

	self->facets = other.self->facets;

	for(auto it = self->facets.begin(); it != self->facets.end(); ++it)
		it->second = it->second->Clone();

	return *this;
}

const Facet& Locale::GetFacet(const FacetType& type) const
{
	if(&type == &NumericalFormat)
		return *self->numerical;

	auto it = self->facets.find(&type);
	if(it == self->facets.end())
		throw format_exception("Unknown locale-facet requested.");
	return *it->second;
}

Facet& Locale::GetFacet(const FacetType& type)
{
	if(&type == &NumericalFormat)
		return *self->numerical;

	auto it = self->facets.find(&type);
	if(it == self->facets.end())
		throw format_exception("Unknown locale-facet requested.");
	return *it->second;
}

void Locale::SetFacet(const FacetType& type, const Facet& f)
{
	if(&type == &NumericalFormat) {
		delete self->numerical;
		self->numerical = dynamic_cast<Facet_NumericalFormat*>(f.Clone());
		return;
	}

	auto it = self->facets.find(&type);
	if(it != self->facets.end()) {
		delete it->second;
		it->second = f.Clone();
	} else {
		self->facets.emplace(&type, f.Clone());
	}
}

const Facet_NumericalFormat& Locale::GetNumericalFacet() const
{
	return *self->numerical;
}

const Facet_BooleanFormat& Locale::GetBooleanFacet() const
{
	return GetFacet(BooleanFormat).As<Facet_BooleanFormat>();
}

Facet_NumericalFormat& Locale::GetNumericalFacet()
{
	return *self->numerical;
}

Facet_BooleanFormat& Locale::GetBooleanFacet()
{
	return GetFacet(BooleanFormat).As<Facet_BooleanFormat>();
}

}
