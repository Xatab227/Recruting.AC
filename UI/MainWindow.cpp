#include "MainWindow.h"
#include "../src/Logger.h"
#include "../src/RiskCalculator.h"
#include "../src/Config.h"
#include "../src/HashScanner.h"
#include "../src/BrowserScanner.h"
#include "../src/DiscordScanner.h"
#include <QApplication>
#include <QMessageBox>
#include <QHeaderView>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QThread>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>

class ScannerThread : public QThread {
    Q_OBJECT
    
public:
    void run() override {
        int totalSteps = 4;
        int currentStep = 0;
        
        emit progressUpdated(0, totalSteps * 100);
        
        // Шаг 1: Сканирование хешей
        Config& config = Config::getInstance();
        HashScanner hashScanner;
        hashScanner.loadHashDatabase(config.getHashDatabasePath());
        
        auto directories = config.getScanDirectories();
        for (size_t i = 0; i < directories.size(); i++) {
            hashScanner.scanDirectory(directories[i], [this, &currentStep, totalSteps](int cur, int tot) {
                int progress = (currentStep * 100 + (cur * 100 / tot)) / totalSteps;
                emit progressUpdated(progress, totalSteps * 100);
            });
        }
        currentStep++;
        emit progressUpdated(currentStep * 100, totalSteps * 100);
        
        // Шаг 2: Сканирование браузера
        BrowserScanner browserScanner;
        auto browserPaths = config.getBrowserPaths();
        for (const auto& path : browserPaths) {
            browserScanner.scanBrowserHistory(path, [this, &currentStep, totalSteps](int cur, int tot) {
                int progress = (currentStep * 100 + (cur * 100 / tot)) / totalSteps;
                emit progressUpdated(progress, totalSteps * 100);
            });
        }
        currentStep++;
        emit progressUpdated(currentStep * 100, totalSteps * 100);
        
        // Шаг 3: Сканирование Discord
        DiscordScanner discordScanner;
        auto discordPaths = config.getDiscordPaths();
        for (const auto& path : discordPaths) {
            discordScanner.scanDiscordFiles(path, [this, &currentStep, totalSteps](int cur, int tot) {
                int progress = (currentStep * 100 + (cur * 100 / tot)) / totalSteps;
                emit progressUpdated(progress, totalSteps * 100);
            });
        }
        currentStep++;
        emit progressUpdated(currentStep * 100, totalSteps * 100);
        
        // Шаг 4: Запись логов в файл
        Logger::getInstance().flushToFile();
        currentStep++;
        emit progressUpdated(currentStep * 100, totalSteps * 100);
        
        emit scanCompleted();
    }
    
signals:
    void progressUpdated(int current, int total);
    void scanCompleted();
};

#include "MainWindow.moc"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), scannerThread(nullptr) {
    setupUI();
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::updateLogs);
}

MainWindow::~MainWindow() {
    if (scannerThread) {
        scannerThread->wait();
        delete scannerThread;
    }
}

void MainWindow::setupUI() {
    setWindowTitle("Forensic Scanner - Античит Сканер");
    setMinimumSize(1000, 700);
    
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    
    // Индикатор прогресса сканирования
    QLabel* progressLabel = new QLabel("Прогресс сканирования:", this);
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setTextVisible(true);
    
    mainLayout->addWidget(progressLabel);
    mainLayout->addWidget(progressBar);
    
    // Индикатор риска
    QLabel* riskTitleLabel = new QLabel("Общая оценка риска:", this);
    riskTitleLabel->setStyleSheet("font-size: 14pt; font-weight: bold;");
    
    riskBar = new QProgressBar(this);
    riskBar->setRange(0, 100);
    riskBar->setValue(0);
    riskBar->setTextVisible(true);
    riskBar->setFormat("%p%");
    riskBar->setStyleSheet("QProgressBar::chunk { background-color: green; }");
    
    riskLabel = new QLabel("0% - Низкий риск", this);
    riskLabel->setStyleSheet("font-size: 12pt; font-weight: bold;");
    
    mainLayout->addWidget(riskTitleLabel);
    mainLayout->addWidget(riskBar);
    mainLayout->addWidget(riskLabel);
    
    // Сводка
    summaryLabel = new QLabel("Триггеры: Хеши: 0 | Браузер: 0 | Discord: 0", this);
    summaryLabel->setStyleSheet("padding: 10px; background-color: #f0f0f0; border: 1px solid #ccc;");
    mainLayout->addWidget(summaryLabel);
    
    // Кнопка сканирования
    scanButton = new QPushButton("Начать сканирование", this);
    scanButton->setStyleSheet("padding: 10px; font-size: 12pt;");
    connect(scanButton, &QPushButton::clicked, this, &MainWindow::startScan);
    mainLayout->addWidget(scanButton);
    
    // Вкладки с логами
    logTabs = new QTabWidget(this);
    
    hashLogList = new QListWidget(this);
    browserLogList = new QListWidget(this);
    discordLogList = new QListWidget(this);
    
    logTabs->addTab(hashLogList, "Лог хешей");
    logTabs->addTab(browserLogList, "Лог браузера");
    logTabs->addTab(discordLogList, "Лог Discord");
    
    mainLayout->addWidget(logTabs);
}

