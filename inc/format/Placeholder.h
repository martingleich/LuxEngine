#ifndef INCLUDED_FORMAT_PLACEHOLDER_H
#define INCLUDED_FORMAT_PLACEHOLDER_H
#include <string.h>
#include <inttypes.h>

namespace format
{
struct Placeholder
{
	struct Option
	{
	private:
		int value; //!< The value of the option
		bool enabled : 1; //!< Is the option enabled, i.e. Used in the output
		bool hasDefaultValue : 1; //!< True if the option doesnt have a value passed
		bool placeholder : 1; //!< True if the option uses a placeholder

	public:
		Option() :
			value(0),
			enabled(false),
			hasDefaultValue(false),
			placeholder(false)
		{
		}

		operator bool() const
		{
			return enabled;
		}

		bool IsEnabled() const
		{
			return enabled;
		}

		void Enable()
		{
			enabled = true;
		}

		void Disable()
		{
			enabled = false;
		}

		bool HasValue() const
		{
			return enabled && !hasDefaultValue;
		}

		bool IsDefault() const
		{
			if(!enabled)
				return false;

			return hasDefaultValue;
		}

		bool IsPlaceholder() const
		{
			return placeholder;
		}

		void SetDefault()
		{
			placeholder = false;
			hasDefaultValue = true;
		}

		void SetPlaceholder()
		{
			enabled = true;
			hasDefaultValue = false;
			placeholder = true;
		}

		void SetValue(int i)
		{
			enabled = true;
			hasDefaultValue = false;
			placeholder = false;
			value = i;
		}

		int GetValue(int _default = 0) const
		{
			if(enabled) {
				if(hasDefaultValue)
					return _default;
				else
					return value;
			} else
				return _default;
		}
	};

	Option master; //!< The first value passed to the placeholder, directly after the ~
	Option hash;
	Option dot;
	Option plus;
	Option minus;
	Option at;
	Option star;
	Option left_align;
	Option right_align;

	char order[8 + 1]; //!< The order in which the option where set, string is null-terminated, only enabled options
	char placeholderOrder[9 + 1]; //!< The order in which the sub-placeholder where used, string is null-terminated, only enabled options
	char type; //!< The type character of the placeholder

	Placeholder(char _c = 0) :
		type(_c)
	{
		order[0] = 0;
		placeholderOrder[0] = 0;
	}

	//! Reset all options to disabled
	void Reset(char _c = 0)
	{
		memset(this, 0, sizeof(Placeholder));
		type = _c;
	}

	//! Get a Option based on the character used to access it
	Placeholder::Option* GetOption(uint32_t c)
	{
		switch(c) {
		case '.': return &dot;
		case '@': return &at;
		case '#': return &hash;
		case '-': return &minus;
		case '+': return &plus;
		case '*': return &star;
		case '>': return &right_align;
		case '<': return &left_align;
		default: return nullptr;
		}
	}

	//! Has the placeholder subplaceholders.
	bool HasSubPlaceholder() const
	{
		return placeholderOrder[0] != '\0';
	}
};

inline bool IsValidPlaceholder(uint32_t c)
{
	return ((c >= 'a' && c <= 'z') || c == '~');
}

inline bool IsArgFreePlaceholder(uint32_t c)
{
	return (c == '~' || c == 't');
}

}

#endif // #ifndef INCLUDED_FORMAT_PLACEHOLDER_H
