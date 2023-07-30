#include "signalselectdialog.h"

SignalSelectDialog::SignalSelectDialog(const CanDb &inputDb,
                                       QWidget *parent)
    : QDialog(parent),
      okButton(tr("OK"), this),
      cancelButton(tr("Cancel"), this),
      layout(this),
      buttonLayout(),
      db(inputDb)
{
    setWindowTitle("Signal select");
    buttonLayout.addWidget(&okButton);
    buttonLayout.addWidget(&cancelButton);
    layout.addWidget(&cmbMessage);
    layout.addWidget(&cmbSignal);
    layout.addLayout(&buttonLayout);

    connect(&okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(&cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(&cmbMessage, SIGNAL(currentIndexChanged(int)), this,
            SLOT(onMsgIndexChanged(int)));

    for (int i = 0; i < db.messageCount(); i++) {
        auto msg = db.at(i);
        cmbMessage.insertItem(i, msg.name);
    }
}

void SignalSelectDialog::onMsgIndexChanged(int index)
{
    auto msg = db.at(index);
    cmbSignal.clear();
    for (int i = 0; i < msg.canSignals.size(); i++) {
        cmbSignal.insertItem(i, msg.canSignals.at(i).name);
    }
}

QPair<int, int> SignalSelectDialog::getResultIndex()
{
    return { cmbMessage.currentIndex(), cmbSignal.currentIndex() };
}
