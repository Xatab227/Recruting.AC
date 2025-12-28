/**
 * @file Config.h
 * @brief Модуль конфигурации форензик-сканера
 * 
 * Управление настройками программы: пути, ключевые слова,
 * списки сайтов, веса для расчёта риска
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>
#include <map>
#include <Windows.h>

namespace ForensicScanner {

/**
 * @struct ConfigData
 * @brief Структура для хранения всех настроек программы
 */
struct ConfigData {
    // Пути для сканирования
    std::vector<std::wstring> scanDirectories;
    
    // Путь к базе хешей
    std::wstring hashDatabasePath;
    
    // Путь к файлу логов
    std::wstring logFilePath;
    
    // Ключевые слова для поиска
    std::vector<std::wstring> keywords;
    
    // Чёрный список сайтов
    std::vector<std::wstring> blacklistSites;
    
    // Названия Discord-серверов/каналов для поиска
    std::vector<std::wstring> discordBlacklist;
    
    // Веса для расчёта риска
    int hashMatchWeight;      // Вес совпадения хеша (по умолчанию 40)
    int keywordMatchWeight;   // Вес ключевого слова (по умолчанию 15)
    int siteVisitWeight;      // Вес посещения сайта (по умолчанию 20)
    int discordMatchWeight;   // Вес совпадения в Discord (по умолчанию 25)
    
    // Пути к данным браузеров
    std::vector<std::wstring> browserPaths;
    
    // Путь к папке Discord
    std::wstring discordPath;
};

/**
 * @class Config
 * @brief Класс для работы с конфигурацией программы
 */
class Config {
public:
    /**
     * @brief Конструктор по умолчанию
     */
    Config();
    
    /**
     * @brief Деструктор
     */
    ~Config();
    
    /**
     * @brief Загрузить конфигурацию из файлов
     * @param configDir Директория с конфигурационными файлами
     * @return true если загрузка успешна
     */
    bool loadConfig(const std::wstring& configDir);
    
    /**
     * @brief Получить данные конфигурации
     * @return Константная ссылка на структуру конфигурации
     */
    const ConfigData& getData() const;
    
    /**
     * @brief Установить значения по умолчанию
     */
    void setDefaults();
    
    /**
     * @brief Загрузить ключевые слова из файла
     * @param filePath Путь к файлу
     * @return true если успешно
     */
    bool loadKeywords(const std::wstring& filePath);
    
    /**
     * @brief Загрузить чёрный список сайтов
     * @param filePath Путь к файлу
     * @return true если успешно
     */
    bool loadBlacklistSites(const std::wstring& filePath);
    
    /**
     * @brief Загрузить чёрный список Discord
     * @param filePath Путь к файлу
     * @return true если успешно
     */
    bool loadDiscordBlacklist(const std::wstring& filePath);
    
    /**
     * @brief Получить расширенную переменную окружения
     * @param path Путь с переменными окружения
     * @return Расширенный путь
     */
    static std::wstring expandEnvironmentPath(const std::wstring& path);

private:
    ConfigData m_data;
    
    /**
     * @brief Загрузить список строк из файла
     * @param filePath Путь к файлу
     * @param outList Выходной список
     * @return true если успешно
     */
    bool loadStringList(const std::wstring& filePath, std::vector<std::wstring>& outList);
};

} // namespace ForensicScanner

#endif // CONFIG_H
