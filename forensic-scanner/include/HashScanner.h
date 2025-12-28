/**
 * @file HashScanner.h
 * @brief Модуль анализа хешей файлов
 * 
 * Подсчёт SHA-256 хешей и сравнение с базой запрещённого ПО
 */

#ifndef HASH_SCANNER_H
#define HASH_SCANNER_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <functional>

namespace ForensicScanner {

/**
 * @struct HashEntry
 * @brief Запись в базе хешей
 */
struct HashEntry {
    std::wstring hash;          ///< SHA-256 хеш
    std::wstring category;      ///< Категория (cheats, injectors и т.п.)
    std::wstring description;   ///< Описание
    int riskWeight;             ///< Вес риска
    
    HashEntry() : riskWeight(40) {}
};

/**
 * @struct ScanResult
 * @brief Результат сканирования файла
 */
struct ScanResult {
    std::wstring filePath;      ///< Путь к файлу
    std::wstring fileName;      ///< Имя файла
    std::wstring hash;          ///< Вычисленный хеш
    bool matched;               ///< Найдено совпадение
    HashEntry matchedEntry;     ///< Совпавшая запись из базы
    
    ScanResult() : matched(false) {}
};

/**
 * @brief Callback для отслеживания прогресса сканирования
 * @param current Текущий файл
 * @param total Всего файлов
 * @param currentFile Имя текущего файла
 */
using ScanProgressCallback = std::function<void(int current, int total, const std::wstring& currentFile)>;

/**
 * @class HashScanner
 * @brief Класс для сканирования и анализа хешей
 */
class HashScanner {
public:
    /**
     * @brief Конструктор
     */
    HashScanner();
    
    /**
     * @brief Деструктор
     */
    ~HashScanner();
    
    /**
     * @brief Загрузить базу хешей
     * @param databasePath Путь к файлу базы
     * @return true если успешно
     */
    bool loadHashDatabase(const std::wstring& databasePath);
    
    /**
     * @brief Сканировать директорию
     * @param directory Путь к директории
     * @param recursive Рекурсивное сканирование
     * @param callback Callback прогресса
     * @return Вектор результатов
     */
    std::vector<ScanResult> scanDirectory(const std::wstring& directory, 
                                          bool recursive = true,
                                          ScanProgressCallback callback = nullptr);
    
    /**
     * @brief Сканировать файл
     * @param filePath Путь к файлу
     * @return Результат сканирования
     */
    ScanResult scanFile(const std::wstring& filePath);
    
    /**
     * @brief Вычислить SHA-256 хеш файла
     * @param filePath Путь к файлу
     * @return Хеш в виде строки (hex)
     */
    std::wstring calculateSHA256(const std::wstring& filePath);
    
    /**
     * @brief Проверить хеш по базе
     * @param hash Хеш для проверки
     * @param outEntry Выходная запись при совпадении
     * @return true если найдено совпадение
     */
    bool checkHash(const std::wstring& hash, HashEntry& outEntry);
    
    /**
     * @brief Получить количество записей в базе
     * @return Количество записей
     */
    size_t getDatabaseSize() const;
    
    /**
     * @brief Получить статистику по категориям
     * @return Map категория -> количество
     */
    std::map<std::wstring, int> getCategoryStats() const;
    
    /**
     * @brief Получить все результаты с совпадениями
     * @return Вектор результатов
     */
    std::vector<ScanResult> getMatches() const;
    
    /**
     * @brief Очистить результаты
     */
    void clearResults();

private:
    std::map<std::wstring, HashEntry> m_hashDatabase;
    std::vector<ScanResult> m_results;
    
    /**
     * @brief Получить список файлов в директории
     * @param directory Путь к директории
     * @param recursive Рекурсивно
     * @return Список путей к файлам
     */
    std::vector<std::wstring> getFilesInDirectory(const std::wstring& directory, bool recursive);
    
    /**
     * @brief Преобразовать байты в hex строку
     * @param bytes Байты
     * @param length Длина
     * @return Hex строка
     */
    std::wstring bytesToHexString(const unsigned char* bytes, size_t length);
};

} // namespace ForensicScanner

#endif // HASH_SCANNER_H
