#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressBar>
#include <QTextEdit>
#include <QTabWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QThread>
#include <QTimer>

class ScannerThread;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void startScan();
    void updateProgress(int current, int total);
    void updateRisk(int risk);
    void scanFinished();
    void updateLogs();

private:
    void setupUI();
    void updateRiskColor(int risk);
    
    QProgressBar* progressBar;
    QProgressBar* riskBar;
    QLabel* riskLabel;
    QLabel* summaryLabel;
    QPushButton* scanButton;
    QTabWidget* logTabs;
    QListWidget* hashLogList;
    QListWidget* browserLogList;
    QListWidget* discordLogList;
    
    ScannerThread* scannerThread;
    QTimer* updateTimer;
};

#endif // MAINWINDOW_H
