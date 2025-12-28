#pragma once

#include <vector>
#include "Types.h"

namespace ForensicScanner {

class RiskCalculator {
public:
    RiskCalculator();
    ~RiskCalculator() = default;
    
    // Расчёт общего риска на основе всех триггеров
    void Calculate(ScanResults& results);
    
    // Расчёт риска для конкретной категории
    int CalculateCategoryRisk(const std::vector<TriggerEvent>& triggers);
    
    // Определение уровня риска по проценту
    static RiskLevel GetRiskLevel(int percent);
    
    // Получение цвета для уровня риска (RGBA)
    static void GetRiskColor(RiskLevel level, float& r, float& g, float& b, float& a);
    static void GetRiskColorByPercent(int percent, float& r, float& g, float& b, float& a);
    
    // Получение текстового описания уровня риска
    static std::string GetRiskDescription(RiskLevel level);
    static std::string GetRiskDescriptionByPercent(int percent);

private:
    // Ограничение значения в диапазоне 0-100
    int ClampPercent(int value);
    
    // Применение понижающего коэффициента при большом количестве триггеров
    // (чтобы не было 1000% риска)
    int ApplyDiminishingReturns(int rawSum, int triggerCount);
};

} // namespace ForensicScanner
