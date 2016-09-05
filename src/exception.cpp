#include "exception.h"

Exception::Exception(QString *parent)
    : Exception("Unknown exception", parent)
{
}

Exception::Exception(QString what, QString *parent)
    : QObject(parent)
{
}

QString Exception::what(void) const throw()
{
    return m_what;
}
