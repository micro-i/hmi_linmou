#ifndef EXTENSION_H
#define EXTENSION_H

#include <QObject>
#include "acquisitionsystem.h"

enum DISPLAY_STYLE {
    SIDEBAR_TAB,
    SEPARATE_WINDOW
};

class Extension : public Plugin
{
    Q_OBJECT
public:
    Extension();
    ~Extension();

    // UHROCT调用扩展部分的显示形式，是TAB还是总是显示在上方的Window
    DISPLAY_STYLE getDisplayStyle() { return this->displayStyle; }
    // 显示扩展部分工具提示
    QString getToolTip() { return this->toolTip; }
    // 返回扩展部分的GUI
    virtual QWidget* getWidget() = 0;
    // 启动/停用扩展部分
    virtual void activateExtension() = 0;
    virtual void deactivateExtension() = 0;

protected:
    // rawDataReceived()是否可以被调用
    bool rawGrabbingAllowed;
    // processedDataReceived()是否可以被调用
    bool processedGrabbingAllowed;
    QWidget* extensionWidget;
    DISPLAY_STYLE displayStyle;
    QString toolTip;

public slots:
    // 每次有rawData可用时调用, buffer-缓冲区的rawData
    virtual void rawDataReceived(void* buffer, unsigned int bitDepth, unsigned int samplesPerLine, unsigned int alinePerBscan, unsigned int bscanPerBuffer, unsigned int buffersPerVolume, unsigned int currentBufferNr) = 0;
    // 处理后的数据可用时调用
    virtual void processedDataReceived(void* buffer, unsigned int bitDepth, unsigned int samplesPerLine, unsigned int alinePerBscan, unsigned int bscanPerBuffer, unsigned int buffersPerVolume, unsigned int currentBufferNr) = 0;
    void enableRawDataGrabbing(bool enabled) { this->rawGrabbingAllowed = enabled; }
    void enableProcessedDataGrabbing(bool enabled) { this->processedGrabbingAllowed = enabled; }
signals:

};

#define Extension_iid "MICROIOCT.extension.interface"
Q_DECLARE_INTERFACE(Extension, Extension_iid)

#endif // EXTENSION_H
