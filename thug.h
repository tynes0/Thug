#pragma once
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

namespace thug
{
	namespace detail
	{
		std::vector<std::string> separate_string(const std::string& source)
		{
			std::string temp = "";
			std::vector<std::string> result;

			for (size_t i = 0; i < source.size(); i++)
			{
				if (!::isspace(source[i]))
				{
					temp += source[i];
				}
				else
				{
					if (temp.empty())
						continue;
					result.push_back(temp);
					temp = "";
				}
			}
			if (!temp.empty())
				result.push_back(temp);

			return result;
		}
	}

	struct morse_format
	{
		char long_press = '-';
		char short_press = '.';
		char space = '/';

		bool operator==(morse_format right) const noexcept
		{
			return (long_press == right.long_press && short_press == right.short_press && space == right.space);
		}

		bool is_key(char key) const noexcept
		{
			return key == long_press || key == short_press || key == space;
		}
	};

	constexpr morse_format default_format{};

	enum class repair_mode
	{
		remove_incorrect_letter = 0, // Completely removes the incorrect letter
		remove_incorrect_key = 1, // It first tries to remove the wrong key. If the result doesn't make sense, it removes that letter entirely.
		try_replacing_with_short_press = 2, // It tries to replace the faulty key with short press key. If the result doesn't make sense, it removes that letter entirely.
		try_replacing_with_long_press = 3, // It tries to replace the faulty key with long press key. If the result doesn't make sense, it removes that letter entirely.
		try_ordered_repair_list_one_by_one = 4, // It tries the repairs on the ordered list one by one. If none of them work, it removes the letter entirely.
		default_mode = remove_incorrect_letter // Default mode is remove_incorrect_letter
	};

	class morse_converter
	{
	public:
		morse_converter(char long_press_key = default_format.long_press, char short_press_key = default_format.short_press, char space_key = default_format.space) : m_format({ long_press_key, short_press_key, space_key })
		{
			m_keys = get_key_table(m_format);
		}

		morse_converter(morse_format fmt) : m_format(fmt)
		{
			m_keys = get_key_table(m_format);
		}

		morse_converter(const morse_converter& other) : m_format(other.m_format)
		{
			m_keys = get_key_table(m_format);
		}

		morse_converter(morse_converter&& other) noexcept : m_format(std::move(other).m_format), m_keys(std::move(other).m_keys) {}

		morse_converter& operator=(morse_format fmt)
		{
			set_format(fmt);
		}

		morse_converter& operator=(const morse_converter& other)
		{
			set_format(other.m_format);
		}

		void set_format(char long_press_key, char short_press_key, char space_key = default_format.space)
		{
			auto temp = m_format;
			m_format = { long_press_key, short_press_key, space_key };
			m_keys = get_key_table(m_format);
		}

		void set_format(morse_format fmt)
		{
			auto temp = m_format;
			m_format = fmt;
			m_keys = get_key_table(m_format);
		}

		// text to morse
		std::string encode(const std::string& text) const
		{
			std::stringstream ss;
			for (size_t i = 0; i < text.size(); ++i)
			{
				if (m_keys.find(tolower(text[i])) != m_keys.end())
					ss << m_keys.at(tolower(text[i]));
				if (i != text.size() - 1)
					ss << ' ';
			}
			return ss.str();
		}

