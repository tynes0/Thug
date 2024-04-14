# Thug Morse Converter

Thug is a "header only" project that allows you to do morse conversion, encryption and similar operations.

### Usage

First of all, lets create a converter

```cpp
thug::morse_converter converter;
// or
thug::morse_converter converter(thug::default_format);
// or you can create with format u want
thug::morse_format fmt('1', '0', '2');
thug::morse_converter converter(fmt);
// or 
thug::morse_converter converter('1', '0', '2');
```

And you can use like this

```cpp
std::cout << converter.encode("Hello, World!");
//^^ Output: .... . .-.. .-.. --- --..-- / .-- --- .-. .-.. -.. -.-.-- ^^
std::cout << converter.decode(".... . .-.. .-.. --- --..-- / .-- --- .-. .-.. -.. -.-.--");
//^^ Output: hello, world! ^^
```

If you want, you can use any format you want instead of dots, dashes and slashes.

```cpp
thug::morse_format fmt('1', '0', '2');
converter.set_format(fmt);
// or you can use converter.set_format('1', '0', '2');
std::cout << converter.encode("Hello, World!");
//^^ Output: 0000 0 0100 0100 111 110011 2 011 111 010 0100 100 101011 ^^
std::cout << converter.decode("0000 0 0100 0100 111 110011 2 011 111 010 0100 100 101011");
//^^ Output: hello, world! ^^
```

You can also perform the same operations by transferring data from a file.

```cpp
// The content of foo.txt file is "Hello, World!"
std::cout << converter.encode_file("foo.txt"); // Output: .... . .-.. .-.. --- --..-- / .-- --- .-. .-.. -.. -.-.--
// ...decode_file works the same way...
```

You can check if a morse text is incorrect. (Checks whether there are non-format characters in the text)

```cpp
bool test1 = decoder.is_valid_morse(".... . .-.. .-.. --- --..-- / .-- --- .-. .-.. -.. -.-.--"); // test1 == true
bool test2 = decoder.is_valid_morse(".... . .-.. .-.. --- a--..-- / .-- --- .-. .-.. -.. -.-.--"); // test2 == false
bool test3 = decoder.is_valid_morse("0000 0 0100 0100 111 110011 2 011 111 010 0100 100 101011", { 1 , 0, 2 }); // test3 == true
```

If there is an error in a morse text, you can remove the incorrect part with the repair_morse method.

```cpp
std::cout << decoder.repair_morse(".... . .-.. .-.. --- a--..-- / .-- --- .-. .-.. -.. -.-.--");
// ^^ Output: .... . .-.. .-.. --- --..-- / .-- --- .-. .-.. -.. -.-.-- ^^
std::cout << decoder.repair_morse("0000 0 0100 0100 111 110011 2 011 111 010 0100 XXX100 101011", { 1, 0, 2 });
// ^^ Output: 0000 0 0100 0100 111 110011 2 011 111 010 0100 100 101011 ^^
```

If you want to encrypt morse message, you can use encrypt and decrypt methods

```cpp
// Note that: default encrypt key is 'thug'
auto bar = decoder.encrypt("Hello, world!"); // bar = ZF[ITFUIYF[GZE[ITEXJTEXIZEXG[H[JYHXJYH[JZH[JZFUJZFUJZE[JY
// same as this
std::cout << decoder.encrypt(".... . .-.. .-.. --- --..-- / .-- --- .-. .-.. -.. -.-.--"); // output: ZF[ITFUIYF[GZE[ITEXJTEXIZEXG[H[JYHXJYH[JZH[JZFUJZFUJZE[JY
std::cout << decoder.decrypt(bar); // Output: hello, world!
```

You can change the encryption key if you want

```cpp
decoder.set_encryption_key("the_key_you_want");
// perform encrypt - decrypt operations
```

Or you can directly use an external key

```cpp
auto bar = decoder.encrypt("Hello, world!", "the_key_you_want");
decoder.decrypt(bar, "the_key_you_want");
```

Finally, you can use switch_format methods to switch between formats.

```cpp
std::cout << decoder.switch_format(".... . .-.. .-.. --- --..-- / .-- --- .-. .-.. -.. -.-.--", default_format, { '1', '0', '2' });
// ^^ Output: 0000 0 0100 0100 111 110011 2 011 111 010 0100 100 101011 ^^
// similar methods exhibit similar functionality: this example is the same as switch_format_from_member(".... . .-.. .-.. --- --..-- / .-- --- .-. .-.. -.. -.-.--", { '1', '0', '2' });
```
