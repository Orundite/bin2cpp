
#include <cstdint>
#include <format>
#include <string>
#include <chrono>
#include <random>
#include <span>
#include <vector>
#include <fstream>
#include <array>
#include <filesystem>
#include<iostream>
#include <cstring>

#ifdef _WIN32
#include <expected>
#include <Windows.h>
#include <cwctype>
#include <io.h>
#include <fcntl.h>
#endif

#ifdef _WIN32
using char_c = wchar_t;
#define cstrcmp std::wcscmp
#define ccerr std::wcerr
#define ccout std::wcout
#define cc(x) L##x
#define cstrtoll std::wcstoll
#else
using char_c = char;
#define cstrcmp std::strcmp
#define ccerr std::cerr
#define ccout std::cout
#define cc(x) x
#define cstrtoll std::strtoll
#endif

enum class standard_language : uint32_t
{
    //C >= 1'000'000
    min_C_zone = 1'000'000,
    //C++ >= 2'000'000
    min_CPP_zone = 2'000'000,
    //C < 2'000'000
    max_C_zone = min_CPP_zone-1,
    //C++ < 3'000'000
    max_CPP_zone = 3'000'000-1,

    //C

    ansi = min_C_zone,
    c99,
    c11,
    c17,
    c23,

    //C++

    cpp98 = min_CPP_zone,
    cpp03,
    cpp11,
    cpp14,
    cpp17,
    cpp20,
    cpp23,

    //safe zone
    min_C = ansi,
    max_C = c23,
    min_CPP = cpp98,
    max_CPP = cpp23
};

struct file_input{
    //указатель на аргумент
    const char_c* path_file;
    //Unix - указатель на аргумент
    //Win - выделенная память
    const char* name_array;
};
namespace parametrs
{
//указатель на аргумент
char_c* output;
std::vector<file_input> files;
standard_language standard = standard_language::c99;
bool enable_size = false;
bool enable_size_0 = false;
std::int64_t width = -1;
bool enable_pragma_once = false;
//Unix - указатель на аргумент
//Win - выделенная память
//nullptr - off, для C игнорируется
char* cpp_namespace = nullptr;
}
namespace detail
{
constexpr char on[]="ON";
constexpr char off[]="OFF";

void help()
{
    ccout
        << cc("Usage:\n")
        << cc("  program [options]\n\n")

        << cc("Options:\n")
        << cc("  -help                 Show this help.\n")
        << cc("  -fo <file>            Specify the output file.\n")
        << cc("  -fi <path> <name>     Add an input file.\n")
        << cc("  -std <standard>       Specify the language standard.\n")
        << cc("  -s                    Include size information.\n")
        << cc("  -s_0                  Include size information (0).\n")
        << cc("  -w <number>           Set the width.\n")
        << cc("  -po                   Add #pragma once.\n")
        << cc("  -cpp_ns <name>        Set the C++ namespace.\n\n")

        << cc("Standards:\n")
        << cc("  ANSI C99 C11 C17 C23\n")
        << cc("  C++98 C++03 C++11 C++14 C++17 C++20 C++23\n\n")

        << cc("Example:\n")
        << cc("  program -fi input.bin data -fo output.hpp -std C++20 -w 16 -po\n");

}

constexpr std::string_view to_string(standard_language v) {
    switch (v) {
    case standard_language::ansi:  return "ANSI";
    case standard_language::c99:   return "C99";
    case standard_language::c11:   return "C11";
    case standard_language::c17:   return "C17";
    case standard_language::c23:   return "C23";
    case standard_language::cpp98: return "C++98";
    case standard_language::cpp03: return "C++03";
    case standard_language::cpp11: return "C++11";
    case standard_language::cpp14: return "C++14";
    case standard_language::cpp17: return "C++17";
    case standard_language::cpp20: return "C++20";
    case standard_language::cpp23: return "C++23";
    default:    return "";
    }
}
constexpr bool is_std_c(standard_language std)
{
    return std>=standard_language::min_C && std<=standard_language::max_C;
}
constexpr bool is_std_cpp(standard_language std)
{
    return std>=standard_language::min_CPP && std<=standard_language::max_CPP;
}

const char* bool_str_onf(bool value)
{
    return value?on:off;
}
constexpr std::string rand_hex(std::size_t length)
{
    static constexpr char hex[] = "0123456789abcdef";

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(0, 15);

    std::string result;
    result.reserve(length);

    for (std::size_t i = 0; i < length; ++i)
        result += hex[dist(mt)];

    return result;
}

constexpr std::string get_guard()
{
    static const std::string guard_str={
        rand_hex(16)
    +
        std::format("{:%Y%m%d%H%M%S}",std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()))
};
return guard_str;
}
constexpr bool iequals(const char_c* a, const char_c* b)
{
    for (; *a && *b; ++a, ++b)
    {
#ifdef _WIN32
        if (std::towlower(*a) != std::towlower(*b))
            return false;
#else
        if (std::tolower(static_cast<unsigned char>(*a)) !=
            std::tolower(static_cast<unsigned char>(*b)))
            return false;
#endif
    }
    return *a == *b;
}

