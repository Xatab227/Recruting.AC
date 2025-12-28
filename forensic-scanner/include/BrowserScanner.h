/**
 * @file BrowserScanner.h
 * @brief Модуль анализа браузера и My Activity
 * 
 * Анализ истории браузеров и экспорта Google My Activity
 * на наличие подозрительных ключевых слов и сайтов
 */

#ifndef BROWSER_SCANNER_H
#define BROWSER_SCANNER_H

#include <string>
#include <vector>
#include <functional>

namespace ForensicScanner {

/**
 * @enum BrowserType
 * @brief Типы браузеров
 */
enum class BrowserType {
    CHROME,
    FIREFOX,
    EDGE,
    OPERA,
    MY_ACTIVITY,    ///< Google My Activity export
    UNKNOWN
};

/**
 * @struct BrowserMatch
 * @brief Результат поиска в браузере
 */
struct BrowserMatch {
    BrowserType browserType;        ///< Тип браузера/источника
    std::wstring source;            ///< Путь к файлу источника
    std::wstring matchType;         ///< Тип совпадения (keyword/url)
    std::wstring matchedValue;      ///< Совпавшее значение
    std::wstring context;           ///< Контекст (URL или строка)
    std::wstring timestamp;         ///< Время посещения (если доступно)
    bool isAuthAttempt;             ///< Попытка авторизации
    int riskWeight;                 ///< Вес риска
    
    BrowserMatch() : browserType(BrowserType::UNKNOWN), isAuthAttempt(false), riskWeight(15) {}
};

/**
 * @brief Callback прогресса сканирования браузера
 */
using BrowserScanCallback = std::function<void(const std::wstring& status, int progress)>;

/**
 * @class BrowserScanner
 * @brief Класс для анализа истории браузеров
 */
class BrowserScanner {
public:
    /**
     * @brief Конструктор
     */
    BrowserScanner();
    
    /**
     * @brief Деструктор
     */
    ~BrowserScanner();
    
    /**
     * @brief Установить список ключевых слов
     * @param keywords Ключевые слова
     */
    void setKeywords(const std::vector<std::wstring>& keywords);
    
    /**
     * @brief Установить чёрный список сайтов
     * @param sites Список сайтов
     */
    void setBlacklistSites(const std::vector<std::wstring>& sites);
    
    /**
     * @brief Установить вес для ключевых слов
     * @param weight Вес
     */
    void setKeywordWeight(int weight);
    
    /**
     * @brief Установить вес для посещения сайтов
     * @param weight Вес
     */
    void setSiteVisitWeight(int weight);
    
    /**
     * @brief Сканировать все браузеры
     * @param callback Callback прогресса
     * @return Вектор найденных совпадений
     */
    std::vector<BrowserMatch> scanAllBrowsers(BrowserScanCallback callback = nullptr);
    
    /**
     * @brief Сканировать Chrome
     * @return Вектор совпадений
     */
    std::vector<BrowserMatch> scanChrome();
    
    /**
     * @brief Сканировать Firefox
     * @return Вектор совпадений
     */
    std::vector<BrowserMatch> scanFirefox();
    
    /**
     * @brief Сканировать Edge
     * @return Вектор совпадений
     */
    std::vector<BrowserMatch> scanEdge();
    
    /**
     * @brief Сканировать Opera
     * @return Вектор совпадений
     */
    std::vector<BrowserMatch> scanOpera();
    
    /**
     * @brief Анализировать файл My Activity
     * @param filePath Путь к файлу (JSON или HTML)
     * @return Вектор совпадений
     */
    std::vector<BrowserMatch> scanMyActivity(const std::wstring& filePath);
    
    /**
     * @brief Анализировать произвольный текстовый файл
     * @param filePath Путь к файлу
     * @param browserType Тип источника
     * @return Вектор совпадений
     */
    std::vector<BrowserMatch> scanTextFile(const std::wstring& filePath, BrowserType browserType);
    
    /**
     * @brief Получить все найденные совпадения
     * @return Вектор совпадений
     */
    const std::vector<BrowserMatch>& getMatches() const;
    
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
     * @brief Преобразовать тип браузера в строку
     * @param type Тип браузера
     * @return Строковое представление
     */
    static std::wstring browserTypeToString(BrowserType type);

private:
    std::vector<std::wstring> m_keywords;
    std::vector<std::wstring> m_blacklistSites;
    std::vector<BrowserMatch> m_matches;
    int m_keywordWeight;
    int m_siteVisitWeight;
    
    /**
     * @brief Сканировать Chromium-based браузер
     * @param profilePath Путь к профилю
     * @param browserType Тип браузера
     * @return Вектор совпадений
     */
    std::vector<BrowserMatch> scanChromiumBrowser(const std::wstring& profilePath, BrowserType browserType);
    
    /**
     * @brief Проверить строку на ключевые слова
     * @param text Текст для проверки
     * @param outKeyword Найденное ключевое слово
     * @return true если найдено
     */
    bool checkKeywords(const std::wstring& text, std::wstring& outKeyword);
    
    /**
     * @brief Проверить URL на чёрный список
     * @param url URL для проверки
     * @param outSite Найденный сайт
     * @return true если найдено
     */
    bool checkBlacklist(const std::wstring& url, std::wstring& outSite);
    
    /**
     * @brief Проверить на попытку авторизации
     * @param url URL для проверки
     * @return true если похоже на авторизацию
     */
    bool isAuthAttempt(const std::wstring& url);
    
    /**
     * @brief Преобразовать строку в нижний регистр
     * @param str Строка
     * @return Строка в нижнем регистре
     */
    std::wstring toLower(const std::wstring& str);
    
    /**
     * @brief Анализировать SQLite базу истории
     * @param dbPath Путь к базе
     * @param browserType Тип браузера
     * @return Вектор совпадений
     */
    std::vector<BrowserMatch> analyzeHistoryDb(const std::wstring& dbPath, BrowserType browserType);
};

} // namespace ForensicScanner

#endif // BROWSER_SCANNER_H
