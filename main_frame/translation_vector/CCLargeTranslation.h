#ifndef CCLARGETRANSLATION_H
#define CCLARGETRANSLATION_H
#include <QObject>
#include "vectormove.h"

class CCLargeTranslation : public QObject
{
    Q_OBJECT
public:
    CCLargeTranslation();
    ~CCLargeTranslation();
    CCLargeTranslation(const CCLargeTranslation&) = delete;
    void generateMove(int step);
    int getStatus();
    void stop();
private:
    vectorMove m_vectorMove;

};

#endif // CCLARGETRANSLATION_H
