/**
 * @file Logger.h
 * @brief Модуль логирования форензик-сканера
 * 
 * Ведение логов в файл и память для отображения в UI
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <vector>
#include <fstream>
#include <mutex>
#include <ctime>

namespace ForensicScanner {

/**
 * @enum LogCategory
 * @brief Категории логов
 */
enum class LogCategory {
    HASH,       ///< Лог хешей
    BROWSER,    ///< Лог браузера
    DISCORD,    ///< Лог Discord
    SYSTEM      ///< Системные сообщения
};

/**
 * @enum TriggerType
 * @brief Типы триггеров
 */
enum class TriggerType {
    HASH_MATCH,         ///< Совпадение хеша
    KEYWORD_MATCH,      ///< Совпадение ключевого слова
    URL_MATCH,          ///< Совпадение URL
    DISCORD_SERVER,     ///< Совпадение Discord сервера
    DISCORD_CHANNEL,    ///< Совпадение Discord канала
    DISCORD_KEYWORD     ///< Ключевое слово в Discord
};

/**
 * @struct LogEntry
 * @brief Запись лога
 */
struct LogEntry {
    LogCategory category;           ///< Категория лога
    TriggerType triggerType;        ///< Тип триггера
    std::wstring source;            ///< Источник (путь к файлу и т.п.)
    std::wstring matchedValue;      ///< Совпавшее значение
    std::wstring details;           ///< Дополнительные детали
    int riskWeight;                 ///< Вес риска в процентах
    std::time_t timestamp;          ///< Время обнаружения
    bool expanded;                  ///< Раскрыта ли запись в UI
    
    LogEntry() : category(LogCategory::SYSTEM), triggerType(TriggerType::KEYWORD_MATCH),
                 riskWeight(0), timestamp(0), expanded(false) {}
};

/**
 * @class Logger
 * @brief Класс для логирования
 */
class Logger {
public:
    /**
     * @brief Получить экземпляр логгера (Singleton)
     * @return Ссылка на экземпляр
     */
    static Logger& getInstance();
    
    /**
     * @brief Инициализировать логгер
     * @param logFilePath Путь к файлу лога
     * @return true если успешно
     */
    bool initialize(const std::wstring& logFilePath);
    
    /**
     * @brief Закрыть логгер
     */
    void shutdown();
    
    /**
     * @brief Добавить запись лога
     * @param entry Запись лога
     */
    void addEntry(const LogEntry& entry);
    
    /**
     * @brief Записать информационное сообщение
     * @param category Категория
     * @param message Сообщение
     */
    void info(LogCategory category, const std::wstring& message);
    
    /**
     * @brief Записать предупреждение
     * @param category Категория
     * @param message Сообщение
     */
    void warning(LogCategory category, const std::wstring& message);
    
    /**
     * @brief Записать ошибку
     * @param category Категория
     * @param message Сообщение
     */
    void error(LogCategory category, const std::wstring& message);
    
    /**
     * @brief Получить все записи логов
     * @return Вектор записей
     */
    const std::vector<LogEntry>& getEntries() const;
    
    /**
     * @brief Получить записи по категории
     * @param category Категория
     * @return Вектор записей
     */
    std::vector<LogEntry> getEntriesByCategory(LogCategory category) const;
    
    /**
     * @brief Получить количество триггеров по категории
     * @param category Категория
     * @return Количество триггеров
     */
    int getTriggerCount(LogCategory category) const;
    
    /**
     * @brief Очистить логи
     */
    void clear();
    
    /**
     * @brief Преобразовать категорию в строку
     * @param category Категория
     * @return Строковое представление
     */
    static std::wstring categoryToString(LogCategory category);
    
    /**
     * @brief Преобразовать тип триггера в строку
     * @param type Тип триггера
     * @return Строковое представление
     */
    static std::wstring triggerTypeToString(TriggerType type);

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    std::vector<LogEntry> m_entries;
    std::wofstream m_logFile;
    std::mutex m_mutex;
    bool m_initialized;
    
    /**
     * @brief Записать в файл
     * @param message Сообщение
     */
    void writeToFile(const std::wstring& message);
    
    /**
     * @brief Форматировать время
     * @param time Время
     * @return Отформатированная строка
     */
    std::wstring formatTime(std::time_t time) const;
};

} // namespace ForensicScanner

#endif // LOGGER_H
