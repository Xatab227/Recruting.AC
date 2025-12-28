/**
 * @file DiscordScanner.h
 * @brief Модуль анализа Discord данных
 * 
 * Анализ локальных файлов Discord на наличие
 * связей с читерскими серверами/каналами
 */

#ifndef DISCORD_SCANNER_H
#define DISCORD_SCANNER_H

#include <string>
#include <vector>
#include <functional>

namespace ForensicScanner {

/**
 * @enum DiscordMatchType
 * @brief Типы совпадений Discord
 */
enum class DiscordMatchType {
    SERVER_NAME,        ///< Название сервера
    CHANNEL_NAME,       ///< Название канала
    MESSAGE_KEYWORD,    ///< Ключевое слово в сообщении
    CACHE_FILE,         ///< Файл в кеше
    LEVELDB_ENTRY       ///< Запись в LevelDB
};

/**
 * @struct DiscordMatch
 * @brief Результат поиска в Discord
 */
struct DiscordMatch {
    DiscordMatchType matchType;     ///< Тип совпадения
    std::wstring source;            ///< Источник (путь к файлу)
    std::wstring serverName;        ///< Имя сервера (если доступно)
    std::wstring channelName;       ///< Имя канала (если доступно)
    std::wstring matchedValue;      ///< Совпавшее значение
    std::wstring context;           ///< Контекст (часть сообщения)
    int riskWeight;                 ///< Вес риска
    
    DiscordMatch() : matchType(DiscordMatchType::MESSAGE_KEYWORD), riskWeight(25) {}
};

/**
 * @brief Callback прогресса сканирования Discord
 */
using DiscordScanCallback = std::function<void(const std::wstring& status, int progress)>;

/**
 * @class DiscordScanner
 * @brief Класс для анализа данных Discord
 */
class DiscordScanner {
public:
    /**
     * @brief Конструктор
     */
    DiscordScanner();
    
    /**
     * @brief Деструктор
     */
    ~DiscordScanner();
    
    /**
     * @brief Установить список ключевых слов
     * @param keywords Ключевые слова
     */
    void setKeywords(const std::vector<std::wstring>& keywords);
    
    /**
     * @brief Установить чёрный список серверов/каналов
     * @param blacklist Чёрный список
     */
    void setBlacklist(const std::vector<std::wstring>& blacklist);
    
    /**
     * @brief Установить путь к папке Discord
     * @param path Путь
     */
    void setDiscordPath(const std::wstring& path);
    
    /**
     * @brief Установить вес риска
     * @param weight Вес
     */
    void setRiskWeight(int weight);
    
    /**
     * @brief Выполнить полное сканирование Discord
     * @param callback Callback прогресса
     * @return Вектор совпадений
     */
    std::vector<DiscordMatch> scan(DiscordScanCallback callback = nullptr);
    
    /**
     * @brief Сканировать LevelDB данные
     * @return Вектор совпадений
     */
    std::vector<DiscordMatch> scanLevelDB();
    
    /**
     * @brief Сканировать кеш-файлы
     * @return Вектор совпадений
     */
    std::vector<DiscordMatch> scanCache();
    
    /**
     * @brief Сканировать локальные хранилища
     * @return Вектор совпадений
     */
    std::vector<DiscordMatch> scanLocalStorage();
    
    /**
     * @brief Анализировать экспортированные логи
     * @param logPath Путь к файлу логов
     * @return Вектор совпадений
     */
    std::vector<DiscordMatch> scanExportedLogs(const std::wstring& logPath);
    
    /**
     * @brief Получить все найденные совпадения
     * @return Вектор совпадений
     */
    const std::vector<DiscordMatch>& getMatches() const;
    
    /**
     * @brief Получить количество совпадений
     * @return Количество
     */
    size_t getMatchCount() const;
    
    /**
     * @brief Очистить результаты
     */
    void clearResults();
    
    /**
     * @brief Преобразовать тип совпадения в строку
     * @param type Тип совпадения
     * @return Строковое представление
     */
    static std::wstring matchTypeToString(DiscordMatchType type);

private:
    std::vector<std::wstring> m_keywords;
    std::vector<std::wstring> m_blacklist;
    std::wstring m_discordPath;
    std::vector<DiscordMatch> m_matches;
    int m_riskWeight;
    
    /**
     * @brief Получить путь к Discord по умолчанию
     * @return Путь
     */
    std::wstring getDefaultDiscordPath();
    
    /**
     * @brief Сканировать текстовый файл
     * @param filePath Путь к файлу
     * @return Вектор совпадений
     */
    std::vector<DiscordMatch> scanTextFile(const std::wstring& filePath);
    
    /**
     * @brief Сканировать бинарный файл на строки
     * @param filePath Путь к файлу
     * @return Вектор совпадений
     */
    std::vector<DiscordMatch> scanBinaryFile(const std::wstring& filePath);
    
    /**
     * @brief Проверить строку на ключевые слова и чёрный список
     * @param text Текст
     * @param outMatch Найденное совпадение
     * @param outType Тип совпадения
     * @return true если найдено
     */
    bool checkText(const std::wstring& text, std::wstring& outMatch, DiscordMatchType& outType);
    
    /**
     * @brief Преобразовать в нижний регистр
     * @param str Строка
     * @return Строка в нижнем регистре
     */
    std::wstring toLower(const std::wstring& str);
    
    /**
     * @brief Получить список файлов рекурсивно
     * @param directory Директория
     * @return Список путей к файлам
     */
    std::vector<std::wstring> getFilesRecursive(const std::wstring& directory);
    
    /**
     * @brief Извлечь контекст вокруг позиции
     * @param text Полный текст
     * @param pos Позиция совпадения
     * @param contextSize Размер контекста
     * @return Строка контекста
     */
    std::wstring extractContext(const std::wstring& text, size_t pos, size_t contextSize = 50);
};

} // namespace ForensicScanner

#endif // DISCORD_SCANNER_H