constexpr bool parse_std(const char_c* str, standard_language& result)
{
    struct entry
    {
        const char_c* name;
        standard_language value;
    };
    static const entry table[] = {
                                   {cc("ANSI"),  standard_language::ansi},
                                   {cc("C99"),   standard_language::c99},
                                   {cc("C11"),   standard_language::c11},
                                   {cc("C17"),   standard_language::c17},
                                   {cc("C23"),   standard_language::c23},
                                   {cc("C++98"), standard_language::cpp98},
                                   {cc("C++03"), standard_language::cpp03},
                                   {cc("C++11"), standard_language::cpp11},
                                   {cc("C++14"), standard_language::cpp14},
                                   {cc("C++17"), standard_language::cpp17},
                                   {cc("C++20"), standard_language::cpp20},
                                   {cc("C++23"), standard_language::cpp23},
                                   };
    for (const entry& e : table)
    {
        if (iequals(str, e.name))
        {
            result = e.value;
            return true;
        }
    }
    return false;
}
#ifdef _WIN32
std::expected<char*, std::wstring>  win_to_utf8(const wchar_t* wstr)
{
    if (wstr == nullptr)
        return nullptr;

    constexpr DWORD flags = WC_ERR_INVALID_CHARS;

    const int size = WideCharToMultiByte(
        CP_UTF8, flags, wstr, -1, nullptr, 0, nullptr, nullptr);

    if (size <= 0)
    {
        return std::unexpected(
            std::wstring(L"Error: failed to determine buffer size, error code ")
            + std::to_wstring(GetLastError()));
    }

    char* utf8 = new char[size];

    const int written = ::WideCharToMultiByte(
        CP_UTF8, flags, wstr, -1, utf8, size, nullptr, nullptr);

    if (written <= 0)
    {
        delete[] utf8;
        return std::unexpected(
            std::wstring(L"Error: UTF-8 conversion failed, error code ")
            + std::to_wstring(GetLastError()));
    }

    return utf8;
}
#endif
}
namespace stages
{

bool setting_parametrs(int arg_count,char_c* agrs[])
{
    using namespace parametrs;
    for (int i = 1; i < arg_count; ++i)
    {
        const char_c* arg = agrs[i];
        auto require = [&](int n) -> bool
        {
            if (i + n >= arg_count)
            {
                ccerr << cc("Error: parameter ") << arg << cc(" requires ")
                << n << cc(" arguments, ") << (arg_count - 1 - i) << cc(" provided\n");
                return false;
            }
            return true;
        };

        if (cstrcmp(arg, cc("-help")) == 0)
        {
            detail::help();
            return false;
        }
        else if (cstrcmp(arg, cc("-fo")) == 0)
        {
            if (!require(1)) return false;
            output = agrs[++i];
        }
        else if (cstrcmp(arg, cc("-fi")) == 0)
        {
            if (!require(2)) return false;
            const char_c* path = agrs[++i];
            const char_c* name = agrs[++i];

#ifdef _WIN32
            auto arr_name = detail::win_to_utf8(name);
            char* name_for_win;
            if (arr_name.has_value())
            {
                name_for_win = arr_name.value();
            } else {
                ccerr << arr_name.error() << '\n';
                return false;
            }
#endif
            for (const file_input& f : files)
            {

#ifdef _WIN32
                if (std::strcmp(f.name_array, name_for_win) == 0)
#else
                if (cstrcmp(f.name_array, name) == 0)
#endif
                {
                    ccerr << cc("Error: array name \"") << name << cc("\" is specified more than once\n");
#ifdef _WIN32
                    delete[] name_for_win;
#endif
                    return false;
                }
            }
#ifdef _WIN32
            files.push_back(file_input{path, name_for_win});
#else
            files.push_back(file_input{path, name});
#endif
        }
        else if (cstrcmp(arg, cc("-std")) == 0)
        {
            if (!require(1)) return false;
            ++i;
            if (!detail::parse_std(agrs[i], standard))
            {
                ccerr << cc("Error: unknown standard \"") << agrs[i]
                      << cc("\". Allowed: ANSI, C99, C11, C17, C23, ")
                    cc("C++98, C++03, C++11, C++14, C++17, C++20, C++23\n");
                return false;
            }
        }
        else if (cstrcmp(arg, cc("-s")) == 0)
        {
            enable_size = true;
        }
        else if (cstrcmp(arg, cc("-s_0")) == 0)
        {
            enable_size_0 = true;
        }
        else if (cstrcmp(arg, cc("-w")) == 0)
        {
            if (!require(1)) return false;
            ++i;
            char_c* end = nullptr;
            errno = 0;
            std::int64_t value = cstrtoll(agrs[i], &end, 10);
            if (end == agrs[i] || *end != cc('\0') || errno == ERANGE || value <= 0)
            {
                ccerr << cc("Error: -w requires a positive integer, got \"") << agrs[i] << cc("\"\n");
                return false;
            }
            width = static_cast<std::int64_t>(value);
        }
        else if (cstrcmp(arg, cc("-po")) == 0)
        {
            enable_pragma_once = true;
        }
        else if (cstrcmp(arg, cc("-cpp_ns")) == 0)
        {
            if (!require(1)) return false;

#ifdef _WIN32
            auto ns_name = detail::win_to_utf8(agrs[++i]);
            if (ns_name.has_value())
            {
                cpp_namespace = ns_name.value();
            } else {
                ccerr << ns_name.error() << '\n';
                return false;
            }
#else
            cpp_namespace = agrs[++i];
#endif

        }
        else
        {
            ccerr << cc("Error: unknown parameter \"") << arg << cc("\". Use -help\n");
            return false;
        }
    }

    // проверки после разбора (порядок ключей не важен)
    if (output == nullptr)
    {
        ccerr << cc("Error: output file (-fo) is not specified\n");
        return false;
    }
    if (files.empty())
    {
        ccerr << cc("Error: no input file (-fi) is specified\n");
        return false;
    }
    return true;
}

void include_guard_start(std::ofstream& stream)
{
    if(parametrs::enable_pragma_once)
    {
        stream<<"#pragma once\n\n";
    }
    else
    {
        stream<<"#ifndef BIN2CPP_"<<detail::get_guard()<<"_HGUARD\n#define BIN2CPP_"<<detail::get_guard()<<"_HGUARD\n\n";
    }
}
void info(std::ofstream& stream)
{
    if(parametrs::standard == standard_language::ansi)
        stream<<"/*\n";
    stream<<"/// -s "<<detail::bool_str_onf(parametrs::enable_size)
    <<"\n/// -s_0 "<<detail::bool_str_onf(parametrs::enable_size_0)
    <<"\n/// -w "<<((parametrs::width>0)?std::to_string(parametrs::width):detail::off)
    <<"\n/// -po "<<detail::bool_str_onf(parametrs::enable_pragma_once)
    <<"\n/// -cpp_ns "<<detail::bool_str_onf(parametrs::cpp_namespace!=nullptr)
    <<"\n/// Standard "<<detail::to_string(parametrs::standard)<<"\n";
    if(parametrs::standard == standard_language::ansi)
        stream<<"*/\n";
}
void includes(std::ofstream& stream)
{
    if(detail::is_std_c(parametrs::standard))
    {
        if(parametrs::standard>=standard_language::c99 ||
            (parametrs::enable_size||parametrs::enable_size_0)
            )
        {
            stream<<"\n#include <stddef.h>\n\n";
        }
    }
    else if(detail::is_std_cpp(parametrs::standard))
    {
        if(parametrs::standard>=standard_language::cpp11 ||
            (parametrs::enable_size||parametrs::enable_size_0)
            )
        {
            stream<<"\n#include <stddef>\n";
        }
        if(parametrs::standard>=standard_language::cpp17)
            stream<<"#include <array>\n";
        stream<<'\n';
    }
}
void namespace_start(std::ofstream& stream)
{
    if(detail::is_std_cpp(parametrs::standard) && parametrs::cpp_namespace)
    {
        stream<<"\nnamespace "<<parametrs::cpp_namespace<<"\n{\n";
    }
}

bool data(std::ofstream& stream, std::span<const file_input> data)
{
    std::array<uint8_t, 128> buffer;
    for(auto obj : data)
    {
        auto path = std::filesystem::path(obj.path_file);
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            ccerr << cc("Error: failed to open for reading ") << obj.path_file<<cc('\n');
            return false;
        }
        file.seekg(0, std::ios::end);
        size_t size_file = file.tellg();
        size_t remainder=size_file%parametrs::width;
        size_t bytes_width = (remainder==0)?0:(parametrs::width - remainder);
        file.seekg(0, std::ios::beg);
        if(parametrs::standard==standard_language::ansi)
            stream<<"static const unsigned char ";
        else if(parametrs::standard == standard_language::cpp98 ||
                 parametrs::standard == standard_language::cpp03)
            stream << "const unsigned char ";
        else if(parametrs::standard==standard_language::c99||
                 parametrs::standard==standard_language::c11||
                 parametrs::standard==standard_language::c17||
                 parametrs::standard==standard_language::c23)
            stream<<"static const uint8_t ";
        else if(parametrs::standard==standard_language::cpp11||
                 parametrs::standard==standard_language::cpp14)
            stream<<"constexpr uint8_t ";
        else if(parametrs::standard==standard_language::cpp17||
                 parametrs::standard==standard_language::cpp20||
                 parametrs::standard==standard_language::cpp23)
            stream<<"constexpr std::array<uint8_t,"<<size_file+1+bytes_width<<"> ";

        stream<<obj.name_array;

        if(parametrs::standard==standard_language::cpp17||
            parametrs::standard==standard_language::cpp20||
            parametrs::standard==standard_language::cpp23)
            stream<<" = {";
        else stream<<"[] = ";
        while (file.read(reinterpret_cast<char*>(buffer.data()), buffer.size()) || file.gcount() > 0)
        {

            std::streamsize bytes_read = file.gcount();

            if (bytes_read > 0) {
                stream<<" \"";
                for(int i=0;i<bytes_read;i++)
                    stream << "\\x"
                           << std::hex
                           << std::uppercase
                           << std::setw(2)
                           << std::setfill('0')
                           << static_cast<int>(buffer[i]);
                stream<<"\"";
            }
        }
        file.close();
        if(parametrs::width>0)
        {
            stream << " \"";
            for (size_t i = 0; i < bytes_width; ++i)
                stream << "\\x00";
            stream << "\"";
        }
        if(parametrs::standard==standard_language::cpp17||
            parametrs::standard==standard_language::cpp20||
            parametrs::standard==standard_language::cpp23)
            stream<<"}";
        stream<<";\n";
        if(parametrs::enable_size||parametrs::enable_size_0)
        {
            stream<<"size_t "<<obj.name_array<<"_size = "<< std::dec;
            if(parametrs::enable_size_0)stream<< size_file+1;
            else stream<<size_file;
            stream<<";\n";
        }
        stream<<"\n";
    }
    return true;
}
void namespace_end(std::ofstream& stream)
{
    if(parametrs::standard>=standard_language::cpp98 && parametrs::cpp_namespace)
    {
        stream<<"}\n\n";
    }
}
void include_guard_end(std::ofstream& stream)
{
    if(!parametrs::enable_pragma_once)
    {
        stream<<"#endif";
        if(parametrs::standard != standard_language::ansi)
        stream<<"// BIN2CPP_"<<detail::get_guard()<<"_HGUARD";
    }
}

}


