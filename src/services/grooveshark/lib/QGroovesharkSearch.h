#ifndef QGROOVESHARKSEARCH_H
#define QGROOVESHARKSEARCH_H

#include "QGrooveshark.h"
#include <QObject>

class QGroovesharkSearch : public QObject
{
    Q_OBJECT
public:
    explicit QGroovesharkSearch(QObject *parent = 0);

    void searchResults(QGrooveshark &manager, QString query);
signals:

public slots:

};

#endif // QGROOVESHARKSEARCH_H
