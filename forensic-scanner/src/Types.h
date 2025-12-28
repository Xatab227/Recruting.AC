#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <ctime>

namespace ForensicScanner {

// Категории триггеров
enum class TriggerCategory {
    Hash,
    Browser,
    Discord
};

// Типы триггеров
enum class TriggerType {
    HashMatch,          // Совпадение хеша с базой
    KeywordFound,       // Найдено ключевое слово
    SuspiciousURL,      // Подозрительный URL
    SuspiciousServer,   // Подозрительный сервер Discord
    SuspiciousChannel,  // Подозрительный канал Discord
    AuthAttempt         // Попытка авторизации
};

// Уровень риска
enum class RiskLevel {
    Low,        // 0-20%  - Зелёный
    Medium,     // 21-50% - Жёлтый  
    High        // 51-100% - Красный
};

// Структура триггера (события)
struct TriggerEvent {
    TriggerCategory category;
    TriggerType type;
    std::string source;         // Путь к файлу или источник
    std::string matchedValue;   // Найденное значение (хеш, слово, URL)
    std::string context;        // Контекст (часть строки)
    std::string details;        // Дополнительные детали
    int weight;                 // Вес триггера (влияние на %)
    std::time_t timestamp;      // Время обнаружения
    bool expanded = false;      // Развёрнут ли в UI
    
    TriggerEvent() : category(TriggerCategory::Hash), type(TriggerType::HashMatch),
                     weight(0), timestamp(0), expanded(false) {}
};

// Результаты сканирования
struct ScanResults {
    std::vector<TriggerEvent> hashTriggers;
    std::vector<TriggerEvent> browserTriggers;
    std::vector<TriggerEvent> discordTriggers;
    
    int totalRiskPercent = 0;
    RiskLevel riskLevel = RiskLevel::Low;
    
    int filesScanned = 0;
    int hashMatches = 0;
    int keywordsFound = 0;
    int suspiciousURLs = 0;
    int discordMatches = 0;
    
    bool scanComplete = false;
    float scanProgress = 0.0f;
    std::string currentOperation;
};

// Статистика сканирования
struct ScanStats {
    int totalFiles = 0;
    int processedFiles = 0;
    int matchesFound = 0;
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point endTime;
};

// Вспомогательные функции для работы с типами
inline std::string TriggerCategoryToString(TriggerCategory cat) {
    switch (cat) {
        case TriggerCategory::Hash: return "Хеши";
        case TriggerCategory::Browser: return "Браузер";
        case TriggerCategory::Discord: return "Discord";
        default: return "Неизвестно";
    }
}

inline std::string TriggerTypeToString(TriggerType type) {
    switch (type) {
        case TriggerType::HashMatch: return "Совпадение хеша";
        case TriggerType::KeywordFound: return "Ключевое слово";
        case TriggerType::SuspiciousURL: return "Подозрительный URL";
        case TriggerType::SuspiciousServer: return "Сервер Discord";
        case TriggerType::SuspiciousChannel: return "Канал Discord";
        case TriggerType::AuthAttempt: return "Попытка авторизации";
        default: return "Неизвестно";
    }
}

inline std::string RiskLevelToString(RiskLevel level) {
    switch (level) {
        case RiskLevel::Low: return "Низкий";
        case RiskLevel::Medium: return "Средний";
        case RiskLevel::High: return "Высокий";
        default: return "Неизвестно";
    }
}

} // namespace ForensicScanner
