#pragma once




namespace Arthmetic
{
	using StringIterator = std::string_view::const_iterator;
	//using StringIterator = std::string::const_iterator;

	


	//Move to NEB Utility
	template <class T>requires(std::is_enum_v<T>)
	inline constexpr T operator~(T a)
	{
		return (T)~(int)a;
	}
	template <class T>requires(std::is_enum_v<T>)
	inline constexpr T operator|(T a, T b)
	{
		return (T)((int)a | (int)b);
	}
	template <class T>requires(std::is_enum_v<T>)
	inline constexpr T operator&(T a, T b)
	{
		return (T)((int)a & (int)b);
	}
	template <class T>requires(std::is_enum_v<T>)
	inline constexpr T operator^(T a, T b)
	{
		return (T)((int)a ^ (int)b);
	}
	template <class T>requires(std::is_enum_v<T>)
	inline constexpr T& operator|=(T& a, T b)
	{
		return (T&)((int&)a |= (int)b);
	}
	template <class T>requires(std::is_enum_v<T>)
	inline constexpr T& operator&=(T& a, T b)
	{
		return (T&)((int&)a &= (int)b);
	}
	template <class T>requires(std::is_enum_v<T>)
	inline constexpr T& operator^=(T& a, T b)
	{
		return (T&)((int&)a ^= (int)b);
	}

	template <class T>requires(std::is_enum_v<T>)
	inline constexpr bool operator!(T t) {
		return t == static_cast<T>(0);
	}

	template <class T>requires(std::is_enum_v<T>)
	inline constexpr bool operator==(T t, bool b) {
		return (static_cast<int>(t) != 0) == b;
	}


	template <class T>requires(std::is_enum_v<T>)
	inline constexpr bool operator!=(T t, bool b) {
		return (static_cast<int>(t) != 0) != b;
	}

	template <class T>requires(std::is_enum_v<T>)
	inline constexpr T operator<<(T t, int i) {
		return static_cast<T>(static_cast<int>(t) << i);
	}
	
	template <class T>requires(std::is_enum_v<T>)
	inline constexpr T operator>>(T t, int i) {
		return static_cast<T>(static_cast<int>(t) >> i);
	}


	template <class T>requires(std::is_enum_v<T>)
	inline constexpr T& operator<<=(T t, int i) {
		return t = t << i;
	}

	template <class T>requires(std::is_enum_v<T>)
	inline constexpr T& operator>>=(T t, int i) {
		return t = t >> i;
	}

	
	template<class T>
	struct type_name { static inline const char* value = typeid(T).name(); };

	//template<class T>
	//const char* type_name_v = typeid(T).name();

	template <class ClassType, class IdentifierType>
	class ClassFactory
	{
	public:
		//This will have to have some light implementation from the class routineitems (and other classes) must derive from.
		using RegisterFunction = ClassType*(*)();

		using Factory = ClassFactory<ClassType, IdentifierType>;

		inline static std::unordered_map<IdentifierType, RegisterFunction> creationFactory{};

	public:
		ClassFactory() = default;

	private://Why does this need to be public?

		//This should be private, wouldn't want someone thinking they had to inherit this.
		ClassFactory(IdentifierType enum_type, RegisterFunction registration)
		{
			//logger::critical("Factory Function created for val {}, of enum:{} related to class:{}",
			//	(int)enum_type, type_name_v<IdentifierType>, type_name_v<ClassType>);

			creationFactory[enum_type] = registration;
		}

	protected:
		//Used to solve the protected constructor issues.
		static inline Factory Ctor(IdentifierType enum_type, RegisterFunction reg) { return Factory(enum_type, reg); }

	public:
		static ClassType* Create(IdentifierType type)
		{
			//I'll just directly access, because if I don't find it, I'm just crashing.
			RegisterFunction func = creationFactory[type];

			if (!func) {
				logger::critical("throwing error for val {}, of enum:{} related to class:{}", 
					(int)type, type_name<IdentifierType>::value, type_name<ClassType>::value);
				throw nullptr;
			}
			else
			{
				logger::debug("Success for val {}, of enum:{} related to class:{}",
					(int)type, type_name<IdentifierType>::value, type_name<ClassType>::value);
			}

			ClassType* result = func();

			return result;
		}
	};

