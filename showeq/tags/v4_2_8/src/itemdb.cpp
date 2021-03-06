/*
 * itemdb.cpp
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 */

static char* itemdbid = "@(#) $Id$";

#include <unistd.h>
#include <stdio.h>

#include <qfileinfo.h>
#include <qstring.h>

#include "itemdb.h"

#ifdef USE_DB3
#include "gdbmconv.h"
#endif

#define UPGRADE_STATUS_UPDATE 100

// define the current version number for data in database
#define CURRENT_ITEM_FORMAT_VERSION 2

// Base Item Database Entry structure from which all EQItemDBEntryData
// structures inherit for identity reasons
struct EQItemDBEntryData
{
   int16_t m_entryFormatVersion;   // Specifies version of entry data format
   size_t m_entrySize;            // size of the whole database entry in bytes
};

struct EQItemDBEntryData_1 : public EQItemDBEntryData
{
 public:
  // public constructors (mostly for convenience)
  EQItemDBEntryData_1();
  EQItemDBEntryData_1(const struct itemStruct* item, uint16_t flag);
  EQItemDBEntryData_1(const EQItemDBEntryData &);
  
  // public member functions
  // initialize the DBEntryData using the itemStruct info
  void Init(const struct itemStruct* item, uint16_t flag);

  // initialize the DBEntryData using Datum from a database
  void Init(const Datum& data);

  // initialize the DBEntryData with empty/default values
  void Init();

  // public data members
  char   m_idfile[6];             // Not sure what this is used for, eg: IT63
  int16_t   m_flag;               // See itemStruct for values
  uint8_t  m_weight;              // Weight of item
  int8_t   m_nosave;              // Nosave flag 1=normal, 0=nosave, -1=spell?
  int8_t   m_nodrop;              // Nodrop flag 1=normal, 0=nodrop, -1=??
  uint8_t  m_size;                // Size of item
  uint16_t  m_iconNr;             // Icon Number
  uint32_t m_equipableSlots;      // Slots where this item goes
  int32_t  m_cost;                // Item cost in copper
  int8_t   m_STR;                 // Strength
  int8_t   m_STA;                 // Stamina
  int8_t   m_CHA;                 // Charisma
  int8_t   m_DEX;                 // Dexterity
  int8_t   m_INT;                 // Intelligence
  int8_t   m_AGI;                 // Agility
  int8_t   m_WIS;                 // Wisdom
  int8_t   m_MR;                  // Magic Resistance
  int8_t   m_FR;                  // Fire Resistance
  int8_t   m_CR;                  // Cold Resistance
  int8_t   m_DR;                  // Disease Resistance
  int8_t   m_PR;                  // Poison Resistance
  int8_t   m_HP;                  // Hitpoints
  int8_t   m_MANA;                // Mana
  int8_t   m_AC;                  // Armor Class
  uint8_t  m_light;               // Light effect of this item
  int8_t   m_delay;               // Weapon Delay
  int8_t   m_damage;              // Weapon Damage
  uint8_t  m_range;               // Range of weapon
  uint8_t  m_skill;               // Skill of this weapon
  int8_t   m_magic;               // Magic flag, 1(0001)=magic
  int8_t   m_level0;              // Casting level
  uint8_t  m_material;            // Material?
  uint32_t m_color;               // Amounts of RGB in original color
  uint16_t  m_spellId0;           // SpellID of special effect
  uint16_t  m_classes;            // Classes that can use this item
  uint16_t  m_races;              // Races that can use this item
  uint8_t  m_level;               // Casting level
  uint16_t  m_spellId;            // spellId of special effect
  int8_t   m_charges;             // number of charges of item effect (-1 = inf)
  uint8_t  m_numSlots;            // number of slots
  uint8_t  m_weightReduction;     // percentage weight reduction
  uint8_t  m_sizeCapacity;        // Maximum size item container can hold
};

struct EQItemDBEntryData_2 : public EQItemDBEntryData_1
{
 public:
  // public constructors (mostly for convenience)
  EQItemDBEntryData_2();
  EQItemDBEntryData_2(const struct itemStruct* item, uint16_t flag);
  EQItemDBEntryData_2(const EQItemDBEntryData &);
  
  // public member functions
  // initialize the DBEntryData using the itemStruct info
  void Init(const struct itemStruct* item, uint16_t flag);

  // initialize the DBEntryData using Datum from a database
  void Init(const Datum& data);

  // initialize the DBEntryData with empty/default values
  void Init();

  // public data members
  int8_t m_stackable;             // 1=stackable, 3=normal, 0=?
  int8_t m_effectType;            // 0=no, 1=click anywhere w/o class check,
                                  // 2=latent, 3=click anywhere EXPENDABLE,
                                  // 4=click worn, 5=click anywhere with
                                  // class check
  uint32_t m_castTime;            // cast time of effect
  uint16_t m_skillModId;          // ID of skill that item modifies
  int8_t m_skillModPercent;       // Percent that item modifies the skill
};

typedef EQItemDBEntryData_2 EQItemDBEntryData_Current;

////////////////////////////////////////////////////////////////////
// Life is easier if new versions can inherit from the immediate
// predecessor version.  Otherwise more work and is necessary in 
// the EQItemDBEntry class accessor methods and in the 
// EQItemDBEntryData child classes.  Therefore next one should be:
//  
// struct EQItemDBEntryData_3 : public EQItemDBEntryData_2
//

////////////////////////////////////////////////////////////////////
// Implementation of the EQItemDBEntryData class
EQItemDBEntryData_1::EQItemDBEntryData_1()
{
}

EQItemDBEntryData_1::EQItemDBEntryData_1(const struct itemStruct* item, uint16_t flag)
{
  // Just call initialize
  Init(item, flag);
}

EQItemDBEntryData_1::EQItemDBEntryData_1(const EQItemDBEntryData& item)
{
  // verify that the version is correct
  if ((item.m_entryFormatVersion == 1) &&
      (item.m_entrySize == sizeof(EQItemDBEntryData_1)))
  {
    // just copy the data into ourselves en-masse
    memcpy((void*)this, &item, sizeof(*this));
  }
  else // otherwise make it empty since nothing is valid
    memset((void*)this, 0, sizeof(*this));
}

