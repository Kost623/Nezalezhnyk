#include "mainwindow.hpp"

#include <QTableView>
#include <QHeaderView>
#include <QPlainTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>

#include "nezalezhnyk/isa.hpp"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    memory_ = std::make_unique<nzk::Memory>(kMemorySize);
    cpu_ = std::make_unique<nzk::Cpu>(*memory_);

    auto* central = new QWidget(this);
    auto* mainLayout = new QVBoxLayout(central);

    // --- панель керування ---
    auto* controls = new QHBoxLayout();
    loadBtn_ = new QPushButton(tr("Завантажити програму..."), this);
    stepBtn_ = new QPushButton(tr("Step"), this);
    runBtn_ = new QPushButton(tr("Run"), this);
    resetBtn_ = new QPushButton(tr("Reset"), this);
    runLimitBox_ = new QSpinBox(this);
    runLimitBox_->setRange(1, 10'000'000);
    runLimitBox_->setValue(100000);
    runLimitBox_->setPrefix(tr("ліміт: "));

    controls->addWidget(loadBtn_);
    controls->addWidget(stepBtn_);
    controls->addWidget(runBtn_);
    controls->addWidget(runLimitBox_);
    controls->addWidget(resetBtn_);
    controls->addStretch();

    pcLabel_ = new QLabel(this);
    statusLabel_ = new QLabel(this);
    auto* statusLayout = new QHBoxLayout();
    statusLayout->addWidget(pcLabel_);
    statusLayout->addWidget(statusLabel_);
    statusLayout->addStretch();

    // --- реєстри + пам'ять поруч ---
    auto* dataLayout = new QHBoxLayout();

    regModel_ = new RegisterModel(this);
    regModel_->setRegisters(&cpu_->registers());
    regView_ = new QTableView(this);
    regView_->setModel(regModel_);
    regView_->horizontalHeader()->setStretchLastSection(true);

    memView_ = new QPlainTextEdit(this);
    memView_->setReadOnly(true);
    memView_->setFont(QFont("monospace"));

    dataLayout->addWidget(regView_, 1);
    dataLayout->addWidget(memView_, 2);

    mainLayout->addLayout(controls);
    mainLayout->addLayout(statusLayout);
    mainLayout->addLayout(dataLayout, 1);

    setCentralWidget(central);
    resize(900, 600);
    setWindowTitle(tr("Nezalezhnyk — симулятор CPU"));

    connect(stepBtn_, &QPushButton::clicked, this, &MainWindow::onStep);
    connect(runBtn_, &QPushButton::clicked, this, &MainWindow::onRun);
    connect(resetBtn_, &QPushButton::clicked, this, &MainWindow::onReset);
    connect(loadBtn_, &QPushButton::clicked, this, &MainWindow::onLoadProgram);

    refreshView();
}

void MainWindow::onStep() {
    if (!cpu_->halted()) cpu_->step();
    refreshView();
}

void MainWindow::onRun() {
    cpu_->run(static_cast<uint64_t>(runLimitBox_->value()));
    refreshView();
}

void MainWindow::onReset() {
    memory_ = std::make_unique<nzk::Memory>(kMemorySize);
    cpu_ = std::make_unique<nzk::Cpu>(*memory_);
    regModel_->setRegisters(&cpu_->registers());
    refreshView();
}

void MainWindow::onLoadProgram() {
    const QString path = QFileDialog::getOpenFileName(this, tr("Обрати бінарний файл програми"));
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("Помилка"), tr("Не вдалося відкрити файл"));
        return;
    }
    const QByteArray raw = file.readAll();
    if (raw.size() % 4 != 0) {
        QMessageBox::warning(this, tr("Помилка"), tr("Розмір файлу не кратний 4 байтам (32-бітні інструкції)"));
        return;
    }

    std::vector<uint32_t> program;
    program.reserve(raw.size() / 4);
    for (int i = 0; i < raw.size(); i += 4) {
        uint32_t word = static_cast<uint8_t>(raw[i])
                       | (static_cast<uint8_t>(raw[i + 1]) << 8)
                       | (static_cast<uint8_t>(raw[i + 2]) << 16)
                       | (static_cast<uint8_t>(raw[i + 3]) << 24);
        program.push_back(word);
    }

    onReset();
    cpu_->loadProgram(program);
    refreshView();
}

void MainWindow::refreshView() {
    regModel_->refresh();

    pcLabel_->setText(tr("PC: 0x%1").arg(cpu_->pc(), 16, 16, QLatin1Char('0')));

    QString status = cpu_->halted() ? tr("зупинено") : tr("виконується");
    if (cpu_->breakpointHit()) status += tr(" (breakpoint)");
    status += tr(" | інструкцій виконано: %1").arg(cpu_->instructionsExecuted());
    statusLabel_->setText(status);

    // Дамп пам'яті навколо PC: по 8 слів до і після
    QStringList lines;
    const uint64_t center = cpu_->pc();
    const uint64_t start = (center >= 32) ? center - 32 : 0;
    for (uint64_t addr = start; addr < start + 64 && addr < memory_->size(); addr += 4) {
        const uint32_t word = memory_->fetchWord(addr);
        const nzk::Instruction instr = nzk::decode(word);
        const auto name = nzk::mnemonicName(instr.mnemonic);
        const QString marker = (addr == center) ? "-> " : "   ";
        lines << QString("%1 0x%2: 0x%3  %4")
                    .arg(marker)
                    .arg(addr, 8, 16, QLatin1Char('0'))
                    .arg(word, 8, 16, QLatin1Char('0'))
                    .arg(QString::fromUtf8(name.data(), static_cast<int>(name.size())));
    }
    memView_->setPlainText(lines.join('\n'));
}