	//There are no safeties on this, take care.
	template <class ClassType, class IdentifierType, class RelativeType, IdentifierType Value>//for relative type std::derived_from<ClassType>
	class IndirectFactoryComponent : private ClassFactory<ClassType, IdentifierType>
	{

	private:

		using ClassFactoryInit = ClassFactory<ClassType, IdentifierType>;
		using FactoryComponent = IndirectFactoryComponent<ClassType, IdentifierType, RelativeType, Value>;

		static ClassType* CreateRelative()
		{
			RelativeType* result = new RelativeType();
			//Some form of inate implementation goes here or something like that. No idea how to move this bit around.
			// But this basically this just allows you to use an enum or integral to create a class.


			logger::debug("Creating {}... {}", type_name<RelativeType>::value, result != nullptr);


			return result;
		}


		inline static ClassFactoryInit _initializer = FactoryComponent::Ctor(Value, CreateRelative);
	public:
	};
	//Make a shorter version, template wise for specific use cases.



	//Not really needed since reinterpret cast will work now.
	union ConversionHelper
	{
	public:
		float _float;
		uint32_t _integer;

		constexpr ConversionHelper(float value) :
			_float(value) {}
		constexpr ConversionHelper(uint32_t value) :
			_integer(value) {}
	};

	struct ArthmeticUtility
	{
		static constexpr uint32_t mantissa = 0x007FFFFF;

		//Simply returns true if the first character
		static char CheckAlpha(StringIterator& it)
		{
			return isalpha(*it) ? '\0' : *it;
		}

		static float ConvertToNaN(uint32_t value)
		{
			ConversionHelper helper(INFINITY);
			helper._integer |= value;
			return helper._float;
			//uint32_t mantissa = std::bit_cast<uint32_t>(INFINITY);

			//uint32_t result = value | mantissa;

			//return reinterpret_cast<float>(result);
		}

		static uint32_t ConvertToValue(float value)
		{
			if (isnan(value) == false)
				return 0;

			//return reinterpret_cast<uint32_t>(value) & Mantissa();

			ConversionHelper helper(value);
			return helper._integer & mantissa;
		}
	};


	//Move this to RoguesGallery
	template<bool...> struct bool_pack;

	template <class Enum, Enum A, Enum B> inline constexpr bool enum_contains = (A & B) == B;

	template <class Type, Type val, Type set> struct set_type { static constexpr Type value = set; };
	template <class Type, Type val, Type set>  inline constexpr Type set_type_v = set_type<Type, val, set>::value;

	template<bool... bs>
	using all_true = std::is_same<bool_pack<bs..., true>, bool_pack<true, bs...>>;

	template<bool... bs>
	using any_true = std::is_same<bool_pack<bs..., true>, bool_pack<true, set_type_v<bool, bs, false>...>>;


	template<bool... bs>  inline constexpr bool all_true_v = all_true<bs...>::value;
	template<bool... bs>  inline constexpr bool any_true_v = any_true<bs...>::value;


	using ArthUtil = ArthmeticUtility;
	using ArthmeticUtil = ArthmeticUtility;


	//Move to the relied upon utility
	using string_hash = uint32_t;



	template <class CharType>
	constexpr CharType ConstToLower(const CharType c)
	{
		switch (c)
		{
		case 'A': return 'a';
		case 'B': return 'b';
		case 'C': return 'c';
		case 'D': return 'd';
		case 'E': return 'e';
		case 'F': return 'f';
		case 'G': return 'g';
		case 'H': return 'h';
		case 'I': return 'i';
		case 'J': return 'j';
		case 'K': return 'k';
		case 'L': return 'l';
		case 'M': return 'm';
		case 'N': return 'n';
		case 'O': return 'o';
		case 'P': return 'p';
		case 'Q': return 'q';
		case 'R': return 'r';
		case 'S': return 's';
		case 'T': return 't';
		case 'U': return 'u';
		case 'V': return 'v';
		case 'W': return 'w';
		case 'X': return 'x';
		case 'Y': return 'y';
		case 'Z': return 'z';
		default: return c;
		}
	}


	template<bool Insensitive = false>
	constexpr string_hash hash(const char* data, size_t const size) noexcept
	{
		string_hash hash = 5381;

		for (const char* c = data; c < data + size; ++c) {
			unsigned char _c;

			if constexpr (Insensitive) _c = (unsigned char)ConstToLower(*c);
			else _c = (unsigned char)*c;

			hash = ((hash << 5) + hash) + _c;
		}

		return hash;
	}

