#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include <QObject>

#include <QString>

class Exception : public QObject
{
    Q_OBJECT
    
public:
    Exception(QObject *parent = parent);
    Exception(QString what, QObject *parent = 0);
    ~Exception(void);
    
    QString what(void) const throw();
    
private:
    QString m_what;
};

#endif // _EXCEPTION_H_
