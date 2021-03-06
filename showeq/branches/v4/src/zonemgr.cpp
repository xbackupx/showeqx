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
 * modified by Fee (fee@users.sourceforge.net)
 *
 */

#include <qfile.h>
#include <qdatastream.h>
#include <qregexp.h>

#include "zonemgr.h"
#include "packet.h"
#include "main.h"

//----------------------------------------------------------------------
// constants
static const char magicStr[5] = "zon2"; // magic is the size of uint32_t + a null
static const uint32_t* magic = (uint32_t*)magicStr;
const float defaultZoneExperienceMultiplier = 0.75;


// Sequence of signals on initial entry into eq from character select screen
// EQPacket                              ZoneMgr                       isZoning
// ----------                            -------                       --------
// zoneEntry(ClientZoneEntryStruct)      zoneBegin()                   true
// zoneEntry(ServerZoneEntryStruct)      zoneBegin(shortName)          false
// zoneNew(newZoneStruct)                zoneEnd(shortName, longName)  false
//
// Sequence of signals on when zoning from zone A to zone B
// EQPacket                              ZoneMgr                       isZoning
// ----------                            -------                       --------
// zoneChange(zoneChangeStruct, client)                                true
// zoneChange(zoneChangeStruct, server)  zoneChanged(shortName)        true
// zoneEntry(ClientZoneEntryStruct)      zoneBegin()                   false
// zoneEntry(ServerZoneEntryStruct)      zoneBegin(shortName)          false
// zoneNew(newZoneStruct)                zoneEnd(shortName, longName)  false
//
ZoneMgr::ZoneMgr(EQPacket* packet, QObject* parent, const char* name)
  : QObject(parent, name),
    m_zoning(false),
    m_zone_exp_multiplier(defaultZoneExperienceMultiplier),
    m_zonePointCount(0),
    m_zonePoints(0)
{
  m_shortZoneName = "unknown";
  m_longZoneName = "unknown";
  m_zoning = false;

  connect(packet, SIGNAL(zoneEntry(const ClientZoneEntryStruct*, uint32_t, uint8_t)),
	  this, SLOT(zoneEntry(const ClientZoneEntryStruct*, uint32_t, uint8_t)));
  connect(packet, SIGNAL(backfillPlayer(const charProfileStruct*, uint32_t, uint8_t)),
	  this, SLOT(zonePlayer(const charProfileStruct*)));
  connect(packet, SIGNAL(zoneEntry(const ServerZoneEntryStruct*, uint32_t, uint8_t)),
	  this, SLOT(zoneEntry(const ServerZoneEntryStruct*, uint32_t, uint8_t)));
  connect(packet, SIGNAL(zoneChange(const zoneChangeStruct*, uint32_t, uint8_t)),
	  this, SLOT(zoneChange(const zoneChangeStruct*, uint32_t, uint8_t)));
  connect(packet, SIGNAL(zoneNew(const newZoneStruct*, uint32_t, uint8_t)),
	  this, SLOT(zoneNew(const newZoneStruct*, uint32_t, uint8_t)));
  connect(packet, SIGNAL(zonePoints(const zonePointsStruct*, uint32_t, uint8_t)),
	  this, SLOT(zonePoints(const zonePointsStruct*, uint32_t, uint8_t)));

  if (showeq_params->restoreZoneState)
    restoreZoneState();
}

struct ZoneNames
{
  const char* shortName;
  const char* longName;
};

static const ZoneNames zoneNames[] =
{
#include "zones.h"
};

QString ZoneMgr::zoneNameFromID(uint16_t zoneId)
{
   const char* zoneName = NULL;
   if (zoneId < (sizeof(zoneNames) / sizeof (ZoneNames)))
       zoneName = zoneNames[zoneId].shortName;

   if (zoneName != NULL)
      return zoneName;

   QString tmpStr;
   tmpStr.sprintf("unk_zone_%d", zoneId);
   return tmpStr;
}

QString ZoneMgr::zoneLongNameFromID(uint16_t zoneId)
{

   const char* zoneName = NULL;
   if (zoneId < (sizeof(zoneNames) / sizeof (ZoneNames)))
       zoneName = zoneNames[zoneId].longName;

   if (zoneName != NULL)
      return zoneName;

   QString tmpStr;
   tmpStr.sprintf("unk_zone_%d", zoneId);
   return tmpStr;
}

void ZoneMgr::saveZoneState(void)
{
  QFile keyFile(showeq_params->saveRestoreBaseFilename + "Zone.dat");
  if (keyFile.open(IO_WriteOnly))
  {
    QDataStream d(&keyFile);
    // write the magic string
    d << *magic;

    d << m_longZoneName;
    d << m_shortZoneName;
  }
}

