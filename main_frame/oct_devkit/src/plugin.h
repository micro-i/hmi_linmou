#ifndef PLUGIN_H
#define PLUGIN_H

#include <QObject>
#include <QVariant>

enum PLUGIN_TYPE {
    SYSTEM,
    EXTENSION
};

class Plugin : public QObject
{
    Q_OBJECT

public:
    Plugin() { }
    ~Plugin() { }

    virtual void settingsLoaded(QVariantMap settings) { }

    PLUGIN_TYPE getType() { return this->type; }
    void setType(PLUGIN_TYPE type) { this->type = type; }
    QString getName() { return this->name; }
    void setName(QString name) { this->name = name; }

protected:
    PLUGIN_TYPE type;
    QString name;
    QVariantMap settings;

public slots:
    void setKLinCoeffsRequestAccepted(int k0, int k1, int k2, int k3) { }
    void setDispCompCoeffsRequestAccepted(int d0, int d1, int d2, int d3) { }

signals:
    void info(QString info);
    void error(QString error);
    void storeSettings(QString name, QVariantMap settings);
    void setKLinCoeffsRequest(double* k0, double* k1, double* k2, double* k3);
    void setDispCompCoeffsRequest(double* d0, double* d1, double* d2, double* d3);
    void startProcessingRequest();
    void stopProcessingRequest();
};

#endif // PLUGIN_H
