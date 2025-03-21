#include "messageconsole.h"

#include <QGridLayout>
#include <QDateTime>
#include <QDebug>
#include <QMenu>
#include <QTextBlock>

#include "infrastructure/CCBaseMacro.h"

MessageConsole::MessageConsole(QWidget *parent) : QWidget(parent){
    // ui part
    auto gridLayout = new QGridLayout(this);
    gridLayout->setMargin(0);
    this->setMinimumHeight(160);
    this->m_messageEdit = new QTextEdit(this);
    this->m_messageEdit->setReadOnly(true);
    this->m_messageEdit->setContextMenuPolicy(Qt::NoContextMenu);

    connect(this->m_messageEdit, &QTextEdit::textChanged, this, [&]{
       m_messageEdit->document()->lastBlock().blockFormat().setLineHeight(10, QTextBlockFormat::ProportionalHeight);
    });
    gridLayout->addWidget(this->m_messageEdit, 0, 0, 1, 1);
    // init messages
    this->m_currentMsgIdx = 0;
}

MessageConsole::~MessageConsole(){

}

void MessageConsole::setParams(const MessageParams& params) {
    this->m_msgParams = params;
    // update params
}

MessageConsole::MessageParams MessageConsole::getParams()
{
    return this->m_msgParams;
}

void MessageConsole::contextMenuEvent(QContextMenuEvent* event) {
    QMenu* menu =this->m_messageEdit->createStandardContextMenu();
    menu->exec(event->globalPos());
    delete menu;
}

void MessageConsole::displayInfo(QString info) {
    QString currentTime = QDateTime::currentDateTime().toString("hh:mm:ss") + " ";
    //#4863A0 = "Steel Blue", RGB: 72, 99, 160, (http://www.computerhope.com/htmcolor.htm#color-codes)
    QString htmlStart = "<font color=\"#4863A0\">";
    QString htmlEnd = "</font>";

    qDebug() << info;
    this->m_messageEdit->moveCursor(QTextCursor::Start);
    this->m_messageEdit->insertHtml(currentTime + htmlStart + info + htmlEnd + "<br>");
    this->m_messageEdit->moveCursor(QTextCursor::Start);
    this->m_messageEdit->ensureCursorVisible();
}

void MessageConsole::displayError(QString error) {
    QString currentTime = QDateTime::currentDateTime().toString("hh:mm:ss") + " ";
    //#E41B17 = "Love Red" (http://www.computerhope.com/htmcolor.htm#color-codes)
    QString htmlStart = "<font color=\"#E41B17\">";
    QString htmlEnd = "</font>";

    qWarning() << error;
    this->m_messageEdit->moveCursor(QTextCursor::Start);
    this->m_messageEdit->insertHtml(currentTime + htmlStart + error + htmlEnd + "<br>");
    this->m_messageEdit->moveCursor(QTextCursor::Start);
    this->m_messageEdit->ensureCursorVisible();
}