void EQItemDBEntryData_1::Init(const struct itemStruct* item, uint16_t flag)
{
   // Start with a clean slate
   memset((void*)this, '\0', sizeof(*this));

   // Set the structure format version information
   m_entryFormatVersion = 1; 
  
   // note the size of the entry
   m_entrySize = sizeof(EQItemDBEntryData_1);

   // now start copying data as appropriate
   memcpy((void*)&m_idfile[0], (void*)item->idfile, sizeof(m_idfile));
   m_flag           = flag;
   m_weight         = item->weight;
   m_nosave         = item->nosave;
   m_nodrop         = item->nodrop;
   m_size           = item->size;
   m_iconNr         = item->iconNr;
   m_equipableSlots = item->equipableSlots;
   m_cost           = item->cost;

   // none of the following fields are valid for book items
   if ((m_flag != ITEM_BOOK) && (m_flag != ITEM_CONTAINER_PLAIN) &&
	 (m_flag != ITEM_CONTAINER))
   {
     const itemItemStruct* itemItem = (const itemItemStruct*)item;
     m_STR               = itemItem->STR;
     m_STA               = itemItem->STA;
     m_CHA               = itemItem->CHA;
     m_DEX               = itemItem->DEX;
     m_INT               = itemItem->INT;
     m_AGI               = itemItem->AGI;
     m_WIS               = itemItem->WIS;
     m_MR                = itemItem->MR;
     m_FR                = itemItem->FR;
     m_CR                = itemItem->CR;
     m_DR                = itemItem->DR;
     m_PR                = itemItem->PR;
     m_HP                = itemItem->HP;
     m_MANA              = itemItem->MANA;
     m_AC                = itemItem->AC;
     m_light             = itemItem->light;
     m_delay             = itemItem->delay;
     m_damage            = itemItem->damage;
     m_range             = itemItem->range;
     m_skill             = itemItem->skill;
     m_magic             = itemItem->magic;
     m_material          = itemItem->material;
     m_color             = itemItem->color;
     m_classes           = itemItem->classes;
     m_races             = itemItem->races;
     m_spellId0        = itemItem->spellId0;
     m_level0          = itemItem->level0;
     m_level           = itemItem->level;
     m_spellId         = itemItem->spellId;
     m_charges         = itemItem->charges;
   }
   else
   {
     m_spellId0          = ITEM_SPELLID_NOSPELL;
     m_spellId           = ITEM_SPELLID_NOSPELL;
   }
   // handle container vs. non-container special case
   if ((m_flag == ITEM_CONTAINER_PLAIN) ||
	 (m_flag == ITEM_CONTAINER))
   {
     const itemContainerStruct* container = (const itemContainerStruct*)item;
     m_numSlots        = container->numSlots;
     m_weightReduction = container->weightReduction;
     m_sizeCapacity    = container->sizeCapacity;
   }
}

void EQItemDBEntryData_1::Init()
{
   // Start with a clean slate
   memset((void*)this, '\0', sizeof(*this));

   // Set the structure format version information
   m_entryFormatVersion = 1; 
  
   // note the size of the entry
   m_entrySize = sizeof(EQItemDBEntryData_1);

   m_nodrop            = -1;
   m_nosave            = -1;
   m_spellId0          = ITEM_SPELLID_NOSPELL;
   m_spellId           = ITEM_SPELLID_NOSPELL;
}

void EQItemDBEntryData_1::Init(const Datum& data)
{
   // Start with a clean slate
   memset((void*)this, '\0', sizeof(*this));

   // Set the structure format version information
   m_entryFormatVersion = 1; 

   // by default copy the entire structure from the data
   size_t copySize = ((EQItemDBEntryData*)data.data)->m_entrySize;

   // but not if it's bigger then us...
   if (copySize > sizeof(EQItemDBEntryData_1))
     copySize = sizeof(EQItemDBEntryData_1);

   // now copy the entire other structure into ourselves
   memcpy((void*)this, data.data, copySize);
  
   // note the size of the entry
   m_entrySize = sizeof(EQItemDBEntryData_1);
}

// EQItemDBEntryData version 2
EQItemDBEntryData_2::EQItemDBEntryData_2()
{
}

EQItemDBEntryData_2::EQItemDBEntryData_2(const struct itemStruct* item, uint16_t flag)
{
  // Just call initialize
  Init(item, flag);
}

EQItemDBEntryData_2::EQItemDBEntryData_2(const EQItemDBEntryData& item)
{
  // verify that the version is correct
  if ((item.m_entryFormatVersion == 2) &&
      (item.m_entrySize == sizeof(EQItemDBEntryData_2)))
  {
    // just copy the data into ourselves en-masse
    memcpy((void*)this, &item, sizeof(*this));
  }
  else if ((item.m_entryFormatVersion == 1) &&
	   (item.m_entrySize == sizeof(EQItemDBEntryData_1)))
  {
    // just copy the data into ourselves en-masse
    memcpy((void*)this, &item, sizeof(EQItemDBEntryData_1));
    
    // and clear out the missing pieces
    m_stackable = 0;
    m_effectType = 0;
    m_castTime = 0;
    m_skillModId = 0;
    m_skillModPercent = 0;
  }
  else // otherwise make it empty since nothing is valid
    memset((void*)this, 0, sizeof(*this));
}

void EQItemDBEntryData_2::Init(const struct itemStruct* item, uint16_t flag)
{
  // initialize the underlying type
  EQItemDBEntryData_1::Init(item, flag);

   // Set the structure format version information
   m_entryFormatVersion = 2; 
  
   // note the size of the entry
   m_entrySize = sizeof(EQItemDBEntryData_2);
  
   // now start copying data as appropriate

   // none of the following fields are valid for book items
   if ((m_flag != ITEM_BOOK) && (m_flag != ITEM_CONTAINER_PLAIN) &&
	 (m_flag != ITEM_CONTAINER))
   {
     const itemItemStruct* itemItem = (const itemItemStruct*)item;
     m_stackable = itemItem->stackable;
     m_effectType = itemItem->effectType;
     m_castTime = itemItem->castTime;
     m_skillModId = itemItem->skillModId;
     m_skillModPercent = itemItem->skillModPercent;
   }
   else
   {
     m_stackable = 0;
     m_effectType = 0;
     m_castTime = 0;
     m_skillModId = 0;
     m_skillModPercent = 0;
   }
}

void EQItemDBEntryData_2::Init()
{
  EQItemDBEntryData_1::Init();
  
   // Set the structure format version information
   m_entryFormatVersion = 2; 
  
   // note the size of the entry
   m_entrySize = sizeof(EQItemDBEntryData_2);

   m_stackable = 0;
   m_effectType = 0;
   m_castTime = 0;
   m_skillModId = 0;
   m_skillModPercent = 0;
}

