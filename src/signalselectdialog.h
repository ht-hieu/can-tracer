#pragma once
#include <QDialog>
#include <QComboBox>
#include <QPushButton>
#include <memory>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPair>
#include "canmsg.h"

class SignalSelectDialog : public QDialog
{
    Q_OBJECT
public:
    SignalSelectDialog(const CanDb &db, QWidget *parent = nullptr);
    QPair<int, int> getResultIndex();

public slots:
    void onMsgIndexChanged(int index);

private:
    QComboBox cmbMessage;
    QComboBox cmbSignal;
    QPushButton okButton;
    QPushButton cancelButton;
    QVBoxLayout layout;
    QHBoxLayout buttonLayout;
    const CanDb& db;
};