#ifdef _WIN32
int wmain(int arg_count, wchar_t* args[])
#else
int main(int arg_count, char* args[])
#endif
{
#ifdef _WIN32
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);
#endif
    if(!stages::setting_parametrs(arg_count,args))return 1;

    auto output = std::filesystem::path(parametrs::output);

    if (output.has_parent_path())
    {
        std::error_code ec;
        std::filesystem::create_directories(output.parent_path(), ec);

        if (ec)
        {
            ccerr << cc("Error: failed to create directory\n");
            return 2;
        }
    }

    auto temp = output;
    temp +="." +detail::get_guard()+ ".tmp";

    std::ofstream stream(temp, std::ios::binary);

    if (!stream.is_open())
    {
        ccerr << cc("Error: failed to create temporary file at path ")
        << temp << cc('\n');
        return 2;
    }

    stages::include_guard_start(stream);
    stages::info(stream);
    stages::includes(stream);
    stages::namespace_start(stream);
    if(!stages::data(stream,parametrs::files))
    {
        stream.close();
        std::error_code ec;
        std::filesystem::remove(temp, ec);
        return 3;
    }
    stages::namespace_end(stream);
    stages::include_guard_end(stream);

    stream.close();

    std::error_code ec;
    std::filesystem::rename(temp, output, ec);

    if (ec)
    {
        ccerr << cc("Error: failed to replace output file\n");
        std::filesystem::remove(temp);
        return 4;
    }

    return 0;
}