void EQItemDBEntryData_2::Init(const Datum& data)
{
   // Start with a clean slate
   memset((void*)this, '\0', sizeof(*this));

   // Set the structure format version information
   m_entryFormatVersion = 2; 

   // by default copy the entire structure from the data
   size_t copySize = ((EQItemDBEntryData*)data.data)->m_entrySize;

   // but not if it's bigger then us...
   if (copySize > sizeof(EQItemDBEntryData_2))
     copySize = sizeof(EQItemDBEntryData_2);

   // now copy the entire other structure into ourselves
   memcpy((void*)this, data.data, copySize);
  
   // note the size of the entry
   m_entrySize = sizeof(EQItemDBEntryData_2);
}

////////////////////////////////////////////////////////////////////
// Implementation of the EQItemDB class
EQItemDB::EQItemDB()
  : QObject(NULL, "eqitemdb")
{
  // What types of item data is saved
  m_dbTypesEnabled = (LORE_DB | NAME_DB | DATA_DB);

  // construct item data file names
  m_ItemNameDB = LOGDIR "/itemname";
  m_ItemLoreDB = LOGDIR "/itemlore";
  m_ItemDataDB = LOGDIR "/itemdata";
  m_ItemRawDataDB = LOGDIR "/itemrawdata";
}

EQItemDB::~EQItemDB()
{
}

QString EQItemDB::GetDBFile(int dbType)
{
  switch(dbType)
  {
    case LORE_DB:
      return m_ItemLoreDB;
    case NAME_DB:
      return m_ItemNameDB;
    case DATA_DB:
      return m_ItemDataDB;
    case RAW_DATA_DB:
      return m_ItemRawDataDB;
  };

  return "";
}
 
bool EQItemDB::SetDBFile(int dbType, const QString& dbFileName)
{
  bool result = true;

  switch(dbType)
  {
    case LORE_DB:
      m_ItemLoreDB = dbFileName;
      break;
    case NAME_DB:
      m_ItemNameDB = dbFileName;
      break;
    case DATA_DB:
      m_ItemDataDB = dbFileName;
      break;
    case RAW_DATA_DB:
      m_ItemRawDataDB = dbFileName;
      break;
    default:
      // no such DB, so can't set it.
      result = false;
  };

  return result;
}

int EQItemDB::GetEnabledDBTypes()
{
  return m_dbTypesEnabled;
}

bool EQItemDB::SetEnabledDBTypes(int dbTypes)
{
  // make sure argument is within the set of acceptable types
  if ((dbTypes & ~(LORE_DB | NAME_DB | DATA_DB | RAW_DATA_DB)) != 0)
    return false;

  m_dbTypesEnabled = dbTypes;

  return true;
}

static bool checkDestinationFile(const QString pfx, const QString fileName)
{
 // Get information about the file, if there is one
  QFileInfo fileInfo(fileName);

  // Get information about the directory the file should be in
  QFileInfo dirInfo(fileInfo.dirPath());

  if (!dirInfo.exists())
  {
    fprintf(stderr, 
	    "%s: Data Directory '%s' doesn't exist.\n",
	    (const char*)pfx, (const char*)dirInfo.absFilePath());
    
    // couldn't run upgrade because directory doesn't exist
    return false;
  }

  if (!dirInfo.isDir())
  {
    fprintf(stderr, 
	    "%s: Data Directory '%s' isn't a directory.\n",
	    (const char*)pfx, (const char*)dirInfo.absFilePath());
    
    // couldn't run upgrade because directory doesn't exist
    return false;
  }

  // make sure the directory is writable by us
  if (!dirInfo.isWritable())
  {
    fprintf(stderr, 
	    "%s: Data Directory '%s' isn't writable.\n",
	    (const char*)pfx, (const char*)dirInfo.absFilePath());
    
    // couldn't run upgrade because directory doesn't exist
    return false;
  }

  // if the db file exists, but isn't writable, we can't upgrade
  if (fileInfo.exists() && !fileInfo.isWritable())
  {
    fprintf(stderr, 
	    "%s: Data File '%s' isn't writable.\n",
	    (const char*)pfx, (const char*)fileInfo.absFilePath());
    
    // couldn't run upgrade because directory doesn't exist
    return false;
  }

  return true;
}

