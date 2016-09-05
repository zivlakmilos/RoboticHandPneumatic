#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include <QObject>

#include <QString>

class ZException
{
public:
    ZException(void);
    ZException(QString what);
    ~ZException(void);
    
    QString what(void) const throw();
    
private:
    QString m_what;
};

#endif // _EXCEPTION_H_
