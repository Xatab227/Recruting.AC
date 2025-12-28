/**
 * @file RiskCalculator.h
 * @brief Модуль расчёта риска
 * 
 * Подсчёт итогового процента риска на основе найденных триггеров
 */

#ifndef RISK_CALCULATOR_H
#define RISK_CALCULATOR_H

#include <string>
#include <vector>
#include "HashScanner.h"
#include "BrowserScanner.h"
#include "DiscordScanner.h"

namespace ForensicScanner {

/**
 * @enum RiskLevel
 * @brief Уровни риска
 */
enum class RiskLevel {
    LOW,        ///< Низкий (0-20%) - зелёный
    MEDIUM,     ///< Средний (21-50%) - жёлтый
    HIGH,       ///< Высокий (51-70%) - оранжевый
    CRITICAL    ///< Критический (>70%) - красный
};

/**
 * @struct RiskSummary
 * @brief Сводка по рискам
 */
struct RiskSummary {
    int totalRiskPercent;           ///< Общий процент риска (0-100)
    RiskLevel riskLevel;            ///< Уровень риска
    
    int hashTriggers;               ///< Количество триггеров хешей
    int browserTriggers;            ///< Количество триггеров браузера
    int discordTriggers;            ///< Количество триггеров Discord
    
    int hashRiskContribution;       ///< Вклад хешей в риск
    int browserRiskContribution;    ///< Вклад браузера в риск
    int discordRiskContribution;    ///< Вклад Discord в риск
    
    std::wstring recommendation;    ///< Рекомендация
    
    RiskSummary() : totalRiskPercent(0), riskLevel(RiskLevel::LOW),
                    hashTriggers(0), browserTriggers(0), discordTriggers(0),
                    hashRiskContribution(0), browserRiskContribution(0), discordRiskContribution(0) {}
};

/**
 * @class RiskCalculator
 * @brief Класс для расчёта риска
 */
class RiskCalculator {
public:
    /**
     * @brief Конструктор
     */
    RiskCalculator();
    
    /**
     * @brief Деструктор
     */
    ~RiskCalculator();
    
    /**
     * @brief Установить результаты сканирования хешей
     * @param results Результаты
     */
    void setHashResults(const std::vector<ScanResult>& results);
    
    /**
     * @brief Установить результаты сканирования браузера
     * @param matches Совпадения
     */
    void setBrowserResults(const std::vector<BrowserMatch>& matches);
    
    /**
     * @brief Установить результаты сканирования Discord
     * @param matches Совпадения
     */
    void setDiscordResults(const std::vector<DiscordMatch>& matches);
    
    /**
     * @brief Добавить результат хеша
     * @param result Результат
     */
    void addHashResult(const ScanResult& result);
    
    /**
     * @brief Добавить результат браузера
     * @param match Совпадение
     */
    void addBrowserResult(const BrowserMatch& match);
    
    /**
     * @brief Добавить результат Discord
     * @param match Совпадение
     */
    void addDiscordResult(const DiscordMatch& match);
    
    /**
     * @brief Рассчитать риск
     * @return Сводка по рискам
     */
    RiskSummary calculate();
    
    /**
     * @brief Получить последнюю сводку
     * @return Сводка по рискам
     */
    const RiskSummary& getSummary() const;
    
    /**
     * @brief Получить уровень риска по проценту
     * @param percent Процент риска
     * @return Уровень риска
     */
    static RiskLevel getRiskLevel(int percent);
    
    /**
     * @brief Получить цвет для уровня риска (RGBA)
     * @param level Уровень риска
     * @param r Красный (выход)
     * @param g Зелёный (выход)
     * @param b Синий (выход)
     * @param a Альфа (выход)
     */
    static void getRiskColor(RiskLevel level, float& r, float& g, float& b, float& a);
    
    /**
     * @brief Преобразовать уровень риска в строку
     * @param level Уровень риска
     * @return Строковое представление
     */
    static std::wstring riskLevelToString(RiskLevel level);
    
    /**
     * @brief Получить рекомендацию по уровню риска
     * @param level Уровень риска
     * @return Текст рекомендации
     */
    static std::wstring getRecommendation(RiskLevel level);
    
    /**
     * @brief Очистить результаты
     */
    void clear();

private:
    std::vector<ScanResult> m_hashResults;
    std::vector<BrowserMatch> m_browserResults;
    std::vector<DiscordMatch> m_discordResults;
    RiskSummary m_summary;
    
    /**
     * @brief Рассчитать вклад хешей
     * @return Процент
     */
    int calculateHashContribution();
    
    /**
     * @brief Рассчитать вклад браузера
     * @return Процент
     */
    int calculateBrowserContribution();
    
    /**
     * @brief Рассчитать вклад Discord
     * @return Процент
     */
    int calculateDiscordContribution();
};

} // namespace ForensicScanner

#endif // RISK_CALCULATOR_H
