#include "RiskCalculator.h"
#include "Logger.h"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace ForensicScanner {

RiskCalculator::RiskCalculator() {
}

void RiskCalculator::Calculate(ScanResults& results) {
    // Суммируем веса всех триггеров
    int hashRisk = CalculateCategoryRisk(results.hashTriggers);
    int browserRisk = CalculateCategoryRisk(results.browserTriggers);
    int discordRisk = CalculateCategoryRisk(results.discordTriggers);
    
    // Общий риск - сумма с учётом понижающего коэффициента
    int totalTriggers = static_cast<int>(
        results.hashTriggers.size() + 
        results.browserTriggers.size() + 
        results.discordTriggers.size()
    );
    
    int rawSum = hashRisk + browserRisk + discordRisk;
    results.totalRiskPercent = ApplyDiminishingReturns(rawSum, totalTriggers);
    results.riskLevel = GetRiskLevel(results.totalRiskPercent);
    
    // Статистика
    results.hashMatches = static_cast<int>(results.hashTriggers.size());
    results.keywordsFound = 0;
    results.suspiciousURLs = 0;
    results.discordMatches = static_cast<int>(results.discordTriggers.size());
    
    // Подсчитываем типы браузерных триггеров
    for (const auto& trigger : results.browserTriggers) {
        if (trigger.type == TriggerType::KeywordFound) {
            results.keywordsFound++;
        } else if (trigger.type == TriggerType::SuspiciousURL || 
                   trigger.type == TriggerType::AuthAttempt) {
            results.suspiciousURLs++;
        }
    }
    
    // Логируем результат
    std::stringstream ss;
    ss << "Расчёт риска завершён:\n";
    ss << "  - Риск по хешам: " << hashRisk << "%\n";
    ss << "  - Риск по браузеру: " << browserRisk << "%\n";
    ss << "  - Риск по Discord: " << discordRisk << "%\n";
    ss << "  - ИТОГО: " << results.totalRiskPercent << "% (" 
       << RiskLevelToString(results.riskLevel) << ")";
    
    Logger::Instance().Info(ss.str(), "risk");
}

int RiskCalculator::CalculateCategoryRisk(const std::vector<TriggerEvent>& triggers) {
    if (triggers.empty()) {
        return 0;
    }
    
    int sum = 0;
    for (const auto& trigger : triggers) {
        sum += trigger.weight;
    }
    
    // Применяем понижающий коэффициент
    return ApplyDiminishingReturns(sum, static_cast<int>(triggers.size()));
}

RiskLevel RiskCalculator::GetRiskLevel(int percent) {
    if (percent <= 20) {
        return RiskLevel::Low;
    } else if (percent <= 50) {
        return RiskLevel::Medium;
    } else {
        return RiskLevel::High;
    }
}

void RiskCalculator::GetRiskColor(RiskLevel level, float& r, float& g, float& b, float& a) {
    a = 1.0f;
    
    switch (level) {
        case RiskLevel::Low:
            // Зелёный
            r = 0.2f;
            g = 0.8f;
            b = 0.2f;
            break;
        case RiskLevel::Medium:
            // Жёлтый/Оранжевый
            r = 1.0f;
            g = 0.7f;
            b = 0.0f;
            break;
        case RiskLevel::High:
            // Красный
            r = 0.9f;
            g = 0.1f;
            b = 0.1f;
            break;
    }
}

void RiskCalculator::GetRiskColorByPercent(int percent, float& r, float& g, float& b, float& a) {
    GetRiskColor(GetRiskLevel(percent), r, g, b, a);
}

std::string RiskCalculator::GetRiskDescription(RiskLevel level) {
    switch (level) {
        case RiskLevel::Low:
            return "Низкий риск. Подозрительных признаков не обнаружено или их количество минимально.";
        case RiskLevel::Medium:
            return "Средний риск. Обнаружены подозрительные признаки. Рекомендуется дополнительная проверка.";
        case RiskLevel::High:
            return "Высокий риск! Обнаружены серьёзные признаки использования запрещённого ПО.";
        default:
            return "Неизвестный уровень риска.";
    }
}

std::string RiskCalculator::GetRiskDescriptionByPercent(int percent) {
    return GetRiskDescription(GetRiskLevel(percent));
}

int RiskCalculator::ClampPercent(int value) {
    return std::clamp(value, 0, 100);
}

int RiskCalculator::ApplyDiminishingReturns(int rawSum, int triggerCount) {
    if (triggerCount <= 0 || rawSum <= 0) {
        return 0;
    }
    
    // Используем логарифмическую шкалу для понижения влияния
    // большого количества однотипных триггеров
    
    // Первые 3 триггера дают полный вес
    // Последующие дают меньше
    
    if (triggerCount <= 3) {
        return ClampPercent(rawSum);
    }
    
    // Формула: базовый_вес * (1 + log2(количество_триггеров))
    // Это даёт понижающий эффект при большом количестве
    double factor = 1.0 + std::log2(static_cast<double>(triggerCount));
    int adjusted = static_cast<int>(rawSum / factor * 1.5);
    
    return ClampPercent(adjusted);
}

} // namespace ForensicScanner
