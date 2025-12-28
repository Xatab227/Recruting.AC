#include "RiskCalculator.h"
#include "Logger.h"

int RiskCalculator::calculateRisk() {
    int totalRisk = 0;
    
    auto hashLogs = Logger::getInstance().getHashLogs();
    auto browserLogs = Logger::getInstance().getBrowserLogs();
    auto discordLogs = Logger::getInstance().getDiscordLogs();
    
    for (const auto& log : hashLogs) {
        totalRisk += log.riskWeight;
    }
    
    for (const auto& log : browserLogs) {
        totalRisk += log.riskWeight;
    }
    
    for (const auto& log : discordLogs) {
        totalRisk += log.riskWeight;
    }
    
    // Ограничиваем диапазон 0-100%
    if (totalRisk > 100) {
        totalRisk = 100;
    }
    if (totalRisk < 0) {
        totalRisk = 0;
    }
    
    return totalRisk;
}

int RiskCalculator::getHashTriggers() {
    return Logger::getInstance().getHashLogs().size();
}

int RiskCalculator::getBrowserTriggers() {
    return Logger::getInstance().getBrowserLogs().size();
}

int RiskCalculator::getDiscordTriggers() {
    return Logger::getInstance().getDiscordLogs().size();
}
