#pragma once
#include <QMainWindow>
#include <memory>
#include "nezalezhnyk/cpu.hpp"
#include "nezalezhnyk/memory.hpp"
#include "register_model.hpp"

class QTableView;
class QPlainTextEdit;
class QLabel;
class QPushButton;
class QSpinBox;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onStep();
    void onRun();
    void onReset();
    void onLoadProgram();

private:
    void refreshView();

    static constexpr uint64_t kMemorySize = 1 << 20; // 1 МіБ для MVP

    std::unique_ptr<nzk::Memory> memory_;
    std::unique_ptr<nzk::Cpu> cpu_;

    RegisterModel* regModel_ = nullptr;
    QTableView* regView_ = nullptr;
    QPlainTextEdit* memView_ = nullptr;
    QLabel* pcLabel_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    QPushButton* stepBtn_ = nullptr;
    QPushButton* runBtn_ = nullptr;
    QPushButton* resetBtn_ = nullptr;
    QPushButton* loadBtn_ = nullptr;
    QSpinBox* runLimitBox_ = nullptr;
};