		// morse to text
		std::string decode(const std::string& morse) const
		{
			std::stringstream ss;
			auto parsed_text = detail::separate_string(morse);
			if (parsed_text.size() == 0)
				return std::string{};
			for (auto& item : parsed_text)
			{
#if _HAS_CXX17
				for (auto& [key, value] : m_keys)
				{
#else // _HAS_CXX17
				for (auto& it : m_keys)
				{
					auto& key = it.first;
					auto& value = it.second;
#endif // _HAS_CXX17
					if (value == item)
					{
						ss << key;
						break;
					}
				}
				}
			return ss.str();
			}

		std::string encode_file(const std::string& file) const
		{
			std::ifstream istr(file);
			std::stringstream ss;
			if (istr.is_open())
			{
				ss << istr.rdbuf();
				istr.close();
			}
			return encode(ss.str());
		}

		std::string decode_file(const std::string& file) const
		{
			std::ifstream istr(file);
			std::stringstream ss;
			if (istr.is_open())
			{
				ss << istr.rdbuf();
				istr.close();
			}
			return decode(ss.str());
		}

		std::string default_to_member(const std::string& morse_text) const
		{
			return switch_format(morse_text, default_format, m_format);
		}

		std::string member_to_default(const std::string& morse_text) const
		{
			return switch_format(morse_text, m_format, default_format);
		}

		std::string switch_format_to_member(const std::string& morse_text, morse_format fmt) const
		{
			return switch_format(morse_text, fmt, m_format);
		}

		std::string switch_format_from_member(const std::string& morse_text, morse_format fmt) const
		{
			return switch_format(morse_text, m_format, fmt);
		}

		std::string switch_format_to_member(const std::string& morse_text, char lpk, char spk, char sk) const
		{
			return switch_format(morse_text, { lpk, spk, sk }, m_format);
		}

		std::string switch_format_from_member(const std::string& morse_text, char lpk, char spk, char sk) const
		{
			return switch_format(morse_text, m_format, { lpk, spk, sk });
		}

		static std::string switch_format(const std::string& morse_text, morse_format old_fmt, morse_format new_fmt)
		{
			if (old_fmt == new_fmt)
				return morse_text;
			std::stringstream ss;
			for (char item : morse_text)
			{
				if (item == old_fmt.long_press)
					ss << new_fmt.long_press;
				else if (item == old_fmt.short_press)
					ss << new_fmt.short_press;
				else if (item == old_fmt.space)
					ss << new_fmt.space;
				else
					ss << item; // I believe this is the space. If not, the morse_text parameter is incorrect. Output also returns with the same error in the same place. Doesn't bother to repair. If you want to repair it, use repair_morse.
			}
			return ss.str();
		}

		static std::string switch_format(const std::string& morse_text, char old_fmt_lpk, char old_fmt_spk, char old_fmt_sk, char new_fmt_lpk, char new_fmt_spk, char new_fmt_sk)
		{
			if (old_fmt_lpk == new_fmt_lpk && old_fmt_spk == new_fmt_spk && old_fmt_sk == new_fmt_sk)
				return morse_text;
			std::stringstream ss;
			for (char item : morse_text)
			{
				if (item == old_fmt_lpk)
					ss << new_fmt_lpk;
				else if (item == old_fmt_spk)
					ss << new_fmt_spk;
				else if (item == old_fmt_sk)
					ss << new_fmt_sk;
				else
					ss << item;
			}
			return ss.str();
		}

		/*
		* @brief The remove_incorrect_letter and try_ordered_repair_list_one_by_one options are ignored in this list. 
		* Other repair modes can be placed in any order. The user is free to choose which repair modes to include.
		* 
		*	Default order:
		*	1- remove_incorrect_key
		*	2- try_replacing_with_short_press
		*	3- try_replacing_with_long_press
		*/
		static void set_repair_order(std::initializer_list<repair_mode> new_order)
		{
			std::unordered_set<repair_mode> seen;

			s_repair_order.clear();
			s_repair_order.reserve(new_order.size());

			for (const auto& elem : new_order) 
				if (seen.insert(elem).second) 
					s_repair_order.push_back(elem);
		}

		static std::string repair_morse(const std::string& morse_text, repair_mode mode = repair_mode::default_mode, morse_format fmt = default_format)
		{
			auto parsed_text = detail::separate_string(morse_text);
			if (parsed_text.empty())
				return std::string();

			auto key_list = get_valid_morse_list(fmt);

			std::stringstream ss;

			auto add_to_stream = [&](const std::string& letter, bool add_space)
				{
					ss << letter;
					if (add_space)
						ss << ' ';
				};

			for (size_t i = 0; i < parsed_text.size(); ++i)
			{
				if (std::find(key_list.begin(), key_list.end(), parsed_text[i]) == key_list.end())
				{
					switch (mode)
					{
					case thug::repair_mode::remove_incorrect_letter:
					{
						break; // Just skip adding letters process.
					}
					case thug::repair_mode::remove_incorrect_key:
					{
						std::string fixed_letter = parsed_text[i];

						// Let's remove irrelevant keys.
						fixed_letter.erase(std::remove_if(fixed_letter.begin(), fixed_letter.end(),
							[fmt](char c) {
								return !fmt.is_key(c);
							}),
							fixed_letter.end());

						if (std::find(key_list.begin(), key_list.end(), fixed_letter) == key_list.end())
						{
							break; // Still wrong, let's remove the letter.
						}
						else // The letter has been repaired. Let's add it.
						{
							add_to_stream(fixed_letter, i < parsed_text.size() - 1);
						}
						break;
					}
					case thug::repair_mode::try_replacing_with_short_press:
					{
						std::string fixed_letter = parsed_text[i];

						// Let's replace the incorrect keys with short_press key
						for (auto& key : fixed_letter)
							if (!fmt.is_key(key))
								key = fmt.short_press;

						if (std::find(key_list.begin(), key_list.end(), fixed_letter) == key_list.end())
						{
							break; // Still wrong, let's remove the letter.
						}
						else // The letter has been repaired. Let's add it.
						{
							add_to_stream(fixed_letter, i < parsed_text.size() - 1);
						}
						break;
					}
					case thug::repair_mode::try_replacing_with_long_press:
					{
						std::string fixed_letter = parsed_text[i];

						// Let's replace the incorrect keys with long_press key
						for (auto& key : fixed_letter)
							if (!fmt.is_key(key))
								key = fmt.long_press;

						if (std::find(key_list.begin(), key_list.end(), fixed_letter) == key_list.end())
						{
							break; // Still wrong, let's remove the letter.
						}
						else // The letter has been repaired. Let's add it.
						{
							add_to_stream(fixed_letter, i < parsed_text.size() - 1);
						}
						break;
					}
					case thug::repair_mode::try_ordered_repair_list_one_by_one:
					{
						bool exit_loop = false;
						for (auto& rm : s_repair_order)
						{
							if (exit_loop)
								break;

							switch (rm)
							{
							case thug::repair_mode::remove_incorrect_key:
							{
								std::string fixed_letter = parsed_text[i];

								// Let's remove irrelevant keys.
								fixed_letter.erase(std::remove_if(fixed_letter.begin(), fixed_letter.end(),
									[fmt](char c) {
										return !fmt.is_key(c);
									}),
									fixed_letter.end());

								if (std::find(key_list.begin(), key_list.end(), fixed_letter) == key_list.end())
								{
									break; // Still wrong, next mode.
								}
								else // The letter has been repaired. Let's add it.
								{
									add_to_stream(fixed_letter, i < parsed_text.size() - 1);
									// we should exit the loop
									exit_loop = true;
								}
								break;
							}
							case thug::repair_mode::try_replacing_with_short_press:
							{
								std::string fixed_letter = parsed_text[i];

								// Let's replace the incorrect keys with short_press key
								for (auto& key : fixed_letter)
									if (!fmt.is_key(key))
										key = fmt.short_press;

								if (std::find(key_list.begin(), key_list.end(), fixed_letter) == key_list.end())
								{
									break; // Still wrong, next mode.
								}
								else // The letter has been repaired. Let's add it.
								{
									add_to_stream(fixed_letter, i < parsed_text.size() - 1);
									// we should exit the loop
									exit_loop = true;
								}
								break;
							}
							case thug::repair_mode::try_replacing_with_long_press:
							{
								std::string fixed_letter = parsed_text[i];

								// Let's replace the incorrect keys with long_press key
								for (auto& key : fixed_letter)
									if (!fmt.is_key(key))
										key = fmt.long_press;

								if (std::find(key_list.begin(), key_list.end(), fixed_letter) == key_list.end())
								{
									break; // Still wrong, next mode.
								}
								else // The letter has been repaired. Let's add it.
								{
									add_to_stream(fixed_letter, i < parsed_text.size() - 1);
									// we should exit the loop
									exit_loop = true;
								}
								break;
							}
							default:
								break;
							}
						}
						break;
					}
					default:
						break; // Same as remove_incorrect_letter, just skip adding letters process.
					}

				}
				else
				{
					add_to_stream(parsed_text[i], i < parsed_text.size() - 1);
				}
			}

			return ss.str();
		}

		static bool is_valid_morse(const std::string& morse_text, morse_format fmt = default_format)
		{
			auto parsed_text = detail::separate_string(morse_text);
			if (parsed_text.empty())
				return true;

			auto key_list = get_valid_morse_list(fmt);

			for (auto& letter : parsed_text)
				if (std::find(key_list.begin(), key_list.end(), letter) == key_list.end())
					return false;

			return true;
		}
	private:
		static std::unordered_map<char, std::string> get_key_table(morse_format fmt)
		{
			std::unordered_map<char, std::string> result;

			result['a'] = switch_format(".-", default_format, fmt);
			result['b'] = switch_format("-...", default_format, fmt);
			result['c'] = switch_format("-.-.", default_format, fmt);
			result['d'] = switch_format("-..", default_format, fmt);
			result['e'] = switch_format(".", default_format, fmt);
			result['f'] = switch_format("..-.", default_format, fmt);
			result['g'] = switch_format("--.", default_format, fmt);
			result['h'] = switch_format("....", default_format, fmt);
			result['i'] = switch_format("..", default_format, fmt);
			result['j'] = switch_format(".---", default_format, fmt);
			result['k'] = switch_format("-.-", default_format, fmt);
			result['l'] = switch_format(".-..", default_format, fmt);
			result['m'] = switch_format("--", default_format, fmt);
			result['n'] = switch_format("-.", default_format, fmt);
			result['o'] = switch_format("---", default_format, fmt);
			result['p'] = switch_format(".--.", default_format, fmt);
			result['q'] = switch_format("--.-", default_format, fmt);
			result['r'] = switch_format(".-.", default_format, fmt);
			result['s'] = switch_format("...", default_format, fmt);
			result['t'] = switch_format("-", default_format, fmt);
			result['u'] = switch_format("..-", default_format, fmt);
			result['v'] = switch_format("...-", default_format, fmt);
			result['w'] = switch_format(".--", default_format, fmt);
			result['x'] = switch_format("-..-", default_format, fmt);
			result['y'] = switch_format("-.--", default_format, fmt);
			result['z'] = switch_format("--..", default_format, fmt);
			result['0'] = switch_format("-----", default_format, fmt);
			result['1'] = switch_format(".----", default_format, fmt);
			result['2'] = switch_format("..---", default_format, fmt);
			result['3'] = switch_format("...--", default_format, fmt);
			result['4'] = switch_format("....-", default_format, fmt);
			result['5'] = switch_format(".....", default_format, fmt);
			result['6'] = switch_format("-....", default_format, fmt);
			result['7'] = switch_format("--...", default_format, fmt);
			result['8'] = switch_format("---..", default_format, fmt);
			result['9'] = switch_format("----.", default_format, fmt);
			result['.'] = switch_format(".-.-.-", default_format, fmt);
			result[','] = switch_format("--..--", default_format, fmt);
			result['?'] = switch_format("..--..", default_format, fmt);
			result['/'] = switch_format("-..-.", default_format, fmt);
			result['('] = switch_format("-.--.", default_format, fmt);
			result[')'] = switch_format("-.--.-", default_format, fmt);
			result[':'] = switch_format("---...", default_format, fmt);
			result['='] = switch_format("-...-", default_format, fmt);
			result['+'] = switch_format(".-.-.", default_format, fmt);
			result['-'] = switch_format("-....-", default_format, fmt);
			result['@'] = switch_format(".--.-.", default_format, fmt);
			result['\''] = switch_format(".----.", default_format, fmt);
			result['\"'] = switch_format(".-..-.", default_format, fmt);
			result['\\'] = switch_format("-..-.", default_format, fmt);

			result[' '] = fmt.space;

			// these are nonstandart
			result['!'] = switch_format("-.-.--", default_format, fmt);
			result['&'] = switch_format(".-...", default_format, fmt);
			result[';'] = switch_format("-.-.-.", default_format, fmt);
			result['_'] = switch_format("..--.-", default_format, fmt);
			result['$'] = switch_format("...-..-", default_format, fmt);
			
			return result;
		}

		static std::vector<std::string> get_valid_morse_list(morse_format fmt)
		{
			std::vector<std::string> result;

			result.reserve(56); // sizeof letters

			result.push_back(switch_format(".-", default_format, fmt));
			result.push_back(switch_format("-...", default_format, fmt));
			result.push_back(switch_format("-.-.", default_format, fmt));
			result.push_back(switch_format("-..", default_format, fmt));
			result.push_back(switch_format(".", default_format, fmt));
			result.push_back(switch_format("..-.", default_format, fmt));
			result.push_back(switch_format("--.", default_format, fmt));
			result.push_back(switch_format("....", default_format, fmt));
			result.push_back(switch_format("..", default_format, fmt));
			result.push_back(switch_format(".---", default_format, fmt));
			result.push_back(switch_format("-.-", default_format, fmt));
			result.push_back(switch_format(".-..", default_format, fmt));
			result.push_back(switch_format("--", default_format, fmt));
			result.push_back(switch_format("-.", default_format, fmt));
			result.push_back(switch_format("---", default_format, fmt));
			result.push_back(switch_format(".--.", default_format, fmt));
			result.push_back(switch_format("--.-", default_format, fmt));
			result.push_back(switch_format(".-.", default_format, fmt));
			result.push_back(switch_format("...", default_format, fmt));
			result.push_back(switch_format("-", default_format, fmt));
			result.push_back(switch_format("..-", default_format, fmt));
			result.push_back(switch_format("...-", default_format, fmt));
			result.push_back(switch_format(".--", default_format, fmt));
			result.push_back(switch_format("-..-", default_format, fmt));
			result.push_back(switch_format("-.--", default_format, fmt));
			result.push_back(switch_format("--..", default_format, fmt));
			result.push_back(switch_format("-----", default_format, fmt));
			result.push_back(switch_format(".----", default_format, fmt));
			result.push_back(switch_format("..---", default_format, fmt));
			result.push_back(switch_format("...--", default_format, fmt));
			result.push_back(switch_format("....-", default_format, fmt));
			result.push_back(switch_format(".....", default_format, fmt));
			result.push_back(switch_format("-....", default_format, fmt));
			result.push_back(switch_format("--...", default_format, fmt));
			result.push_back(switch_format("---..", default_format, fmt));
			result.push_back(switch_format("----.", default_format, fmt));
			result.push_back(switch_format(".-.-.-", default_format, fmt));
			result.push_back(switch_format("--..--", default_format, fmt));
			result.push_back(switch_format("..--..", default_format, fmt));
			result.push_back(switch_format("-..-.", default_format, fmt));
			result.push_back(switch_format("-.--.", default_format, fmt));
			result.push_back(switch_format("-.--.-", default_format, fmt));
			result.push_back(switch_format("---...", default_format, fmt));
			result.push_back(switch_format("-...-", default_format, fmt));
			result.push_back(switch_format(".-.-.", default_format, fmt));
			result.push_back(switch_format("-....-", default_format, fmt));
			result.push_back(switch_format(".--.-.", default_format, fmt));
			result.push_back(switch_format(".----.", default_format, fmt));
			result.push_back(switch_format(".-..-.", default_format, fmt));
			result.push_back(switch_format("-..-.", default_format, fmt));

			result.push_back(std::string(1, fmt.space));

			// these are nonstandart
			result.push_back(switch_format("-.-.--", default_format, fmt));
			result.push_back(switch_format(".-...", default_format, fmt));
			result.push_back(switch_format("-.-.-.", default_format, fmt));
			result.push_back(switch_format("..--.-", default_format, fmt));
			result.push_back(switch_format("...-..-", default_format, fmt));

			return result;
		}

		static std::vector<repair_mode> s_repair_order;
		std::unordered_map<char, std::string> m_keys;
		morse_format m_format;
		};

		std::vector<repair_mode> morse_converter::s_repair_order = 
		{ 
			repair_mode::remove_incorrect_key,
			repair_mode::try_replacing_with_short_press,
			repair_mode::try_replacing_with_long_press,
		};
	}
