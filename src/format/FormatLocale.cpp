#include "format/FormatLocale.h"
#include "format/GeneralParsing.h"
#include <vector>

namespace format
{
Facet_NumericalFormat NumericalFormat_English("\'", ".", "+", "-", "nan", "inf", 4);
Facet_NumericalFormat NumericalFormat_German(".", ",", "+", "-", "nan", "inf", 4);

Facet_BooleanFormat BooleanFormat_English("true", "false");
Facet_BooleanFormat BooleanFormat_German("wahr", "falsch");
Facet_BooleanFormat BooleanFormat_Digit("1", "0");

Locale InvariantLocale(NumericalFormat_English, BooleanFormat_Digit);
Locale English(NumericalFormat_English, BooleanFormat_English);
Locale German(NumericalFormat_German, BooleanFormat_German);

///////////////////////////////////////////////////////////////////////////////

namespace
{
struct FunctionEntry
{
	std::string name;
	std::vector<Facet_Functions::EValueKind> args;
	Facet_Functions::EValueKind retType;
	Facet_Functions::FormatFunctionPtr function;
};

static Facet_Functions::EValueKind ParseKind(parser::SaveStringReader& str)
{
	auto w = parser::ReadWord(str);
	if(w == Slice(3, "str"))
		return Facet_Functions::EValueKind::String;
	if(w == Slice(3, "int"))
		return Facet_Functions::EValueKind::Integer;
	if(w == Slice(3,"plc"))
		return Facet_Functions::EValueKind::Placeholder;
	throw format_exception("invalid_interface");
}
static void ParseInterface(parser::SaveStringReader& str, FunctionEntry& entry)
{
	entry.retType = ParseKind(str);
	parser::SkipSpace(str);
	auto name = ReadWord(str);
	if(name.size == 0)
		throw format_exception("invalid_interface");
	entry.name.assign(name.data, name.size);
	if(!str.CheckAndGet('('))
		throw format_exception("invalid_interface");
	do {
		parser::SkipSpace(str);
		auto kind = ParseKind(str);
		entry.args.push_back(kind);
		parser::SkipSpace(str);
	} while(str.CheckAndGet(','));

	if(!str.CheckAndGet(')'))
		throw format_exception("invalid_interface");
}

}

struct Facet_Functions::SelfData
{
	std::vector<FunctionEntry> m_Entries;
};

Facet_Functions::Facet_Functions(const Facet_Functions* base, const InitList& init) :
	self(std::make_shared<Facet_Functions::SelfData>())
{
	self->m_Entries.reserve((base ? base->self->m_Entries.size() : 0) + init.size());
	if(base)
		self->m_Entries = base->self->m_Entries;
	for(auto& x : init)
		AddFunction(Slice(int(std::strlen(x.first)), x.first), x.second);
}

// TODO: Faster lookup.
Facet_Functions::FormatFunctionPtr Facet_Functions::GetFunction(
	Slice name,
	int& args,
	const EValueKind*& outArgs,
	EValueKind& outRetType) const
{
	auto entryId = LookUp(name);
	if(entryId < 0)
		return nullptr;

	auto& entry = self->m_Entries[entryId];
	args = int(entry.args.size());
	outArgs = entry.args.data();
	outRetType = entry.retType;
	return entry.function;
}

int Facet_Functions::LookUp(Slice name) const
{
	for(size_t i = 0; i < self->m_Entries.size(); ++i) {
		auto& e = self->m_Entries[i];
		if(e.name.size() == size_t(name.size) && 
			0 == std::memcmp(e.name.data(), name.data, name.size))
			return int(i);
	}
	return -1;
}

void Facet_Functions::AddFunction(Slice interface, FormatFunctionPtr function)
{
	FunctionEntry newEntry;
	parser::SaveStringReader reader(interface);
	ParseInterface(reader, newEntry);
	newEntry.function = function;

	Slice nameSlice(int(newEntry.name.size()), newEntry.name.c_str());
	auto oldEntryId = LookUp(nameSlice);
	if(oldEntryId >= 0)
		self->m_Entries[oldEntryId] = newEntry;
	else
		self->m_Entries.push_back(newEntry);
}

///////////////////////////////////////////////////////////////////////////////

struct Locale::SelfData
{
	std::vector<std::pair<const std::type_info&, std::unique_ptr<Facet>>> facets;
};

Locale::Locale(
	const Facet_NumericalFormat& num,
	const Facet_BooleanFormat& boolean) :
	self(std::make_unique<SelfData>())
{
	SetFacet(num);
	SetFacet(boolean);
	SetFacet(Facet_Functions({}));
}

Locale::Locale(const Locale& other) :
	self(std::make_unique<SelfData>())
{
	self->facets.reserve(other.self->facets.size());
	for(auto& e : other.self->facets)
		self->facets.emplace_back(e.first, std::unique_ptr<Facet>(e.second->Clone()));
}

Locale::~Locale()
{
}

Locale& Locale::operator=(const Locale& other)
{
	self->facets.clear();

	for(auto& e : other.self->facets)
		self->facets.emplace_back(e.first, std::unique_ptr<Facet>(e.second->Clone()));

	return *this;
}

const Facet& Locale::GetFacet(const std::type_info& type) const
{
	for(auto& p : self->facets) {
		if(p.first == type)
			return *p.second;
	}
	throw locale_exception("Unknown locale-facet requested.");
}

Facet& Locale::GetFacet(const std::type_info& type)
{
	for(auto& p : self->facets) {
		if(p.first == type)
			return *p.second;
	}
	throw locale_exception("Unknown locale-facet requested.");
}

void Locale::SetFacet(const std::type_info& type, const Facet& f)
{
	for(auto& p : self->facets) {
		if(p.first == type) {
			p.second = std::unique_ptr<Facet>(f.Clone());
			return;
		}
	}
	self->facets.emplace_back(type, std::unique_ptr<Facet>(f.Clone()));
}

const Facet_NumericalFormat& Locale::GetNumericalFacet() const
{
	return GetFacet<Facet_NumericalFormat>();
}

const Facet_BooleanFormat& Locale::GetBooleanFacet() const
{
	return GetFacet<Facet_BooleanFormat>();
}

const Facet_Functions& Locale::GetFunctionsFacet() const
{
	return GetFacet<Facet_Functions>();
}

///////////////////////////////////////////////////////////////////////////////

static Locale* g_Locale = &format::English;

void SetLocale(Locale* l)
{
	g_Locale = l;
}

Locale* GetLocale()
{
	return g_Locale;
}

}