	template<bool Insensitive = false>
	constexpr string_hash hash(const char* data) noexcept
	{
		string_hash hash = 5381;

		for (const char* c = data; *c == '\0'; ++c) {
			//hash = ((hash << 5) + hash) + (unsigned char)std::tolower(*c);
			unsigned char _c;

			if constexpr (Insensitive) _c = (unsigned char)ConstToLower(*c);
			else _c = (unsigned char)*c;

			hash = ((hash << 5) + hash) + _c;
		}

		return hash;
	}
	
	template<bool Insensitive = false, size_t N = 1>
	constexpr string_hash hash(const char(&data)[N]) noexcept
	{
		string_hash hash = 5381;

		for (const char* c = data; *c == '\0'; ++c) {
			unsigned char _c;

			if constexpr (Insensitive) _c = (unsigned char)ConstToLower(*c);
			else _c = (unsigned char)*c;

			hash = ((hash << 5) + hash) + _c;
		}

		return hash;
	}

	template<bool Insensitive = false>
	constexpr string_hash hash(std::string& string) noexcept
	{
		return hash<Insensitive>(string.data(), string.size());
	}

	template<bool Insensitive = false>
	constexpr string_hash hash(std::string_view& view) noexcept
	{
		return hash<Insensitive>(view.data(), view.size());
	}

	template<bool Insensitive = false>
	constexpr string_hash hash(const std::string& string) noexcept
	{
		return hash<Insensitive>(string.data(), string.size());
	}

	template<bool Insensitive = false>
	constexpr string_hash hash(const std::string_view& view) noexcept
	{
		return hash<Insensitive>(view.data(), view.size());
	}


	constexpr string_hash operator"" _h(const char* str, size_t size) noexcept
	{
		//function and operator courtesy of Ershin's TDM
		return hash<false>(str, size);
	}


	constexpr string_hash operator"" _ih(const char* str, size_t size) noexcept
	{
		//function and operator courtesy of Ershin's TDM
		return hash<true>(str, size);
	}

	template<size_t N>
	struct StringLiteral {
		static constexpr size_t Size = N;
		constexpr StringLiteral(const char(&str)[N]) {
			std::copy_n(str, N, value);
		}

		constexpr operator const char* () { return &value; }
		

		constexpr operator std::string_view() { return std::string_view(&value, Size); }

		
		constexpr std::string_view view() { return std::string_view(&value, Size); }

		char value[N];
	};

	//template<const char* c_str> constexpr string_hash hash_string = hash(c_str);
	template<StringLiteral Str, bool Insensitive = true> constexpr string_hash hash_string = hash<Insensitive>(Str.value);

	//test idea for case insensitive character traits
	struct ci_char_traits : public std::char_traits<char> {
		static char to_upper(char ch) {
			return std::toupper((unsigned char)ch);
		}
		static bool eq(char c1, char c2) {
			return to_upper(c1) == to_upper(c2);
		}
		static bool lt(char c1, char c2) {
			return to_upper(c1) < to_upper(c2);
		}
		static int compare(const char* s1, const char* s2, std::size_t n) {
			while (n-- != 0) {
				if (to_upper(*s1) < to_upper(*s2)) return -1;
				if (to_upper(*s1) > to_upper(*s2)) return 1;
				++s1; ++s2;
			}
			return 0;
		}
		static const char* find(const char* s, std::size_t n, char a) {
			auto const ua(to_upper(a));
			while (n-- != 0)
			{
				if (to_upper(*s) == ua)
					return s;
				s++;
			}
			return nullptr;
		}
	};


	//Core function should go on longer.
	int powi(int base, unsigned int exp)
	{
		int res = 1;
		while (exp) {
			if (exp & 1)
				res *= base;
			exp >>= 1;
			base *= base;
		}
		return res;
	}

