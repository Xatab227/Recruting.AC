#include "Utils.h"
#include <fstream>
#include <filesystem>
#include <locale>
#include <codecvt>

#ifdef _WIN32
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#endif

namespace fs = std::filesystem;

namespace ForensicScanner {
namespace Utils {

std::string ToLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string ToUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::string Trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

std::vector<std::string> Split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(Trim(token));
    }
    return tokens;
}

bool Contains(const std::string& str, const std::string& substr, bool caseSensitive) {
    if (caseSensitive) {
        return str.find(substr) != std::string::npos;
    }
    return ToLower(str).find(ToLower(substr)) != std::string::npos;
}

bool StartsWith(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && 
           str.compare(0, prefix.size(), prefix) == 0;
}

bool EndsWith(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() && 
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string ExpandEnvironmentPath(const std::string& path) {
#ifdef _WIN32
    char buffer[MAX_PATH];
    ExpandEnvironmentStringsA(path.c_str(), buffer, MAX_PATH);
    return std::string(buffer);
#else
    // Для Linux - простая замена переменных окружения
    std::string result = path;
    size_t pos = 0;
    while ((pos = result.find('%', pos)) != std::string::npos) {
        size_t end = result.find('%', pos + 1);
        if (end != std::string::npos) {
            std::string var = result.substr(pos + 1, end - pos - 1);
            const char* value = std::getenv(var.c_str());
            if (value) {
                result.replace(pos, end - pos + 1, value);
            }
        }
        pos++;
    }
    return result;
#endif
}

std::string GetFileName(const std::string& path) {
    fs::path p(path);
    return p.filename().string();
}

std::string GetDirectory(const std::string& path) {
    fs::path p(path);
    return p.parent_path().string();
}

std::string GetExtension(const std::string& path) {
    fs::path p(path);
    return p.extension().string();
}

bool FileExists(const std::string& path) {
    return fs::exists(path) && fs::is_regular_file(path);
}

bool DirectoryExists(const std::string& path) {
    return fs::exists(path) && fs::is_directory(path);
}

std::vector<std::string> GetFilesInDirectory(const std::string& directory, bool recursive) {
    std::vector<std::string> files;
    
    try {
        if (!DirectoryExists(directory)) {
            return files;
        }
        
        if (recursive) {
            for (const auto& entry : fs::recursive_directory_iterator(directory, 
                    fs::directory_options::skip_permission_denied)) {
                if (entry.is_regular_file()) {
                    files.push_back(entry.path().string());
                }
            }
        } else {
            for (const auto& entry : fs::directory_iterator(directory)) {
                if (entry.is_regular_file()) {
                    files.push_back(entry.path().string());
                }
            }
        }
    } catch (const std::exception& e) {
        // Игнорируем ошибки доступа
    }
    
    return files;
}

std::string FormatTimestamp(std::time_t timestamp) {
    std::tm* tm = std::localtime(&timestamp);
    std::stringstream ss;
    ss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::time_t GetCurrentTimestamp() {
    return std::time(nullptr);
}

std::string BytesToHexString(const unsigned char* bytes, size_t length) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < length; ++i) {
        ss << std::setw(2) << static_cast<int>(bytes[i]);
    }
    return ss.str();
}

std::string CalculateSHA256(const std::string& filePath) {
#ifdef _WIN32
    // Windows BCrypt API для SHA-256
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    BCRYPT_HASH_HANDLE hHash = nullptr;
    NTSTATUS status;
    
    std::string result;
    
    // Открываем файл
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }
    
    // Инициализация алгоритма
    status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, 0);
    if (!BCRYPT_SUCCESS(status)) {
        return "";
    }
    
    // Получаем размеры буферов
    DWORD hashObjectSize = 0;
    DWORD dataSize = 0;
    status = BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, 
                               (PUCHAR)&hashObjectSize, sizeof(DWORD), &dataSize, 0);
    if (!BCRYPT_SUCCESS(status)) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return "";
    }
    
    DWORD hashSize = 0;
    status = BCryptGetProperty(hAlg, BCRYPT_HASH_LENGTH,
                               (PUCHAR)&hashSize, sizeof(DWORD), &dataSize, 0);
    if (!BCRYPT_SUCCESS(status)) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return "";
    }
    
    // Выделяем память
    std::vector<UCHAR> hashObject(hashObjectSize);
    std::vector<UCHAR> hash(hashSize);
    
    // Создаём хеш-объект
    status = BCryptCreateHash(hAlg, &hHash, hashObject.data(), hashObjectSize, 
                              nullptr, 0, 0);
    if (!BCRYPT_SUCCESS(status)) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return "";
    }
    
    // Читаем файл блоками и обновляем хеш
    const size_t bufferSize = 65536;
    std::vector<char> buffer(bufferSize);
    
    while (file.good()) {
        file.read(buffer.data(), bufferSize);
        std::streamsize bytesRead = file.gcount();
        if (bytesRead > 0) {
            status = BCryptHashData(hHash, (PUCHAR)buffer.data(), 
                                   static_cast<ULONG>(bytesRead), 0);
            if (!BCRYPT_SUCCESS(status)) {
                BCryptDestroyHash(hHash);
                BCryptCloseAlgorithmProvider(hAlg, 0);
                return "";
            }
        }
    }
    
    // Завершаем хеширование
    status = BCryptFinishHash(hHash, hash.data(), hashSize, 0);
    if (BCRYPT_SUCCESS(status)) {
        result = BytesToHexString(hash.data(), hashSize);
    }
    
    // Очистка
    BCryptDestroyHash(hHash);
    BCryptCloseAlgorithmProvider(hAlg, 0);
    
    return result;
#else
    // Заглушка для Linux - можно использовать OpenSSL
    return "";
#endif
}

std::string ReadFileContent(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::vector<std::string> ReadFileLines(const std::string& path) {
    std::vector<std::string> lines;
    std::ifstream file(path);
    
    if (!file.is_open()) {
        return lines;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        line = Trim(line);
        if (!line.empty() && line[0] != '#') { // Игнорируем пустые строки и комментарии
            lines.push_back(line);
        }
    }
    
    return lines;
}

std::string WStringToString(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    
#ifdef _WIN32
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string result(size - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], size, nullptr, nullptr);
    return result;
#else
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
#endif
}

std::wstring StringToWString(const std::string& str) {
    if (str.empty()) return L"";
    
#ifdef _WIN32
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring result(size - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result[0], size);
    return result;
#else
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
#endif
}

std::string FormatFileSize(size_t bytes) {
    const char* units[] = {"Б", "КБ", "МБ", "ГБ", "ТБ"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024.0 && unitIndex < 4) {
        size /= 1024.0;
        unitIndex++;
    }
    
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << size << " " << units[unitIndex];
    return ss.str();
}

} // namespace Utils
} // namespace ForensicScanner
