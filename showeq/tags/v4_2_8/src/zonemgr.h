/*
 * zonemgr.h
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Copyright 2001 Zaphod (dohpaz@users.sourceforge.net). All Rights Reserved.
 *
 * Contributed to ShowEQ by Zaphod (dohpaz@users.sourceforge.net) 
 * for use under the terms of the GNU General Public License, 
 * incorporated herein by reference.
 *
 */

#ifndef ZONEMGR_H
#define ZONEMGR_H

#include <qobject.h>
#include <qstring.h>

#include "everquest.h"

//----------------------------------------------------------------------
// forward declarations
class EQPacket;

class ZoneMgr : public QObject
{
  Q_OBJECT

 public:
  ZoneMgr(EQPacket* packet, QObject* parent = 0, const char* name =0);

  QString zoneNameFromID(uint32_t zoneId);
  bool isZoning() const { return m_zoning; }
  const QString& shortZoneName() const { return m_shortZoneName; }
  const QString& longZoneName() const { return m_longZoneName; }
  
 public slots:
  void saveZoneState(void);
  void restoreZoneState(void);

 protected slots:
  void zoneEntry(const ClientZoneEntryStruct* zsentry);
  void zoneEntry(const ServerZoneEntryStruct* zsentry);
  void zoneChange(const zoneChangeStruct* zoneChange, uint32_t, uint8_t);
  void zoneNew(const newZoneStruct* zoneNew, uint32_t, uint8_t);

 signals:
  void zoneBegin();
  void zoneBegin(const QString& shortZoneName);
  void zoneChanged(const QString& shortZoneName);
  void zoneEnd(const QString& shortZoneName, const QString& longZoneName);

 private:
  bool m_zoning;
  QString m_longZoneName;
  QString m_shortZoneName;
};

#endif // ZONEMGR
