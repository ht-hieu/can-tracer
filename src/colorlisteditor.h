#pragma once

#include <QComboBox>

class ColorListEditor : public QComboBox
{
    Q_OBJECT
public:
    ColorListEditor(QWidget *widget = nullptr);
    void setColor(const QColor &color);
    QColor color() const;

private:
    void populateList();
};
