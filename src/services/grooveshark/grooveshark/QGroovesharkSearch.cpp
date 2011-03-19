//#include "QGrooveshark.h"
#include "QGroovesharkSearch.h"

QGroovesharkSearch::QGroovesharkSearch(QObject *parent) :
    QObject(parent)
{
}

void QGroovesharkSearch::searchResults(QGrooveshark &manager, QString query) {
    // function: Get search results playlist from query
    QVariantMap jParameters;
    jParameters.insert("query", query);
    jParameters.insert("type", "Songs");

    manager.makeRequest(QString("getSearchResultsEx"), jParameters);
}
