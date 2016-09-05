#include "zexception.h"

ZException::ZException(void)
    : ZException("Unknown exception")
{
}

ZException::ZException(QString what)
    : m_what(what)
{
}

ZException::~ZException(void)
{
}

QString ZException::what(void) const throw()
{
    return m_what;
}
