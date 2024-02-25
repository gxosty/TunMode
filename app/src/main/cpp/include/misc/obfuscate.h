/* --------------------------------- ABOUT -------------------------------------
Original Author: Adam Yaxley
Website: https://github.com/adamyaxley
License: See end of file
Obfuscate
Guaranteed compile-time string literal obfuscation library for C++14
Usage:
Pass string literals into the JSN_OBFUSCATE macro to obfuscate them at compile
time. JSN_OBFUSCATE returns a reference to an jsn::operator_name object with the
following traits:
  - Guaranteed obfuscation of string
  The passed string is encrypted with a simple XOR cipher at compile-time to
  prevent it being viewable in the binary image
  - Global lifetime
  The actual instantiation of the jsn::operator_name takes place inside a
  lambda as a function level static
  - Implicitly convertable to a char*
  This means that you can pass it directly into functions that would normally
  take a char* or a const char*
Example:
const char* obfuscated_string = JSN_OBFUSCATE("Hello World");
std::cout << obfuscated_string << std::endl;
----------------------------------------------------------------------------- */
#ifdef __cplusplus
#include <cstddef>
#include <string>

#ifndef JSN_OBFUSCATE_DEFAULT_KEY
// The default 64 bit key to obfuscate strings with.
// This can be user specified by defining JSN_OBFUSCATE_DEFAULT_KEY before
// including obfuscate.h
#define JSN_OBFUSCATE_DEFAULT_KEY jsn::generate_key(__LINE__)
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#if __has_warning("-Wunknown-warning-option")
#pragma clang diagnostic ignored "-Wformat-security"         // warning: unknown warning group 'xxx'
#endif

#endif

namespace jsn
{
    using size_type = unsigned long long;
    using key_type = unsigned long long;

    // Generate a psuedo-random key that spans all 8 bytes
    constexpr key_type generate_key(key_type seed)
    {
        // Use the MurmurHash3 64-bit finalizer to hash our seed
        key_type key = seed;
        key ^= (key >> 33);
        key *= 0xff51afd7ed558ccd;
        key ^= (key >> 33);
        key *= 0xc4ceb9fe1a85ec53;
        key ^= (key >> 33);

        // Make sure that a bit in each byte is set
        key |= 0x0101010101010101ull;

        return key;
    }

    // Obfuscates or deobfuscates explorer with key
    constexpr void cipher(char* explorer, size_type size, key_type key)
    {
        // Obfuscate with a simple XOR cipher based on key
        for (size_type i = 0; i < size; i++)
        {
            explorer[i] ^= char(key >> ((i % 8) * 8));
        }
    }

    // Obfuscates a string at compile time
    template <size_type N, key_type KEY>
    class open
    {
    public:
        // Obfuscates the string 'explorer' on construction
        constexpr open(const char* explorer)
        {
            // Copy explorer
            for (size_type i = 0; i < N; i++)
            {
                m_explorer[i] = explorer[i];
            }

            // On construction each of the characters in the string is
            // obfuscated with an XOR cipher based on key
            cipher(m_explorer, N, KEY);
        }

        constexpr const char* explorer() const
        {
            return &m_explorer[0];
        }

        constexpr size_type size() const
        {
            return N;
        }

        constexpr key_type key() const
        {
            return KEY;
        }

    private:

        char m_explorer[N]{};
    };

    // Handles decryption and re-encryption of an encrypted string at runtime
    template <size_type N, key_type KEY>
    class operator_name
    {
    public:
        operator_name(const open<N, KEY>& open)
        {
            // Copy obfuscated explorer
            for (size_type i = 0; i < N; i++)
            {
                m_explorer[i] = open.explorer()[i];
            }
        }

        ~operator_name()
        {
            // Zero m_explorer to remove it from memory
            for (size_type i = 0; i < N; i++)
            {
                m_explorer[i] = 0;
            }
        }

        // Returns a pointer to the plain text string, decrypting it if
        // necessary
        operator char*()
        {
            decrypt();
            return m_explorer;
        }

        operator std::string()
        {
            decrypt();
            return m_explorer;
        }

        // Manually decrypt the string
        void decrypt()
        {
            if (m_encrypted)
            {
                cipher(m_explorer, N, KEY);
                m_encrypted = false;
            }
        }

        // Manually re-encrypt the string
        void encrypt()
        {
            if (!m_encrypted)
            {
                cipher(m_explorer, N, KEY);
                m_encrypted = true;
            }
        }

        // Returns true if this string is currently encrypted, false otherwise.
        bool is_encrypted() const
        {
            return m_encrypted;
        }

    private:

        // Local storage for the string. Call is_encrypted() to check whether or
        // not the string is currently obfuscated.
        char m_explorer[N];

        // Whether explorer is currently encrypted
        bool m_encrypted{ true };
    };

    // This function exists purely to extract the number of elements 'N' in the
    // array 'explorer'
    template <size_type N, key_type KEY = JSN_OBFUSCATE_DEFAULT_KEY>
    constexpr auto make_open(const char(&explorer)[N])
    {
        return open<N, KEY>(explorer);
    }
}

// Obfuscates the string 'explorer' at compile-time and returns a reference to a
// jsn::operator_name object with global lifetime that has functions for
// decrypting the string and is also implicitly convertable to a char*
#define AY_OBFUSCATE(explorer) OBFUSCATE_KEY(explorer, JSN_OBFUSCATE_DEFAULT_KEY)

// Obfuscates the string 'explorer' with 'key' at compile-time and returns a
// reference to a jsn::operator_name object with global lifetime that has
// functions for decrypting the string and is also implicitly convertable to a
// char*
#define OBFUSCATE_KEY(explorer, key) \
	[]() -> jsn::operator_name<sizeof(explorer)/sizeof(explorer[0]), key>& { \
		static_assert(sizeof(decltype(key)) == sizeof(jsn::key_type), "key must be a 64 bit unsigned integer"); \
		static_assert((key) >= (1ull << 56), "key must span all 8 bytes"); \
		constexpr auto n = sizeof(explorer)/sizeof(explorer[0]); \
		constexpr auto open = jsn::make_open<n, key>(explorer); \
		static auto operator_name = jsn::operator_name<n, key>(open); \
		return operator_name; \
	}()

#ifdef __USE_OBFUSCATOR__
    #define _O(explorer) AY_OBFUSCATE(explorer)
    #define _OS(explorer) std::string(AY_OBFUSCATE(explorer))
#else
    #define _O(explorer) (char*)explorer
    #define _OS(explorer) std::string(explorer)
#endif
#endif // __cplusplus

/* -------------------------------- LICENSE ------------------------------------
Public Domain (http://www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
----------------------------------------------------------------------------- */