void MainWindow::startScan() {
    if (scannerThread && scannerThread->isRunning()) {
        return;
    }
    
    scanButton->setEnabled(false);
    progressBar->setValue(0);
    
    Logger::getInstance().clear();
    
    scannerThread = new ScannerThread();
    connect(scannerThread, &ScannerThread::progressUpdated, this, &MainWindow::updateProgress);
    connect(scannerThread, &ScannerThread::scanCompleted, this, &MainWindow::scanFinished);
    
    scannerThread->start();
    updateTimer->start(500); // Обновление логов каждые 500мс
}

void MainWindow::updateProgress(int current, int total) {
    int percent = total > 0 ? (current * 100) / total : 0;
    progressBar->setValue(percent);
}

void MainWindow::updateRisk(int risk) {
    riskBar->setValue(risk);
    updateRiskColor(risk);
    
    int hashTriggers = RiskCalculator::getHashTriggers();
    int browserTriggers = RiskCalculator::getBrowserTriggers();
    int discordTriggers = RiskCalculator::getDiscordTriggers();
    
    summaryLabel->setText(QString("Триггеры: Хеши: %1 | Браузер: %2 | Discord: %3")
                          .arg(hashTriggers).arg(browserTriggers).arg(discordTriggers));
}

void MainWindow::updateRiskColor(int risk) {
    QString color;
    QString text;
    
    if (risk <= 20) {
        color = "green";
        text = QString("%1% - Низкий риск").arg(risk);
    } else if (risk <= 50) {
        color = "yellow";
        text = QString("%1% - Средний риск").arg(risk);
    } else if (risk <= 70) {
        color = "orange";
        text = QString("%1% - Повышенный риск").arg(risk);
    } else {
        color = "red";
        text = QString("%1% - Высокий риск").arg(risk);
    }
    
    riskBar->setStyleSheet(QString("QProgressBar::chunk { background-color: %1; }").arg(color));
    riskLabel->setText(text);
    riskLabel->setStyleSheet(QString("font-size: 12pt; font-weight: bold; color: %1;").arg(color));
}

void MainWindow::scanFinished() {
    scanButton->setEnabled(true);
    updateTimer->stop();
    updateLogs();
    updateRisk(RiskCalculator::calculateRisk());
    
    QMessageBox::information(this, "Сканирование завершено", 
                            "Сканирование системы завершено. Проверьте результаты в логах.");
}

void MainWindow::updateLogs() {
    hashLogList->clear();
    browserLogList->clear();
    discordLogList->clear();
    
    auto hashLogs = Logger::getInstance().getHashLogs();
    for (const auto& log : hashLogs) {
        QString itemText = QString("%1 | %2 | Риск: %3%")
                          .arg(log.triggerType.c_str())
                          .arg(log.source.c_str())
                          .arg(log.riskWeight);
        hashLogList->addItem(itemText);
    }
    
    auto browserLogs = Logger::getInstance().getBrowserLogs();
    for (const auto& log : browserLogs) {
        QString itemText = QString("%1 | %2 | Риск: %3%")
                          .arg(log.triggerType.c_str())
                          .arg(log.matchedValue.c_str())
                          .arg(log.riskWeight);
        browserLogList->addItem(itemText);
    }
    
    auto discordLogs = Logger::getInstance().getDiscordLogs();
    for (const auto& log : discordLogs) {
        QString itemText = QString("%1 | %2 | Риск: %3%")
                          .arg(log.triggerType.c_str())
                          .arg(log.matchedValue.c_str())
                          .arg(log.riskWeight);
        discordLogList->addItem(itemText);
    }
    
    updateRisk(RiskCalculator::calculateRisk());
}