bool EQItemDB::Upgrade()
{
  int ret;
  QString newname;
  QString oldname;
  // upgrade existing data sources to new format
  QString gdbmExt = GDBMConvenience::extension();

#ifdef USE_DB3
  QString db3Ext = DB3Convenience::extension();
#endif
  QString destFile;
  bool goodDest = false;

  /////////////////////////////////////////////////////////
  // first check for the Item Data DB upgrade from old GDBM to whichever new

  // Get information about the file, if there is one
  QFileInfo fileInfo(m_ItemLoreDB + gdbmExt);

  // if the old item lore name file doesn't exist, then nothing to do...
  if (fileInfo.exists())
  { 
#ifdef USE_DB3
    destFile = m_ItemDataDB + db3Ext;
#else 
    destFile = m_ItemDataDB + gdbmExt;
#endif 

    // check to make sure the destination for the new format DB is correct
    goodDest = checkDestinationFile("EQItemDB::Upgrade()", destFile);

    if (!goodDest)
    {
      fprintf(stderr, 
	      "Destination for upgraded DB '%s' is bad.\n"
	      "\tAborting upgrade!\n", (const char*)destFile);
      return false;
    }

    int srcNum = 2;
    bool itemNameExists = false;
    bool itemDataExists = false;

    // let the user know what's going on...
    fprintf(stderr, "Upgrading item database file format\n");
    fprintf(stderr, "\tSource 1: %s\n", (const char*)(m_ItemLoreDB + gdbmExt));

    oldname = m_ItemNameDB + gdbmExt;
    fileInfo.setFile(oldname);
    itemNameExists = fileInfo.exists();
    if (itemNameExists)
      fprintf(stderr, "\tSource %d: %s\n", 
	      srcNum++, (const char*)(oldname));

    oldname = m_ItemDataDB + gdbmExt;
    fileInfo.setFile(oldname);
    itemDataExists = fileInfo.exists();
    if (itemDataExists)
      fprintf(stderr, "\tSource %d: %s\n", 
	      srcNum++, (const char*)(oldname));

    fprintf(stderr, "\tDestination: %s\n",(const char*)destFile);

    // our database convenience class
    GDBMConvenience* gdbmconv;

#ifdef USE_DB3
    gdbmconv = new GDBMConvenience;
#else
    gdbmconv = (GDBMConvenience*)this;
#endif
    
    // iterator over a database file
    GDBMIterator gdbmit;
    bool hasData;
    Datum key;
    Datum loredata, namedata, datadata, newdata;
    int itemNr;
    int loreSize;
    int nameSize;
    int count = 0;
    
    // Initialize the iterator on the lore file and retrieve the first key
    hasData = gdbmit.GetFirstKey(m_ItemLoreDB, key);
    
    // iterate for as long as there is data
    while (hasData)
    {
      // get current Item Number
      itemNr = *(uint16_t*)key.data;
      
      // retrieve data
      if (gdbmit.GetData(loredata))
      {
	// successfully retrieved lore name data
	// retrieve item name data
	if (itemNameExists)
	  gdbmconv->GetEntry(m_ItemNameDB, key, namedata);
	
	// retrieve item data data
	if (itemDataExists)
	  gdbmconv->GetEntry(m_ItemDataDB, key, datadata);
	
	// calculate the size of the existing strings (must have space for NULL)
	loreSize = (loredata.size != 0) ? loredata.size : 1;
	nameSize = (namedata.size != 0) ? namedata.size : 1;
	
	// calculate the size of the new data item
	newdata.size = sizeof(EQItemDBEntryData_Current) + loreSize + nameSize;
	
	// allocate storage buffer
	unsigned char databuffer[newdata.size];
	
	// copy any existing item data, or initialize it to defaults
	if (datadata.data != NULL)
	  ((EQItemDBEntryData_Current*)databuffer)->Init(datadata);
	else
	  ((EQItemDBEntryData_Current*)databuffer)->Init();
	
	// copy existing item lore name data, or put NULL in it's place
	if (loredata.data != NULL)
	  strncpy((char*)(databuffer + sizeof(EQItemDBEntryData_Current)),
		  (const char*)loredata.data, loredata.size);
	else
	  ((char*)(databuffer + sizeof(EQItemDBEntryData_Current)))[0] = '\0';
	
	// copy existing item name data, or put NULL in it's place
	if (namedata.data != NULL)
	  strncpy((char*)(databuffer + sizeof(EQItemDBEntryData_Current) + loreSize),
		  (const char*)namedata.data, namedata.size);
	else
	  ((char*)(databuffer + sizeof(EQItemDBEntryData_Current) + loreSize))[0] = '\0';
	
	// put databuffer into datum data pointer
	newdata.data = (void*)databuffer;
	
	// insert the new data into the item data db, replacing it if it exists
	Insert(m_ItemDataDB, key, newdata, true);
	
	// increment count
	count++;
	
	// print a notification every n items
	if ((count % UPGRADE_STATUS_UPDATE) == 0)
	  fprintf(stderr, "\t\tUpgraded %d items so far...\n", count);
	
	// release old data storage
	gdbmit.Release(loredata);
	gdbmconv->Release(namedata);
	gdbmconv->Release(datadata);
      }
      
      // release storage for the old key
      gdbmit.Release(key);
      
      // retrieve the next lore key
      hasData = gdbmit.GetNextKey(key);
    }

    // shutdown the iterator
    gdbmit.Done();
    
#ifdef USE_DB3
    // shutdown the GDBM convenience class
    gdbmconv->Shutdown();
  
    // don't need the GDBM convenience class anymore, delete it.
    delete gdbmconv;
#endif
    // let users know what's going on
    fprintf(stderr, "\t\tFlushing changes to disk.\n");

    // make sure everythings flushed to disk
    sync();

    // let users know what's going on
    fprintf(stderr, "\t\tRenaming old database files.\n");
    
    newname = m_ItemNameDB + gdbmExt + ".old";
    ret = rename((const char*)(m_ItemNameDB + gdbmExt), (const char*)newname);
    if (ret != 0)
    {
      fprintf(stderr, 
	      "EQItemDB::Upgrade(): Failed to rename '%s' to '%s'\n",
	      (const char*)(m_ItemNameDB + gdbmExt), (const char*)newname);
    }

    newname = m_ItemLoreDB + gdbmExt + ".old";
    ret = rename((const char*)(m_ItemLoreDB + gdbmExt), (const char*)newname);
    if (ret != 0)
    {
      fprintf(stderr, 
	      "EQItemDB::Upgrade(): Failed to rename '%s' to '%s'\n",
	      (const char*)(m_ItemLoreDB + gdbmExt), (const char*)newname);
    }

#ifdef USE_DB3
    newname = m_ItemDataDB + gdbmExt + ".old";
    ret = rename((const char*)(m_ItemDataDB + gdbmExt), (const char*)newname);
    if (ret != 0)
    {
      fprintf(stderr, 
	      "EQItemDB::Upgrade(): Failed to rename '%s' to '%s'\n",
	      (const char*)(m_ItemDataDB + gdbmExt), (const char*)newname);
    }
#endif

    fprintf(stderr, "Finished upgrading %d items in item database\n",
	    count);
  }

#ifdef USE_DB3
  /////////////////////////////////////////////////////////
  // now check for the Item Data DB upgrade
  oldname = m_ItemDataDB + gdbmExt;
  fileInfo.setFile(oldname);

  if (fileInfo.exists())
  {
    // construct the name of the destination file
    destFile = m_ItemDataDB + db3Ext;

    // check to make sure the destination for the new format DB is correct
    goodDest = checkDestinationFile("EQItemDB::Upgrade()", destFile);

    if (!goodDest)
    {
      fprintf(stderr, 
	      "Destination for upgraded DB '%s' is bad.\n"
	      "\tAborting upgrade!\n", (const char*)destFile);
      return false;
    }

    // let the user know what's going on...
    fprintf(stderr, "Upgrading item database file format\n");
    fprintf(stderr, "\tSource: %s\n", (const char*)oldname);
    fprintf(stderr, "\tDestination: %s\n",(const char*)destFile);

    // our database convenience class
    GDBMConvenience* gdbmconv;

    gdbmconv = new GDBMConvenience;
    
    // iterator over a database file
    GDBMIterator gdbmit;
    bool hasData;
    Datum key;
    Datum data;
    int count = 0;
    
    // Initialize the iterator on the lore file and retrieve the first key
    hasData = gdbmit.GetFirstKey(m_ItemDataDB, key);
    
    // iterate for as long as there is data
    while (hasData)
    {
      // retrieve data
      if (gdbmit.GetData(data))
      {
	// insert the new data into the item data db, replacing it if it exists
	Insert(m_ItemDataDB, key, data, true);
	
	// increment the count
	count++;
	
	// print a notification every n items
	if ((count % UPGRADE_STATUS_UPDATE) == 0)
	  fprintf(stderr, "\t\tUpgraded %d items so far...\n", count);

	// release the data
	gdbmit.Release(data);
      }
      
      // release thekey
      gdbmit.Release(key);
      
      // retrieve the next lore key
      hasData = gdbmit.GetNextKey(key);
    }

    // shutdown the iterator
    gdbmit.Done();

    // shutdown the GDBM convenience class
    gdbmconv->Shutdown();
  
    // don't need the GDBM convenience class anymore, delete it.
    delete gdbmconv;

    // let users know what's going on
    fprintf(stderr, "\t\tFlushing changes to disk.\n");

    // make sure everythings flushed to disk
    sync();

    // let users know what's going on
    fprintf(stderr, "\t\tRenaming old database file.\n");

    newname = m_ItemDataDB + gdbmExt + ".old";
    ret = rename((const char*)oldname, (const char*)newname);
    if (ret != 0)
    {
      fprintf(stderr, 
	      "EQItemDB::Upgrade(): Failed to rename '%s' to '%s'\n",
	      (const char*)oldname, (const char*)newname);
    }

    fprintf(stderr, "Finished upgrading %d items in item database\n",
	    count);
  }

  /////////////////////////////////////////////////////////
  // now check for the Raw Item Data DB upgrade
  oldname = m_ItemRawDataDB + gdbmExt;
  fileInfo.setFile(oldname);

  if (fileInfo.exists())
  {
    // construct the name of the destination file
    destFile = m_ItemRawDataDB + db3Ext;

    // check to make sure the destination for the new format DB is correct
    goodDest = checkDestinationFile("EQItemDB::Upgrade()", destFile);

    if (!goodDest)
    {
      fprintf(stderr, 
	      "Destination for upgraded DB '%s' is bad.\n"
	      "\tAborting upgrade!\n", (const char*)destFile);
      return false;
    }

    // let the user know what's going on...
    fprintf(stderr, "Upgrading item database file format\n");
    fprintf(stderr, "\tSource: %s\n", (const char*)oldname);
    fprintf(stderr, "\tDestination: %s\n",(const char*)destFile);

    // our database convenience class
    GDBMConvenience* gdbmconv;

    gdbmconv = new GDBMConvenience;
    
    // iterator over a database file
    GDBMIterator gdbmit;
    bool hasData;
    Datum key;
    Datum data;
    int count = 0;
    
    // Initialize the iterator on the lore file and retrieve the first key
    hasData = gdbmit.GetFirstKey(m_ItemRawDataDB, key);
    
    // iterate for as long as there is data
    while (hasData)
    {
      // retrieve data
      if (gdbmit.GetData(data))
      {
	// insert the new data into the item data db, replacing it if it exists
	Insert(m_ItemRawDataDB, key, data, true);
	
	// increment the count
	count++;
	
	// print a notification every n items
	if ((count % UPGRADE_STATUS_UPDATE) == 0)
	  fprintf(stderr, "\t\tUpgraded %d items so far...\n", count);

	// release the data
	gdbmit.Release(data);
      }
      
      // release thekey
      gdbmit.Release(key);
      
      // retrieve the next lore key
      hasData = gdbmit.GetNextKey(key);
    }

    // shutdown the iterator
    gdbmit.Done();

    // shutdown the GDBM convenience class
    gdbmconv->Shutdown();
  
    // don't need the GDBM convenience class anymore, delete it.
    delete gdbmconv;

    // let users know what's going on
    fprintf(stderr, "\t\tFlushing changes to disk.\n");

    // make sure everythings flushed to disk
    sync();

    // let users know what's going on
    fprintf(stderr, "\t\tRenaming old database file.\n");

    newname = m_ItemRawDataDB + gdbmExt + ".old";
    ret = rename((const char*)oldname, (const char*)newname);
    if (ret != 0)
    {
      fprintf(stderr, 
	      "EQItemDB::Upgrade(): Failed to rename '%s' to '%s'\n",
	      (const char*)oldname, (const char*)newname);
    }

    fprintf(stderr, "Finished upgrading %d items in raw item database\n",
	    count);
  }
#endif


  return true;
}

