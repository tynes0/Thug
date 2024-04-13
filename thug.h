#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>

namespace thug
{
	namespace detail
	{
		constexpr const char* default_encryption_key = "thug"; // change this if you want.

		std::vector<std::string> separate_string(const std::string& source, char token)
		{
			std::string temp = "";
			std::vector<std::string> result;

			for (size_t i = 0; i < source.size(); i++)
			{
				if (source[i] != token)
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

		std::string xor_encrypt_decrypt(const std::string& plain_text, const std::string& key)
		{
			std::string cipher_text;
			for (size_t i = 0; i < plain_text.size(); ++i)
				cipher_text += (plain_text[i] ^ key[i % key.size()]);
			return cipher_text;
		}
	}

	struct morse_format
	{
		char long_press		= 0;
		char short_press	= 0;
		char space			= 0;

		bool operator==(morse_format right) const noexcept
		{
			return (long_press == right.long_press && short_press == right.short_press && space == right.space);
		}
	};

	constexpr morse_format default_format = { '-', '.', '/' };

	class morse_converter
	{
	public:
		morse_converter(char long_press_key = default_format.long_press, char short_press_key = default_format.short_press, char space_key = default_format.space) : m_format({ long_press_key, short_press_key, space_key })
		{
			reset_key_table();
		}

		morse_converter(morse_format fmt) : m_format(fmt)
		{
			reset_key_table();
		}

		morse_converter(const morse_converter& other) : m_format(other.m_format)
		{
			reset_key_table();
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
			reset_key_table();
		}

		void set_format(morse_format fmt)
		{
			auto temp = m_format;
			m_format = fmt;
			reset_key_table();
		}

		// If you change the key after encrypting any data, you will get an error during decryption. Be careful! 
		// If you don't want to get an error, call the encrypyt_decrypyt method overload with the key parameter. And use the key you used for encryption.
		// NOTE THAT: Empty key causes an error.
		void set_encryption_key(const std::string& key)
		{
			m_encryption_key = key;
		}

		std::string get_encryption_key() const noexcept
		{
			return m_encryption_key;
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
			auto parsed_text = detail::separate_string(morse, ' ');
			if (parsed_text.size() == 0)
				return std::string{};
			for (auto& item : parsed_text)
			{
				for (auto& [key, value] : m_keys)
				{
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

		// If it is plain text, encrypt it; if it is encrypted text, decrypt it.
		std::string encrypyt_decrypyt(const std::string& text) const
		{
			return detail::xor_encrypt_decrypt(text, m_encryption_key);
		}

		// If it is plain text, encrypt it; if it is encrypted text, decrypt it.
		static std::string encrypyt_decrypyt(const std::string& text, const std::string& encryption_key)
		{
			return detail::xor_encrypt_decrypt(text, encryption_key);
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

		static std::string repair_morse(const std::string& morse_text, morse_format fmt = default_format)
		{
			if (is_valid_morse(morse_text, fmt))
				return morse_text;
			std::stringstream ss;
			for (char item : morse_text)
			{
				if (item == fmt.long_press)
					ss << fmt.long_press;
				else if (item == fmt.short_press)
					ss << fmt.short_press;
				else if (item == fmt.space)
					ss << fmt.space;
				else if (isspace(item))
					ss << item;
			}
			return ss.str();
		}

		static bool is_valid_morse(const std::string& morse_text, morse_format fmt = default_format)
		{
			for (char item : morse_text)
				if (item != fmt.long_press && item != fmt.short_press && item != fmt.space && !isspace(item))
					return false;
			return true;
		}
	private:
		void reset_key_table()
		{
			m_keys['a'] = default_to_member(".-");
			m_keys['b'] = default_to_member("-...");
			m_keys['c'] = default_to_member("-.-.");
			m_keys['d'] = default_to_member("-..");
			m_keys['e'] = default_to_member(".");
			m_keys['f'] = default_to_member("..-.");
			m_keys['g'] = default_to_member("--.");
			m_keys['h'] = default_to_member("....");
			m_keys['i'] = default_to_member("..");
			m_keys['j'] = default_to_member(".---");
			m_keys['k'] = default_to_member("-.-");
			m_keys['l'] = default_to_member(".-..");
			m_keys['m'] = default_to_member("--");
			m_keys['n'] = default_to_member("-.");
			m_keys['o'] = default_to_member("---");
			m_keys['p'] = default_to_member(".--.");
			m_keys['q'] = default_to_member("--.-");
			m_keys['r'] = default_to_member(".-.");
			m_keys['s'] = default_to_member("...");
			m_keys['t'] = default_to_member("-");
			m_keys['u'] = default_to_member("..-");
			m_keys['v'] = default_to_member("...-");
			m_keys['w'] = default_to_member(".--");
			m_keys['x'] = default_to_member("-..-");
			m_keys['y'] = default_to_member("-.--");
			m_keys['z'] = default_to_member("--..");
			m_keys['0'] = default_to_member("-----");
			m_keys['1'] = default_to_member(".----");
			m_keys['2'] = default_to_member("..---");
			m_keys['3'] = default_to_member("...--");
			m_keys['4'] = default_to_member("....-");
			m_keys['5'] = default_to_member(".....");
			m_keys['6'] = default_to_member("-....");
			m_keys['7'] = default_to_member("--...");
			m_keys['8'] = default_to_member("---..");
			m_keys['9'] = default_to_member("----.");
			m_keys['.'] = default_to_member(".-.-.-");
			m_keys[','] = default_to_member("--..--");
			m_keys['?'] = default_to_member("..--..");
			m_keys['/'] = default_to_member("-..-.");
			m_keys['('] = default_to_member("-.--.");
			m_keys[')'] = default_to_member("-.--.-");
			m_keys[':'] = default_to_member("---...");
			m_keys['='] = default_to_member("-...-");
			m_keys['+'] = default_to_member(".-.-.");
			m_keys['-'] = default_to_member("-....-");
			m_keys['@'] = default_to_member(".--.-.");
			m_keys['\''] = default_to_member(".----.");
			m_keys['\"'] = default_to_member(".-..-.");
			m_keys['\\'] = default_to_member("-..-.");

			m_keys[' '] = m_format.space;

			// these are nonstandart
			m_keys['!'] = default_to_member("-.-.--");
			m_keys['&'] = default_to_member(".-...");
			m_keys[';'] = default_to_member("-.-.-.");
			m_keys['_'] = default_to_member("..--.-");
			m_keys['$'] = default_to_member("...-..-");
		}
		std::unordered_map<char, std::string> m_keys;
		morse_format m_format;
		std::string m_encryption_key = detail::default_encryption_key;
	};

}
