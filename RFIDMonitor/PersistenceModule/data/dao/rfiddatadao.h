// ****************************************************
// * Codigo Generado por DAOBuilder                   *
// * Fecha: 30/07/2013 10:33:09                       *
// * Luis Valdes 2012 - Todos los Derechos Reservados *
// ****************************************************
#ifndef RFIDDATADAO_H
#define RFIDDATADAO_H

#include <QList>
#include <QMutex>

#include "../../core/genericdao.h"

class Rfiddata;

/*!
 * \brief The RfiddataDAO class is responsible to manipulate the persistence
 * of Rfiddata object in the database.
 */
class  RfiddataDAO : public GenericDAO<Rfiddata>
{
	Q_OBJECT

public:
    RfiddataDAO(QObject *parent = 0);

	static RfiddataDAO * instance();

    bool insertObject(Rfiddata *obj);
    bool updateObject(Rfiddata *obj);
    bool updateObjectList(const QList<Rfiddata *> &list);
    bool deleteObject(Rfiddata *rfiddata);

    Rfiddata * getById(qlonglong id, QObject *parent=0);

    QList<Rfiddata *> getAll(QObject *parent=0);

    QList<Rfiddata *> getByMatch(const QString &columnName, QVariant value, QObject *parent=0);

private:
    QString m_module;
    QMutex m_mutex;

};

#endif // RFIDDATADAO_H
