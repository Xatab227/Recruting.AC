/**
 * @file RiskCalculator.cpp
 * @brief Реализация модуля расчёта риска
 */

#include "../include/RiskCalculator.h"
#include <algorithm>

namespace ForensicScanner {

RiskCalculator::RiskCalculator() {
}

RiskCalculator::~RiskCalculator() {
}

void RiskCalculator::setHashResults(const std::vector<ScanResult>& results) {
    m_hashResults = results;
}

void RiskCalculator::setBrowserResults(const std::vector<BrowserMatch>& matches) {
    m_browserResults = matches;
}

void RiskCalculator::setDiscordResults(const std::vector<DiscordMatch>& matches) {
    m_discordResults = matches;
}

void RiskCalculator::addHashResult(const ScanResult& result) {
    m_hashResults.push_back(result);
}

void RiskCalculator::addBrowserResult(const BrowserMatch& match) {
    m_browserResults.push_back(match);
}

void RiskCalculator::addDiscordResult(const DiscordMatch& match) {
    m_discordResults.push_back(match);
}

RiskSummary RiskCalculator::calculate() {
    m_summary = RiskSummary();
    
    // Считаем триггеры
    for (const auto& result : m_hashResults) {
        if (result.matched) {
            m_summary.hashTriggers++;
        }
    }
    
    m_summary.browserTriggers = static_cast<int>(m_browserResults.size());
    m_summary.discordTriggers = static_cast<int>(m_discordResults.size());
    
    // Рассчитываем вклад каждой категории
    m_summary.hashRiskContribution = calculateHashContribution();
    m_summary.browserRiskContribution = calculateBrowserContribution();
    m_summary.discordRiskContribution = calculateDiscordContribution();
    
    // Общий риск
    m_summary.totalRiskPercent = m_summary.hashRiskContribution + 
                                  m_summary.browserRiskContribution + 
                                  m_summary.discordRiskContribution;
    
    // Ограничиваем диапазоном 0-100
    m_summary.totalRiskPercent = std::max(0, std::min(100, m_summary.totalRiskPercent));
    
    // Определяем уровень риска
    m_summary.riskLevel = getRiskLevel(m_summary.totalRiskPercent);
    
    // Формируем рекомендацию
    m_summary.recommendation = getRecommendation(m_summary.riskLevel);
    
    return m_summary;
}

int RiskCalculator::calculateHashContribution() {
    int contribution = 0;
    
    for (const auto& result : m_hashResults) {
        if (result.matched) {
            contribution += result.matchedEntry.riskWeight;
        }
    }
    
    // Ограничиваем максимальный вклад хешей 50%
    return std::min(50, contribution);
}

int RiskCalculator::calculateBrowserContribution() {
    int contribution = 0;
    
    for (const auto& match : m_browserResults) {
        contribution += match.riskWeight;
        
        // Дополнительно за попытку авторизации
        if (match.isAuthAttempt) {
            contribution += 5;
        }
    }
    
    // Ограничиваем максимальный вклад браузера 30%
    return std::min(30, contribution);
}

int RiskCalculator::calculateDiscordContribution() {
    int contribution = 0;
    
    for (const auto& match : m_discordResults) {
        contribution += match.riskWeight;
    }
    
    // Ограничиваем максимальный вклад Discord 30%
    return std::min(30, contribution);
}

const RiskSummary& RiskCalculator::getSummary() const {
    return m_summary;
}

RiskLevel RiskCalculator::getRiskLevel(int percent) {
    if (percent <= 20) {
        return RiskLevel::LOW;
    } else if (percent <= 50) {
        return RiskLevel::MEDIUM;
    } else if (percent <= 70) {
        return RiskLevel::HIGH;
    } else {
        return RiskLevel::CRITICAL;
    }
}

void RiskCalculator::getRiskColor(RiskLevel level, float& r, float& g, float& b, float& a) {
    a = 1.0f;
    
    switch (level) {
        case RiskLevel::LOW:
            r = 0.2f; g = 0.8f; b = 0.2f; // Зелёный
            break;
        case RiskLevel::MEDIUM:
            r = 1.0f; g = 0.8f; b = 0.0f; // Жёлтый
            break;
        case RiskLevel::HIGH:
            r = 1.0f; g = 0.5f; b = 0.0f; // Оранжевый
            break;
        case RiskLevel::CRITICAL:
            r = 0.9f; g = 0.1f; b = 0.1f; // Красный
            break;
        default:
            r = 0.5f; g = 0.5f; b = 0.5f; // Серый
            break;
    }
}

std::wstring RiskCalculator::riskLevelToString(RiskLevel level) {
    switch (level) {
        case RiskLevel::LOW:        return L"НИЗКИЙ";
        case RiskLevel::MEDIUM:     return L"СРЕДНИЙ";
        case RiskLevel::HIGH:       return L"ВЫСОКИЙ";
        case RiskLevel::CRITICAL:   return L"КРИТИЧЕСКИЙ";
        default:                    return L"НЕИЗВЕСТНО";
    }
}

std::wstring RiskCalculator::getRecommendation(RiskLevel level) {
    switch (level) {
        case RiskLevel::LOW:
            return L"Риск низкий. Подозрительных признаков не обнаружено или их очень мало.";
            
        case RiskLevel::MEDIUM:
            return L"Обнаружены подозрительные признаки. Рекомендуется дополнительная проверка.";
            
        case RiskLevel::HIGH:
            return L"Высокий риск! Обнаружены значительные признаки использования запрещённого ПО.";
            
        case RiskLevel::CRITICAL:
            return L"КРИТИЧЕСКИЙ РИСК! Высокая вероятность наличия читов или запрещённых модификаций!";
            
        default:
            return L"Статус неопределён.";
    }
}

void RiskCalculator::clear() {
    m_hashResults.clear();
    m_browserResults.clear();
    m_discordResults.clear();
    m_summary = RiskSummary();
}

} // namespace ForensicScanner
