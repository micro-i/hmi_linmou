#ifndef MESSAGECONSOLE_H
#define MESSAGECONSOLE_H

#include <QTextEdit>
#include <QContextMenuEvent>

class MessageTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit MessageTextEdit(QWidget *parent = nullptr);
    ~MessageTextEdit();
private:

};

class MessageConsole : public QWidget
{
	Q_OBJECT
public:
    struct MessageParams {

    };

	explicit MessageConsole(QWidget *parent = nullptr);
	~MessageConsole();
    // set Param
    void setParams(const MessageParams& params);
    // get Param
    MessageParams getParams();

private:
    QTextEdit*          m_messageEdit;   // show message
    MessageParams       m_msgParams;     // params 预留
    int                 m_currentMsgIdx; // msg index
private:
	void contextMenuEvent(QContextMenuEvent* event) override;

signals:
	void error(QString);
	void info(QString);

public slots:
	void displayInfo(QString info);
	void displayError(QString error);
};

#endif // MESSAGECONSOLE_H
