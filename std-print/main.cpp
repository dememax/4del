// from https://cppreference.com/w/cpp/io/print.html
#include <cstdio>
#include <filesystem>
#include <print>
 
int main()
{
    std::print("{2} {1}{0}!\n", 23, "C++", "Hello");  // overload (1)
 
    const auto tmp{std::filesystem::temp_directory_path() / "test.txt"};
    if (std::FILE* stream{std::fopen(tmp.c_str(), "w")})
    {
        std::print(stream, "File: {}", tmp.string()); // overload (2)
        std::fclose(stream);
    }
}
