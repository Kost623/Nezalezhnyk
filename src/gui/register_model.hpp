#pragma once
#include <QAbstractTableModel>
#include "nezalezhnyk/register_file.hpp"

class RegisterModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit RegisterModel(QObject* parent = nullptr) : QAbstractTableModel(parent) {}

    void setRegisters(const nzk::RegisterFile* regs) {
        beginResetModel();
        regs_ = regs;
        endResetModel();
    }

    // Викликається після кожного step()/run(), щоб перемалювати значення
    void refresh() {
        emit dataChanged(index(0, 1), index(rowCount() - 1, 1));
    }

    int rowCount(const QModelIndex& = QModelIndex()) const override { return 32; }
    int columnCount(const QModelIndex& = QModelIndex()) const override { return 2; }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if (role != Qt::DisplayRole) return {};
        if (orientation == Qt::Horizontal)
            return section == 0 ? QStringLiteral("Регістр") : QStringLiteral("Значення (hex)");
        return QString("x%1").arg(section);
    }

    QVariant data(const QModelIndex& idx, int role) const override {
        if (!regs_ || role != Qt::DisplayRole) return {};
        const int reg = idx.row();
        if (idx.column() == 0) return QString("x%1").arg(reg);
        return QString("0x%1").arg(regs_->read(static_cast<uint8_t>(reg)), 16, 16, QLatin1Char('0'));
    }

private:
    const nzk::RegisterFile* regs_ = nullptr;
};