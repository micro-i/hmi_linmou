#ifndef DBMODEL_H
#define DBMODEL_H

#include <QString>
#include <QVariant>
#include <QDateTime>

namespace Db {

// 产品表
struct Product
{
    quint32 productId;                 // 产品ID
    QString productName;                // 产品名称
    QString productEnName;              // 属性英文名
    QString description;                // 产品信息
    Product(){}
};
// 产品缺陷配置表
struct ProductGrade
{
    quint32 productId;
    quint32 gradeCode;
    QString algoName;
    QString custName;
    ProductGrade(){}
};
// 每次开始一个任务会创建一个数据
struct Task
{
    quint32 taskId;        // 任务ID
    quint32 productId;     // 产品ID
    QString batchNo;        // 任务详情
    QString orderDate;      // 创建时间
    QString description;    // 创建任务的软件详细信息
    Task() {}
};
// 产品缺陷分类
struct ProductClassfiedResult {
    quint32 taskId;
    quint32 productId;
    quint32 workpieceId = -1;
    QString sn;
    QString comment;
    QString defectCodeList;
    QString time;
    ProductClassfiedResult() {}
};
// 块缺陷分类
struct ProductModelInspectResult {
    quint32 taskId;
    quint32 productId;
    quint32 workpieceId;
    QString sn;
    QString comment;
    QString defectCodes;
    quint32 width;
    quint32 height;
    quint32 depth;
    quint32 widthId;
    quint32 heightId;
    quint32 depthId;
    QString dtype;
    QString time;
    float xMin;
    float yMin;
    float zMin;
    float xMax;
    float yMax;
    float zMax;
    float score;
    QString mask;
    ProductModelInspectResult(){}
};

}
#endif // DBMODEL_H
