#include "model.h"

Model::Model(QObject *parent) :
    QObject(parent),
    id(QUuid::createUuid())
{
    qObject = this;
}