bool EQItemDB::AddItem(const itemStruct* item, 
		       uint32_t size, 
		       uint16_t flag,
		       bool update)
{
  Datum key, data;
  bool result, result2;

  // setup key datum to use for queries and insertions
  key.size = sizeof(item->itemNr);
  key.data = (void*)&item->itemNr;

  // calculate the size of the lore string
  size_t loreSize = strlen(item->lore) + 1;

  // calculate the size of the name string
  size_t nameSize = strlen(item->name) + 1;

  // calculate the size necessary for storage
  data.size = sizeof(EQItemDBEntryData_Current) + loreSize + nameSize;

  // allocate the storage buffer
  unsigned char databuffer[data.size];

  // initialize the EQItemDBEntryData_Current portion
  ((EQItemDBEntryData_Current*)databuffer)->Init(item, flag);

  // add lore string after EQItemDBEntryData_Current
  strncpy((char*)(databuffer + sizeof(EQItemDBEntryData_Current)),
	  item->lore, loreSize);

  // add name string after lore string
  strncpy((char*)(databuffer + sizeof(EQItemDBEntryData_Current) + loreSize),
	  item->name, nameSize);

  data.data = (void*)databuffer;

  result = Insert(m_ItemDataDB, key, data, update);

  // only add if database is enabled and no entry exists
  if (m_dbTypesEnabled & RAW_DATA_DB)
  {
    // setup datum to insert
    data.size = size;
    data.data = (void*)item;

    result2 = Insert(m_ItemRawDataDB, key, data, update);

    if (!result2)
      result = result2;
  }

  return result;
}

bool EQItemDB::DeleteItem(uint16_t itemNr)
{
  bool result = true;
  bool result2;
  Datum key;

  // setup key datum to use for queries and insertions
  key.size = sizeof(itemNr);
  key.data = (void*)&itemNr;
  
  // delete data from ItemDataDB
    result = Delete(m_ItemDataDB, key);

  // only delete if database is enabled
  if ((m_dbTypesEnabled & RAW_DATA_DB) &&
      IsEntryExist(m_ItemRawDataDB, key))
  {
    result2 = Delete(m_ItemRawDataDB, key);

    if (!result2)
      result = result2;
  }

  return result;
}

