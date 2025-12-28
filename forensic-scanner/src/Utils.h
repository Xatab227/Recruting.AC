#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#endif

namespace ForensicScanner {
namespace Utils {

// Работа со строками
std::string ToLower(const std::string& str);
std::string ToUpper(const std::string& str);
std::string Trim(const std::string& str);
std::vector<std::string> Split(const std::string& str, char delimiter);
bool Contains(const std::string& str, const std::string& substr, bool caseSensitive = false);
bool StartsWith(const std::string& str, const std::string& prefix);
bool EndsWith(const std::string& str, const std::string& suffix);

// Работа с путями
std::string ExpandEnvironmentPath(const std::string& path);
std::string GetFileName(const std::string& path);
std::string GetDirectory(const std::string& path);
std::string GetExtension(const std::string& path);
bool FileExists(const std::string& path);
bool DirectoryExists(const std::string& path);
std::vector<std::string> GetFilesInDirectory(const std::string& directory, bool recursive = false);

// Работа с датой/временем
std::string FormatTimestamp(std::time_t timestamp);
std::time_t GetCurrentTimestamp();

// Работа с хешами
std::string BytesToHexString(const unsigned char* bytes, size_t length);
std::string CalculateSHA256(const std::string& filePath);

// Чтение файлов
std::string ReadFileContent(const std::string& path);
std::vector<std::string> ReadFileLines(const std::string& path);

// Кодировки
std::string WStringToString(const std::wstring& wstr);
std::wstring StringToWString(const std::string& str);

// Форматирование размера файла
std::string FormatFileSize(size_t bytes);

} // namespace Utils
} // namespace ForensicScanner
