#include "Utils/MDiscardableDialog.h"
#include "MerkaartorPreferences.h"

#include <QVBoxLayout>
#include <QApplication>
#include <QLabel>
#include <QSettings>

MDiscardableDialog::MDiscardableDialog(QWidget *parent, QString title)
    : QDialog(parent), mainWidget(0), Title(title)
{
    QSettings* Sets;
    if (!g_Merk_Portable) {
        Sets = new QSettings();
    } else {
        Sets = new QSettings(qApp->applicationDirPath() + "/merkaartor.ini", QSettings::IniFormat);
    }
    Sets->beginGroup("DiscardableDialogs");
    DiscardableRole = Sets->value(title, -1).toInt();

    setWindowTitle(title);
    setMinimumSize(300, 100);

    theLayout = new QVBoxLayout(this);
    theLayout->setSpacing(4);
    theLayout->setMargin(4);

    theDSA.setText(tr("Don't ask me this again"));
    theLayout->addWidget(&theDSA);

    delete Sets;
}

void MDiscardableDialog::setWidget ( QWidget * widget )
{
    if (mainWidget)
        theLayout->removeWidget(mainWidget);

    mainWidget = widget;
    mainWidget->setParent(this);
    theLayout->insertWidget(0, mainWidget);
}

QWidget* MDiscardableDialog::getWidget()
{
    mainWidget = new QWidget();
    mainWidget->setParent(this);

    theLayout->addWidget(mainWidget);

    return mainWidget;
}

int MDiscardableDialog::check()
{
    if (DiscardableRole != -1)
        return DiscardableRole;

    int tmpRet = exec();
    if (theDSA.isChecked()) {
        DiscardableRole = tmpRet;

        QSettings* Sets;
        if (!g_Merk_Portable) {
            Sets = new QSettings();
        } else {
            Sets = new QSettings(qApp->applicationDirPath() + "/merkaartor.ini", QSettings::IniFormat);
        }
        Sets->beginGroup("DiscardableDialogs");
        Sets->setValue(Title, DiscardableRole);
        delete Sets;
    }

    return tmpRet;
}

/* MDiscardableMessage */

MDiscardableMessage::MDiscardableMessage(QWidget *parent, QString title, QString msg)
    : MDiscardableDialog(parent, title)
{
    theBB.setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No);
    theLayout->addWidget(&theBB);

    connect(&theBB, SIGNAL(accepted()), this, SLOT(accept()));
    connect(&theBB, SIGNAL(rejected()), this, SLOT(reject()));

    QLabel * txt = new QLabel();
    txt->setText(msg);
    setWidget(txt);
}