bool EQItemDB::ItemExist(uint16_t itemNr)
{
  Datum key;
  // setup key datum for query
  key.size = sizeof(itemNr);
  key.data = (void*)&itemNr;

  // so does the item exist in the Item Lore database
  return IsEntryExist(m_ItemDataDB, key);
}

QString EQItemDB::GetItemName(uint16_t itemNr)
{
  Datum key, data;
  QString result = "";

  // setup key datum for query
  key.size = sizeof(itemNr);
  key.data = (void*)&itemNr;

  // attempt to retrieve entry from the item name database
  if (GetEntry(m_ItemDataDB, key, data))
  {
    // get the size of the entry (can be different sizes in different versions)
    size_t entrySize = ((EQItemDBEntryData*)data.data)->m_entrySize;
    
    // calculate pointer to lore string
    const char* loreString = ((const char*)data.data) 
      + entrySize;
      
    // calculate pointer to name string
    result = ((const char*)data.data) + entrySize
      + 1 + strlen(loreString);
    
    // release the database memory
    Release(data);
  }

  return result;
}

QString EQItemDB::GetItemLoreName(uint16_t itemNr)
{
  Datum key, data;
  QString result = "";

  // setup key datum for query
  key.size = sizeof(itemNr);
  key.data = (void*)&itemNr;

  // attempt to retrieve entry from the item name database
  if (GetEntry(m_ItemDataDB, key, data))
  {
    // get the size of the entry (can be different sizes in different versions)
    size_t entrySize = ((EQItemDBEntryData*)data.data)->m_entrySize;
    
    // calculate pointer to lore string
    result = ((const char*)data.data) + entrySize;

    // release the database memory
    Release(data);
  }

  return result;
}

bool EQItemDB::GetItemData(uint16_t itemNr, class EQItemDBEntry** itemData)
{
  Datum key, data;
  bool result = false;

  // setup key datum for query
  key.size = sizeof(itemNr);
  key.data = (void*)&itemNr;

  // attempt to retrieve entry from the data database
  if (GetEntry(m_ItemDataDB, key, data))
  {
    // return a copy of the data
    if (itemData != NULL)
      *itemData = new EQItemDBEntry(itemNr, data.data);
    else
      Release(data); // if they don't want the data, just release it

    result = true;
  }
  else
  {
    // i no data, set itemData to NULL so they don't accidentally use it
    if (itemData != NULL)
      *itemData = (class EQItemDBEntry*)NULL;
  }  

  return result;
}

int EQItemDB::GetItemRawData(uint16_t itemNr, unsigned char** itemData)
{
  Datum key, data;
  int result = 0;

  // setup key datum for query
  key.size = sizeof(itemNr);
  key.data = (void*)&itemNr;

  // attempt to tretrieve entry from the raw data database
  if (GetEntry(m_ItemRawDataDB, key, data))
  {
    // return a copy of the data
    if (itemData != NULL) 
    {
      // allocate a memory to store the copy
      *itemData = new unsigned char[data.size];
      
      // copy the data (yes, it's obvious, but hey...)
      memcpy((void*)*itemData, (void*)data.data, data.size);
    }

    // result is the size of the data, whether or not it's returned
    result = data.size;

    // free the database copy of the memory
    Release(data);
  }
  else // if item is not found set itemData to NULL, just in case
    if (itemData != NULL)
      *itemData = (unsigned char*)NULL; 

  return result;
}

bool EQItemDB::ReorganizeDatabase(void)
{
  bool result, result2;

  // Reorganize Item Lore database
  result = Reorganize(m_ItemLoreDB);

  // Reorganize Item Name database if enabled
  if (m_dbTypesEnabled & NAME_DB)
  {
    result2 = Reorganize(m_ItemNameDB);
    
    if (!result2)
      result = result2;
  }

  // Reorganize Item Data database if enabled
  if (m_dbTypesEnabled & DATA_DB)
  {
    result2 = Reorganize(m_ItemDataDB);
    
    if (!result2)
      result = result2;
  }

  // Reorganize Item Raw Data database if enabled
  if (m_dbTypesEnabled & RAW_DATA_DB)
  {
    result2 = Reorganize(m_ItemRawDataDB);
    
    if (!result2)
      result = result2;
  }

  return result;
}

void EQItemDB::Shutdown()
{
#ifdef USE_DB3
  DB3Convenience::Shutdown();
#else
  GDBMConvenience::Shutdown();
#endif
}

const char* EQItemDB::Version()
{
  return itemdbid;
}

void EQItemDB::itemShop(const itemInShopStruct* items, uint32_t size, uint8_t)
{
  // really should check for the different types in itemType and use
  // the appropriate alternate sizes...
  switch (items->itemType)
  {
  case 0:
    if (size == sizeof(itemInShopStruct))
      AddItem(&items->item, sizeof(items->item), ITEM_NORMAL);
    break;
  case 1:
    if (size == (sizeof(itemInShopStruct) - sizeof(itemItemStruct) + 
		 sizeof(itemContainerStruct)))
      AddItem(&items->container, sizeof(items->container), ITEM_CONTAINER);
    break;
  case 2:
    if (size == (sizeof(itemInShopStruct) - sizeof(itemItemStruct) + 
		 sizeof(itemBookStruct)))
      AddItem(&items->book, sizeof(items->book), ITEM_BOOK);
    break;
  }
}

void EQItemDB::itemPlayerReceived(const itemOnCorpseStruct* itemc, uint32_t size, uint8_t)
{
  if (size == sizeof(itemOnCorpseStruct))
    AddItem(&itemc->item, sizeof(itemc->item), ITEM_NORMAL);
}

void EQItemDB::tradeItemOut(const tradeItemOutStruct* itemt, uint32_t size, uint8_t)
{
  switch (itemt->itemType)
  {
  case 0:
    if (size == sizeof(tradeItemOutStruct))
      AddItem(&itemt->item, sizeof(itemt->item), ITEM_NORMAL);
    break;
  case 1:
    if (size == (sizeof(tradeItemOutStruct) - sizeof(itemItemStruct) + 
		 sizeof(itemContainerStruct)))
    AddItem(&itemt->container, sizeof(itemt->container), ITEM_CONTAINER);
    break;
  case 2:
    if (size == (sizeof(tradeItemOutStruct) - sizeof(itemItemStruct) + 
		 sizeof(itemBookStruct)))
    AddItem(&itemt->book, sizeof(itemt->book), ITEM_BOOK);
    break;
  }
}

