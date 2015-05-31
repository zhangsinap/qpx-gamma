/*******************************************************************************
 *
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 *
 * This software can be redistributed and/or modified freely provided that
 * any derivative works bear some notice that they are derived from it, and
 * any modified versions bear some notice that they have been modified.
 *
 * Description:
 *      QpxSpecialDelegate - displays colors, patterns, allows chosing of
 *      detectors.
 *
 * Author(s):
 *      Martin Shetty (NIST)
 *
 ******************************************************************************/

#include "special_delegate.h"
#include <QComboBox>
#include <QPainter>
#include <QDoubleSpinBox>

void QpxSpecialDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    if (index.data().canConvert<QpxPattern>()) {
        QpxPattern qpxPattern = qvariant_cast<QpxPattern>(index.data());
        if (option.state & QStyle::State_Selected)
            painter->fillRect(option.rect, option.palette.highlight());
        qpxPattern.paint(painter, option.rect, option.palette);
    } else if (index.data().type() == QVariant::Color) {
        QColor thisColor = qvariant_cast<QColor>(index.data());
        painter->fillRect(option.rect, thisColor);
    }
    else
        QStyledItemDelegate::paint(painter, option, index);

}

QSize QpxSpecialDelegate::sizeHint(const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    if (index.data().canConvert<QpxPattern>()) {
        QpxPattern qpxPattern = qvariant_cast<QpxPattern>(index.data());
        return qpxPattern.sizeHint();
    } else {
        return QStyledItemDelegate::sizeHint(option, index);
    }
}

QWidget *QpxSpecialDelegate::createEditor(QWidget *parent,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const

{
    if (index.data().type() == QVariant::String) {
        QComboBox *editor = new QComboBox(parent);
        editor->addItem(QString("none"));
        for (int i=0; i < detectors_.size(); i++)
            editor->addItem(QString::fromStdString(detectors_.get(i).name_));
        return editor;
    } else if (index.data().type() == QVariant::Double) {
        QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
        editor->setDecimals(6);
        editor->setRange(std::numeric_limits<double>::min(),std::numeric_limits<double>::max());
        return editor;
    } else {
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
}

void QpxSpecialDelegate::setEditorData ( QWidget *editor, const QModelIndex &index ) const
{
    if (QComboBox *cb = qobject_cast<QComboBox *>(editor)) {
        QString currentText = index.data(Qt::EditRole).toString();
        int cbIndex = cb->findText(currentText);
        if(cbIndex >= 0)
            cb->setCurrentIndex(cbIndex);
    } else if (QDoubleSpinBox *sb = qobject_cast<QDoubleSpinBox *>(editor)) {
        sb->setValue(index.data(Qt::EditRole).toDouble());
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

void QpxSpecialDelegate::setModelData ( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
    if (QComboBox *cb = qobject_cast<QComboBox *>(editor))
        model->setData(index, QVariant::fromValue(detectors_.get(cb->currentText().toStdString())), Qt::EditRole);
    else if (QDoubleSpinBox *sb = qobject_cast<QDoubleSpinBox *>(editor))
        model->setData(index, QVariant::fromValue(sb->value()), Qt::EditRole);
    else
        QStyledItemDelegate::setModelData(editor, model, index);
}

void QpxSpecialDelegate::eat_detectors(const XMLableDB<Pixie::Detector> &detectors) {
    detectors_ = detectors;
}