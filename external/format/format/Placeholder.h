#pragma once

namespace format
{
struct Placeholder
{
	struct Option
	{
	private:
		int value;
		bool enabled;
		bool default;
		bool placeholder;

	public:
		Option() :
			enabled(false),
			default(false),
			placeholder(false),
			value(0)
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
			return enabled && !default;
		}

		bool IsDefault() const
		{
			if(!enabled)
				return false;

			return default;
		}

		bool IsPlaceholder() const
		{
			return placeholder;
		}

		void SetDefault()
		{
			placeholder = false;
			default = true;
		}

		void SetPlaceholder()
		{
			default = false;
			placeholder = true;
		}

		void SetValue(int i)
		{
			default = false;
			placeholder = false;
			value = i;
		}

		int GetValue(int _default = 0) const
		{
			if(enabled) {
				if(default)
					return _default;
				else
					return value;
			} else
				return _default;
		}
	};

	char type;
	Option master;
	Option hash;
	Option dot;
	Option plus;
	Option minus;
	Option at;
	Option star;
	Option left_align;
	Option right_align;

	char order[8 + 1];
	char placeholderOrder[9 + 1];
	int subPlaceholderCount;

	Placeholder(char _c = 0) :
		type(_c),
		subPlaceholderCount(0)
	{
		order[0] = 0;
		placeholderOrder[0] = 0;
	}

	void Reset(char _c = 0)
	{
		memset(this, 0, sizeof(Placeholder));
		type = _c;
	}

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