void EQItemDB::tradeItemIn(const tradeItemInStruct* itemr, uint32_t size, uint8_t)
{
  if (size == sizeof(tradeItemInStruct))
    AddItem(&itemr->item, sizeof(itemr->item), ITEM_NORMAL);
}

void EQItemDB::tradeContainerIn(const tradeContainerInStruct* itemr, uint32_t size, uint8_t)
{
  if (size == sizeof(tradeContainerInStruct))
    AddItem(&itemr->container, sizeof(itemr->container), ITEM_CONTAINER);
}

void EQItemDB::tradeBookIn(const tradeBookInStruct* itemr, uint32_t size, uint8_t)
{
  if (size == sizeof(tradeBookInStruct))
    AddItem(&itemr->item, sizeof(itemr->book), ITEM_BOOK);
}

void EQItemDB::summonedItem(const summonedItemStruct* items, uint32_t size, uint8_t)
{
  if (size == sizeof(summonedItemStruct))
    AddItem(&items->item, sizeof(items->item), ITEM_NORMAL);
}

void EQItemDB::summonedContainer(const summonedContainerStruct* items, uint32_t size, uint8_t)
{
  if (size == sizeof(summonedContainerStruct))
    AddItem(&items->container, sizeof(items->container), ITEM_CONTAINER);
}

void EQItemDB::playerItem(const playerItemStruct* itemp, uint32_t size, uint8_t)
{
  if (size == sizeof(playerItemStruct))
    AddItem(&itemp->item, sizeof(itemp->item), ITEM_NORMAL);
}

void EQItemDB::playerBook(const playerBookStruct* bookp, uint32_t size, uint8_t)
{
  if ((size == sizeof(playerBookStruct)) || 
      (size == (sizeof(playerBookStruct) - sizeof(itemBookStruct) +
		sizeof(itemItemStruct))))
    AddItem(&bookp->book, ITEM_BOOK);
  else
    fprintf(stderr, "Ooops!!!! playerBook()\n");
}

void EQItemDB::playerContainer(const playerContainerStruct* containp, uint32_t size, uint8_t)
{
  if ((size == sizeof(playerContainerStruct)) ||
      (size == (sizeof(playerContainerStruct) - sizeof(itemContainerStruct) + 
		sizeof(itemItemStruct))))
    AddItem(&containp->container, ITEM_CONTAINER);
  else 
    fprintf(stderr, "Ooops!!!! plyaerContainer()\n");
}

////////////////////////////////////////////////////////////////////
// Implementation of the EQItemDBIterator class
EQItemDBIterator::EQItemDBIterator(EQItemDB* pItemDB, int dbType)
: m_pItemDB(pItemDB),
  m_dbType(dbType)
{
}

EQItemDBIterator::~EQItemDBIterator()
{
  Done();
}

bool EQItemDBIterator::GetFirstItemNumber(uint16_t* itemNr)
{
  bool result;
  Datum key;

  // if they passed in an itemNr pointer, set it 0 in case of failure
  if (itemNr)
    *itemNr = 0;

  // if we weren't initialized with an ItemDB, then nothing to do...
  if (m_pItemDB == NULL)
    return false;

  // retrieve the filename of the database
  QString dbName = m_pItemDB->GetDBFile(m_dbType);

  // if there isn't a file, obviously nothing to iterate over
  if (dbName.isEmpty())
    return false;

  // retrieve the first key
#ifdef USE_DB3
  result = GetFirstKey(m_pItemDB, dbName, key);
#else
  result = GetFirstKey(dbName, key);
#endif

  if (result)
  {
    // save the item number for later use
    m_itemNr = *(uint16_t*)key.data;

    // if successful and the caller wants the number, set it
    if (itemNr != NULL)
      *itemNr = m_itemNr;
  }

  // release the key
  Release(key);

  return result;
}

bool EQItemDBIterator::GetNextItemNumber(uint16_t* itemNr)
{
  bool result;
  Datum nextKey;

  // Retrieve the next key  
  result = GetNextKey(nextKey);

  // did it succeed
  if (result)
  {
    // save the item number for later use if successful
    m_itemNr = *(uint16_t*)nextKey.data;

    // if the caller requested the item number, return it
    if (itemNr != NULL)
      *itemNr = m_itemNr;
  }
  else if (itemNr)
    itemNr = 0;
    
  // release the key
  Release(nextKey);

  return result;
}

bool EQItemDBIterator::GetItemData(QString& itemData)
{
  Datum data;
  bool result;
  itemData = "";

  // make sure to only return strings for DB's containing strings
  // otherwise don't bother
  if ((m_dbType != EQItemDB::LORE_DB) &&
      (m_dbType != EQItemDB::NAME_DB))
    return false;

  // retrieve the data
  result = GetData(data);
  
  if (result)
  {
    // success, set the query result
    itemData = (const char*)data.data;

    // release the database copy
    Release(data);
  }
  
  return result;
}

bool EQItemDBIterator::GetItemData(class EQItemDBEntry** itemData)
{
  Datum data;
  bool result = false;

  // if the user is requesting data, then set it to NULL just in case
  if (itemData != NULL)
    *itemData = (class EQItemDBEntry*)NULL;

  // don't bother searching if request is for data not stored in this DB
  if (m_dbType != EQItemDB::DATA_DB)
    return false;

  // retrieve the data
  result = GetData(data);

  // if something was found, return it
  if (result)
  {
    // return a copy of the data
    if (itemData != NULL)
      *itemData = new EQItemDBEntry(m_itemNr, data.data);
    else
      Release(data); // if they don't want the data, just release it
  }
  
  return result;
}

int EQItemDBIterator::GetItemData(unsigned char** itemData)
{
  Datum data;
  int result = 0;
  bool found = false;

  // retrieve the data
  found = GetData(data);

  if (found)
  { 
    // data was found, did the caller want the data?
    if (itemData != NULL) 
    {
      // allocate a copy of the data
      *itemData = new unsigned char[data.size];

      // copy the data
      memcpy((void*)*itemData, (void*)data.data, data.size);
    }

    // return the size if successful, irregardless of if the caller wanted data
    result = data.size;

    // release the databases copy of the data
    Release(data);
  }
  else 
  {
    // if the caller requested, and no data, set itemData to NULL for safety
    if (itemData != NULL)
      *itemData = (unsigned char*)NULL;
  }

  return result;
}

void EQItemDBIterator::Done()
{ 
  // Have the iterator do it's schtick
#ifdef USE_DB3
  DB3Iterator::Done();
#else
  GDBMIterator::Done();
#endif
}

