#include "colorlisteditor.h"

ColorListEditor::ColorListEditor(QWidget *widget) : QComboBox(widget)
{
    populateList();
}

void ColorListEditor::populateList()
{
    const QStringList colorNames = QColor::colorNames();
    for (int i = 0; i < colorNames.size(); i++) {
        QColor color(colorNames[i]);
        insertItem(i, colorNames[i]);
        setItemData(i, color, Qt::DecorationRole);
    }
}

void ColorListEditor::setColor(const QColor &color)
{
    setCurrentIndex(findData(color, Qt::DecorationRole));
}

QColor ColorListEditor::color() const
{
    return qvariant_cast<QColor>(itemData(currentIndex(), Qt::DecorationRole));
}