	uint32_t StrHexToInt32(const std::string& hex, bool zero_x_acc = true)
	{
		auto size = hex.size();
		size_t front;

		if (hex.size() == 0)
			return 0;
		//Accounting for the use of Ox
		if (size > 10)
			return 0xFFFFFFFF;

		//If the use of Ox isnt allowed, the size is over
		if (std::strncmp(hex.c_str(), "0x", 2) == 0)
			if (!zero_x_acc)
				return 0xFFFFFFFF;
			else
				front = 2;
		else
			front = 0;

		auto it = hex.crbegin();
		auto end = hex.crend() + front;

		int expo = 0;

		uint32_t result = 0;

		while (it != end)
		{
			int inc = 0;

			switch (*it)
			{
			case '0': break;
			case '1': inc = 1; break;
			case '2': inc = 2; break;
			case '3': inc = 3; break;
			case '4': inc = 4; break;
			case '5': inc = 5; break;
			case '6': inc = 6; break;
			case '7': inc = 7; break;
			case '8': inc = 8; break;
			case '9': inc = 9; break;

			case 'A':
			case 'a': inc = 10; break;
			case 'B':
			case 'b': inc = 11; break;
			case 'C':
			case 'c': inc = 12; break;
			case 'D':
			case 'd': inc = 13; break;
			case 'E':
			case 'e': inc = 14; break;
			case 'F':
			case 'f': inc = 15; break;

			case 'X':
			case 'x':
				return result;//If it's not caught some how.
			default:
				return 0xFFFFFFFF;//invalid value.

			}

			if (inc)
			{
				result |= inc * powi(16, expo);
			}

			expo++;
			it++;
		}

		return result;
	}

	static bool CharCmpI(char c1, char c2)
	{
		//May only have to do one.
		return c1 == c2 || std::toupper(c1) == std::toupper(c2);
	}


	static bool CStrCmpI(const char* str1, const char* str2, size_t length)
	{
		if (!str1 || !str2)
			return str1 == str2;

		for (int i = 0; i < length; i++)
		{
			const char& char1 = str1[i];
			const char& char2 = str2[i];

			if (char1 == '\0' || char2 == '\0')
				return str1 == str2;

			if (CharCmpI(char1, char2) == false)
				return false;
		}

		return true;
	}


	inline static bool CStrCmpI(std::string& str1, const char* str2) { return CStrCmpI(str1.data(), str2, str1.size()); }
	inline static bool CStrCmpI(const char* str1, std::string& str2) { return CStrCmpI(str1, str2.data(), str2.size()); }

	////_strnicmp() exists, use it instead.
	static bool StrCmpI(std::string str1, std::string str2)
	{
		return str1.size() == str2.size() &&
			std::equal(str1.begin(), str1.end(), str2.begin(), CharCmpI);
	}


	template <class EnumType, EnumType... Enums>requires(std::is_enum_v<EnumType>)
	inline constexpr bool ContainsEnum(EnumType value)
	{
		static constexpr size_t Size = sizeof...(Enums);

		static constexpr std::array<EnumType, Size> EnumValues{ Enums... };

		for (int i = 0; i < Size; i++)
			if (value = EnumValues[i])
				return true;

		return false;
	}


#define get_switch(mc_behaviour) auto switch_value = mc_behaviour; switch(switch_value)
//Performs an action, then returns. Uses braces to work with if statements.
#define do_return(mc_action) { mc_action; return; } true
#define do_return_X(mc_action, mc_what) { mc_action; return mc_what; } true

//What this should do is skip the true false, then when it wants to break it will do it by jumping back to __break;

#define cycle_switch(mc_flag) \
	if (false){\
		break_##mc_flag:\
		true;\
	}\
	else\
	for (decltype(mc_flag) __flag = (decltype(mc_flag))0x1; \
	__flag < sizeof(decltype(mc_flag)) * 8; __flag = __flag << 1) \
	switch (__flag & mc_flag)
//I'd like a second version that as a staring point, that way if I used a version that's a mix of these, I have the choice to bail.

	namespace
	{
		enum TestEnum
		{
			AAAA = 1 << 0,
			BBBB = 1 << 1,
			CCCC = 1 << 2,
			DDDD = 1 << 3,
			EEEE = 1 << 4,
		};


		void Test()
		{
			TestEnum d = AAAA | BBBB | CCCC | DDDD | EEEE;
			cycle_switch(d)
			{
				case AAAA:
					logger::info("a");
					break;

				case BBBB:
					logger::info("b");
					break;

				case CCCC:
					logger::info("c");
					break;

				case DDDD:
					logger::info("d");
					break;

				case EEEE:
					logger::info("e");
					break;

				default:
					logger::info("failed {}", (int)__flag);
					break;
			}

		}
	}
	//Would like a mirror version for stuff that isn't total to declare the error for
	template <class EnumType> requires(std::is_enum_v<EnumType>)
	constexpr size_t total = static_cast<size_t>(EnumType::Total);
}