////////////////////////////////////////////////////////////////////
// Implementation of the EQItemDBEntry class
EQItemDBEntry::EQItemDBEntry(uint16_t itemNr, 
                             void* entryData)
: m_itemEntryData((struct EQItemDBEntryData*)entryData),
  m_itemNr(itemNr)
  
{
  // lore name is stored after the entry
  m_itemLore = (((const char*)entryData) +  m_itemEntryData->m_entrySize);

  // and the item name is stored after that
  m_itemName = (((const char*)entryData) +  m_itemEntryData->m_entrySize 
		+ strlen(m_itemLore) + 1);
    
}

EQItemDBEntry::~EQItemDBEntry()
{
  if (m_itemEntryData)
    free(m_itemEntryData);
}

QString EQItemDBEntry::GetIdFile()
{
   return ((EQItemDBEntryData_1*)m_itemEntryData)->m_idfile;
}

int16_t EQItemDBEntry::GetFlag() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_flag; 
}

uint8_t EQItemDBEntry::GetWeight() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_weight; 
}

int8_t EQItemDBEntry::GetNoSave() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_nosave; 
}

int8_t EQItemDBEntry::GetNoDrop() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_nodrop; 
}

uint8_t EQItemDBEntry::GetSize() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_size; 
}

uint16_t EQItemDBEntry::GetIconNr() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_iconNr; 
}

uint32_t EQItemDBEntry::GetSlots() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_equipableSlots; 
}

int32_t  EQItemDBEntry::GetCost() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_cost; 
}

int8_t   EQItemDBEntry::GetSTR() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_STR; 
}

int8_t   EQItemDBEntry::GetSTA() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_STA; 
}

int8_t   EQItemDBEntry::GetCHA() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_CHA; 
}

int8_t   EQItemDBEntry::GetDEX() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_DEX; 
}

int8_t   EQItemDBEntry::GetINT() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_INT; 
}

int8_t   EQItemDBEntry::GetAGI() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_AGI; 
}

int8_t   EQItemDBEntry::GetWIS() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_WIS; 
}

int8_t   EQItemDBEntry::GetMR() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_MR; 
}

int8_t   EQItemDBEntry::GetFR() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_FR; 
}

int8_t   EQItemDBEntry::GetCR() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_CR; 
}

int8_t   EQItemDBEntry::GetDR() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_DR; 
}

int8_t   EQItemDBEntry::GetPR() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_PR; 
}

int8_t   EQItemDBEntry::GetHP() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_HP; 
}

int8_t   EQItemDBEntry::GetMana() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_MANA; 
}

int8_t   EQItemDBEntry::GetAC() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_AC; 
}

uint8_t  EQItemDBEntry::GetLight() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_light; 
}

uint8_t  EQItemDBEntry::GetDelay() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_delay; 
}

uint8_t  EQItemDBEntry::GetDamage() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_damage; 
}

uint8_t  EQItemDBEntry::GetRange() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_range; 
}

uint8_t  EQItemDBEntry::GetSkill() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_skill; 
}

int8_t   EQItemDBEntry::GetMagic() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_magic; 
}

int8_t   EQItemDBEntry::GetLevel0() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_level0; 
}

uint8_t  EQItemDBEntry::GetMaterial() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_material; 
}

uint32_t EQItemDBEntry::GetColor() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_color; 
}

uint16_t  EQItemDBEntry::GetSpellId0() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_spellId0; 
}

uint16_t  EQItemDBEntry::GetClasses() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_classes; 
}

uint16_t  EQItemDBEntry::GetRaces() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_races; 
}

uint8_t  EQItemDBEntry::GetLevel() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_level; 
}

uint16_t  EQItemDBEntry::GetSpellId() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_spellId; 
}

int8_t   EQItemDBEntry::GetCharges()
{
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_charges;
}

uint8_t  EQItemDBEntry::GetNumSlots() 
{ 
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_numSlots; 
}

uint8_t  EQItemDBEntry::GetWeightReduction()
{
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_weightReduction;
}

uint8_t  EQItemDBEntry::GetSizeCapacity()
{
  return ((EQItemDBEntryData_1*)m_itemEntryData)->m_sizeCapacity;
}

int8_t EQItemDBEntry::GetStackable()
{
  if (m_itemEntryData->m_entryFormatVersion >= 2)
    return ((EQItemDBEntryData_2*)m_itemEntryData)->m_stackable;
  else
    return -1;
}


int8_t EQItemDBEntry::GetEffectType()
{
  if (m_itemEntryData->m_entryFormatVersion >= 2)
    return ((EQItemDBEntryData_2*)m_itemEntryData)->m_effectType;
  else
    return -1;
}

QString EQItemDBEntry::GetEffectTypeString()
{
  static const char* effectNames[] = 
  {
    "No Effect",
    "Click anywhere w/o class check",
    "Latent/Worn",
    "Click anywhere expendable",
    "Click Worn",
    "Click anywhere w/ class check"
  };

  int effect = GetEffectType();
  if (effect == -1)
    return "Unknown (Old Data)";
  else if (effect < (int)(sizeof(effectNames) / sizeof (char*)))
    return effectNames[effect];
  else
    return QString::number(effect);
}

uint32_t EQItemDBEntry::GetCastTime()
{
  if (m_itemEntryData->m_entryFormatVersion >= 2)
    return ((EQItemDBEntryData_2*)m_itemEntryData)->m_castTime;
  else
    return 0;
}

uint16_t EQItemDBEntry::GetSkillModId()
{
  if (m_itemEntryData->m_entryFormatVersion >= 2)
    return ((EQItemDBEntryData_2*)m_itemEntryData)->m_skillModId;
  else
    return 0;
}

int8_t EQItemDBEntry::GetSkillModPercent()
{
  if (m_itemEntryData->m_entryFormatVersion >= 2)
    return ((EQItemDBEntryData_2*)m_itemEntryData)->m_skillModPercent;
  else
    return 0;
}

bool   EQItemDBEntry::IsBook()
{
  return (((EQItemDBEntryData_1*)m_itemEntryData)->m_flag == ITEM_BOOK);
}

bool   EQItemDBEntry::IsContainer()
{
  return ((((EQItemDBEntryData_1*)m_itemEntryData)->m_flag == ITEM_CONTAINER) ||
	  (((EQItemDBEntryData_1*)m_itemEntryData)->m_flag == ITEM_CONTAINER_PLAIN));
}
	  