void ZoneMgr::restoreZoneState(void)
{
  QString fileName = showeq_params->saveRestoreBaseFilename + "Zone.dat";
  QFile keyFile(fileName);
  if (keyFile.open(IO_ReadOnly))
  {
    QDataStream d(&keyFile);

    // check the magic string
    uint32_t magicTest;
    d >> magicTest;

    if (magicTest != *magic)
    {
      fprintf(stderr, 
	      "Failure loading %s: Bad magic string!\n",
	      (const char*)fileName);
      return;
    }

    d >> m_longZoneName;
    d >> m_shortZoneName;

    fprintf(stderr, "Restored Zone: %s (%s)!\n",
	    (const char*)m_shortZoneName,
	    (const char*)m_longZoneName);
  }
  else
  {
    fprintf(stderr,
	    "Failure loading %s: Unable to open!\n", 
	    (const char*)fileName);
  }
}

void ZoneMgr::zoneEntry(const ClientZoneEntryStruct* zsentry, uint32_t len, uint8_t dir)
{
  m_shortZoneName = "unknown";
  m_longZoneName = "unknown";
  m_zone_exp_multiplier = defaultZoneExperienceMultiplier;
  m_zoning = false;

  emit zoneBegin();
  emit zoneBegin(zsentry, len, dir);

  if (showeq_params->saveZoneState)
    saveZoneState();
}

void ZoneMgr::zonePlayer(const charProfileStruct* player)
{
  m_shortZoneName = zoneNameFromID(player->zoneId);
  m_longZoneName = zoneLongNameFromID(player->zoneId);
  m_zone_exp_multiplier = defaultZoneExperienceMultiplier;
  m_zoning = false;
  emit zoneBegin(m_shortZoneName);

  if (showeq_params->saveZoneState)
    saveZoneState();
}


void ZoneMgr::zoneEntry(const ServerZoneEntryStruct* zsentry, uint32_t len, uint8_t dir)
{
  m_zoning = false;
  emit zoneBegin(m_shortZoneName);
  emit zoneBegin(zsentry, len, dir);

  if (showeq_params->saveZoneState)
    saveZoneState();
}

void ZoneMgr::zoneChange(const zoneChangeStruct* zoneChange, uint32_t len, uint8_t dir)
{
  m_shortZoneName = zoneNameFromID(zoneChange->zoneId);
  m_longZoneName = zoneLongNameFromID(zoneChange->zoneId);
  m_zoning = true;

  if (dir == DIR_SERVER)
    emit zoneChanged(m_shortZoneName);
    emit zoneChanged(zoneChange, len, dir);

  if (showeq_params->saveZoneState)
    saveZoneState();
}

void ZoneMgr::zoneNew(const newZoneStruct* zoneNew, uint32_t len, uint8_t dir)
{
  m_zone_exp_multiplier = zoneNew->zone_exp_multiplier;

  // ZBNOTE: Apparently these come in with the localized names, which means we 
  //         may not wish to use them for zone short names.  
  //         An example of this is: shortZoneName 'ecommons' in German comes 
  //         in as 'OGemeinl'.  OK, now that we have figured out the zone id
  //         issue, we'll only use this short zone name if there isn't one or
  //         it is an unknown zone.
  if (m_shortZoneName.isEmpty() || m_shortZoneName.startsWith("unk"))
  {
    m_shortZoneName = zoneNew->shortName;

    // LDoN likes to append a _262 to the zonename. Get rid of it.
    QRegExp rx("_\\d+$");
    m_shortZoneName.replace( rx, "");
  }

  m_longZoneName = zoneNew->longName;
  m_zoning = false;

#if 1 // ZBTEMP
  printf("Welcome to lovely downtown '%s' with an experience multiplier of %f\n",
	 zoneNew->longName, zoneNew->zone_exp_multiplier);
  printf("Safe Point (%f, %f, %f)\n", 
	 zoneNew->safe_x, zoneNew->safe_y, zoneNew->safe_z);
#endif // ZBTEMP
  
  // fprintf(stderr,"zoneNew: m_short(%s) m_long(%s)\n",
  //    (const char*)m_shortZoneName,
  //    (const char*)m_longZoneName);
  
  emit zoneEnd(m_shortZoneName, m_longZoneName);

  if (showeq_params->saveZoneState)
    saveZoneState();
}

void ZoneMgr::zonePoints(const zonePointsStruct* zp, uint32_t len, uint8_t)
{
  // note the zone point count
  m_zonePointCount = zp->count;

  // delete the previous zone point set
  if (m_zonePoints)
    delete [] m_zonePoints;
  
  // allocate storage for zone points
  m_zonePoints = new zonePointStruct[m_zonePointCount];

  // copy the zone point information
  memcpy((void*)m_zonePoints, zp, sizeof(zonePointStruct) * m_zonePointCount);
}

