# Morse Converter Library

A lightweight C++17+ header-only library for converting between **text** and **Morse code**, with support for custom formats, file-based encoding/decoding, error repair, and flexible configuration.  

---

## ✨ Features
- **Text → Morse** and **Morse → Text** conversion  
- **Customizable formats** (choose your own symbols for `long press`, `short press`, and `space`)  
- **File encoding/decoding** (read/write Morse directly from files)  
- **Format switching** (convert Morse texts between different formats)  
- **Error detection and repair** strategies for corrupted Morse strings  
- **Validation utilities** to check if a given Morse string is valid  
- Header-only, **no external dependencies**  

---

## 📦 Installation
Simply include the header file in your project:

    #include "morse_converter.h"

The library requires **C++17 or later**.

---

## 🔧 Basic Usage

### Creating a Converter

    #include "morse_converter.h"
    #include <iostream>

    int main() {
        thug::morse_converter converter;

        std::string text = "hello world";
        std::string morse = converter.encode(text);

        std::cout << "Text: " << text << "\n";
        std::cout << "Morse: " << morse << "\n";

        std::string decoded = converter.decode(morse);
        std::cout << "Decoded: " << decoded << "\n";
    }

Output:

    Text: hello world
    Morse: .... . .-.. .-.. --- / .-- --- .-. .-.. -..
    Decoded: hello world

---

## 🎨 Custom Formats

By default:
- `long press = '-'`
- `short press = '.'`
- `space = '/'`

You can customize them:

    thug::morse_converter converter('*', '_', '|');

    std::string text = "sos";
    std::string morse = converter.encode(text);
    // morse = "***|___|***"

Or use the `morse_format` struct:

    thug::morse_format fmt{ '=', '.', ' ' };
    thug::morse_converter converter(fmt);

---

## 📂 File Encoding/Decoding

Encode a file:

    std::string morse_file = converter.encode_file("input.txt");

Decode a file:

    std::string text_file = converter.decode_file("encoded_morse.txt");

---

## 🔄 Format Switching

Convert Morse between formats:

    std::string original = "... --- ..."; // default format

    // switch to a custom format
    std::string custom = converter.switch_format_from_member(original, '*', '_', '|');

    // switch back to default
    std::string back = converter.switch_format_to_member(custom, '*', '_', '|');

---

## 🛠 Error Repair

When Morse contains invalid characters, you can **repair** it using multiple strategies:

### Modes
- `remove_incorrect_letter` → Removes the entire invalid letter  
- `remove_incorrect_key` → Strips invalid keys from the letter  
- `try_replacing_with_short_press` → Replace invalid keys with `short_press`  
- `try_replacing_with_long_press` → Replace invalid keys with `long_press`  
- `try_ordered_repair_list_one_by_one` → Try multiple repair strategies in order  

Example:

    std::string broken_morse = "...#-- ..@.";

    std::string repaired = thug::morse_converter::repair_morse(
        broken_morse,
        thug::repair_mode::remove_incorrect_key
    );

### Custom Repair Order
You can define which repair strategies should be tried first:

    thug::morse_converter::set_repair_order({
        thug::repair_mode::try_replacing_with_short_press,
        thug::repair_mode::remove_incorrect_key
    });

Then use `try_ordered_repair_list_one_by_one`.

---

## ✅ Validation
Check if a string is valid Morse:

    bool ok = thug::morse_converter::is_valid_morse(".... . .-.. .-.. ---");
    if(ok) {
        std::cout << "Valid Morse!" << std::endl;
    }

---

## 📖 API Overview

### `struct morse_format`
- `char long_press` → Default: `'-'`  
- `char short_press` → Default: `'.'`  
- `char space` → Default: `'/'`  
- `bool is_key(char)` → Checks if character is a valid Morse key  

---

### `class morse_converter`
#### Constructors
- `morse_converter(char long_press, char short_press, char space='/')`  
- `morse_converter(morse_format fmt)`  
- Copy/move constructors supported  

#### Encoding/Decoding
- `std::string encode(const std::string& text)`  
- `std::string decode(const std::string& morse)`  
- `std::string encode_file(const std::string& file)`  
- `std::string decode_file(const std::string& file)`  

#### Format Conversion
- `std::string default_to_member(const std::string& morse_text)`  
- `std::string member_to_default(const std::string& morse_text)`  
- `std::string switch_format_to_member(const std::string& morse_text, morse_format fmt)`  
- `std::string switch_format_from_member(const std::string& morse_text, morse_format fmt)`  

#### Repair & Validation
- `static std::string repair_morse(const std::string& morse_text, repair_mode mode, morse_format fmt=default_format)`  
- `static bool is_valid_morse(const std::string& morse_text, morse_format fmt=default_format)`  
- `static void set_repair_order(std::initializer_list<repair_mode> order)`  

---

## 🚀 Example with Repair

    std::string broken = "..#.- / ....&";

    // repair by replacing with short press
    std::string repaired = thug::morse_converter::repair_morse(
        broken,
        thug::repair_mode::try_replacing_with_short_press
    );

    std::string decoded = converter.decode(repaired);

    std::cout << "Repaired Morse: " << repaired << "\n";
    std::cout << "Decoded Text: " << decoded << "\n";

---

## ⚖️ License
This project is licensed under the MIT License.  
Feel free to use, modify, and distribute in personal or commercial projects.
