#ifndef ACQUISITIONPARAMETER_H
#define ACQUISITIONPARAMETER_H

#include <QDebug>
#include <QObject>

struct AcquisitionParams {
    //// ODOCT system features
    bool ODSystemFlag; // flag if the system is ODOCT acquisition system
    unsigned int samplesPerFrame;  // samples in one snap, equal to samplesPerAline in SDOCT
    float* envMod;
    float* speDark;
    unsigned int* idxLine;
    float* resamplingCurve;

    // for calculating coordinates in x/y/z direction
    double lengthX;
    double lengthY;
    double lengthZ;

    QString procFilePath;

    // Aline采样数
    unsigned int samplesPerAline;
    // 每个Bscan有多少个Aline
    unsigned int alinesPerBscan;
    // 每次缓存多少个Bscan
    unsigned int bscansPerBuffer;
    // 每个volume需要的缓存次数
    unsigned int buffersPerVolume;
    // 每个采样数据的位深
    unsigned int bitDepth;
};

class AcquisitionParameter : public QObject
{
    Q_OBJECT
public:
    explicit AcquisitionParameter(QObject *parent = nullptr);
    ~AcquisitionParameter();

    AcquisitionParams params;

public slots:
    void slot_updateParams(AcquisitionParams newParams);

signals:
    void updated(AcquisitionParams newParams);

};

#endif // ACQUISITIONPARAMETER_H
