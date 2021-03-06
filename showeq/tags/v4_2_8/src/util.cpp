/*
 * util.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include <qcolor.h>
#include <qfileinfo.h>
#include <qdir.h>

#include "util.h"
#include "main.h"

static const spellInfoStruct spellInfo[] =
{
#include "spells.h"
};

/*
 * Generate comma separated string from long int
 */
QString
Commanate(uint32_t number)
{
   uint32_t	threeDigits;
   QString      oldstring;
   QString      newstring;
   QString      buffer;

   if (number >= 1000L)
     oldstring = Commanate(number / 1000L);

   threeDigits = number % 1000L;
   if (oldstring.isEmpty())
     buffer.sprintf("%u", threeDigits);
   else
     buffer.sprintf("%03u", threeDigits);

   if (oldstring.isEmpty())
     newstring = buffer;
   else
     newstring = oldstring + "," + buffer;

   return newstring;
} /* END Commanate */

QString
spell_name (uint16_t spellId)
{
   if (spellId < (sizeof(spellInfo) / sizeof(spellInfoStruct)))
     return spellInfo[spellId].name;
   else
     return QString::number(spellId, 16);
}

const spellInfoStruct*
spell_info (uint16_t spellId)
{
   static struct spellInfoStruct fake = { "Unknown", 0, false };

   if (spellId < (sizeof(spellInfo) / sizeof(spellInfoStruct)))
     return &spellInfo[spellId];
   else
     return &fake;
}

QString
skill_name (uint16_t skillId)
{
  // a non-sparse array of skill names
  static const char*  skillnames[] = 
  {
#include "skills.h"
  };

  // return skill name from list if it's within range
  if (skillId < (sizeof(skillnames) / sizeof (char*)))
    return skillnames[skillId];
  else
    return QString::number(skillId);
}

QString size_name(uint8_t sizeId)
{
  // a non-sparse array of size names
  static const char*  sizenames[] = 
  {
    "Tiny",    // 0  - Tiny Items
    "Small",   // 1  - Small Items
    "Medium",  // 2  - Medium Items
    "Large",   // 3  - Large Items
    "Giant",   // 4  - Giant Items
  };

  // return light name from list if it's within range
  if (sizeId < (sizeof(sizenames) / sizeof (char*)))
    return sizenames[sizeId];
  else
    return QString::number(sizeId);
}

QString
language_name (uint8_t langId)
{
  // non-sparse array of language names
  static const char*  languagenames[] = 
  {
#include "languages.h"
  };

  // return language name from list if it's within range
  if (langId < (sizeof(languagenames) / sizeof (char*)))
    return languagenames[langId];
  else
    return QString::number(langId);
}

/* Saves spawn to spawn DB */
void spawndb (const dbSpawnStruct *dbSpawn)
{
   FILE *sdb;
   struct dbSpawnStruct s;
   int found=0;
   char thisname[256];
   char dbname[256];

   /* Check if this is not a player or a corpse */
   if (dbSpawn->spawn.NPC == 0 || dbSpawn->spawn.NPC == 2) {
      return;
   }

   strcpy (thisname, dbSpawn->spawn.name);

   /* Strip off number after spawn name */
   size_t nameLen = strlen(dbSpawn->spawn.name);
   for (size_t a=0; a < nameLen; a++)
     if (thisname[a]<='9')
       thisname[a]=0;

   if ((sdb = fopen (SPAWNFILE, "r")) != NULL)
   {
      while (fread (&s, sizeof(dbSpawnStruct), 1, sdb))
      {
	 strcpy (dbname, s.spawn.name);
	 size_t nameLen = strlen(s.spawn.name);
	 for (size_t a=0; a < nameLen; a++)
	   if (dbname[a]<='9')
	     dbname[a]=0;
	 if (	(!strcmp(dbname, thisname)) &&
		(dbSpawn->spawn.maxHp == s.spawn.maxHp) &&	// Added to catch changed guards
		(dbSpawn->spawn.level == s.spawn.level) &&
		(!strcmp(dbSpawn->zoneName, s.zoneName)) )
	 {
	    found=1;
	    break;
	 }
      }

      fclose (sdb);
   }
   else {
      puts("Error opening spawn file '" SPAWNFILE "'");
   }

   if (!found)
   {
      if ((sdb = fopen (SPAWNFILE, "a")) != NULL)
      {
	 fwrite (dbSpawn, sizeof(dbSpawnStruct), 1, sdb);
	 fclose (sdb);
      }
      else {
         puts("Error opening spawn file '" SPAWNFILE "'");
      }
   }
}

void petdb(const petStruct *spawn)
{
   FILE *pdb;
   struct petStruct s;
   int found=0;

   if ((pdb = fopen (LOGDIR "/pet.db", "r")) != NULL)
   {
      while (fread (&s, sizeof(petStruct), 1, pdb))
      {
	 // Unique on owner class, owner level and pet level
	 if ((spawn->owner.class_==s.owner.class_) && 
	     (spawn->owner.level==s.owner.level) &&
	     (spawn->pet.level==s.pet.level))
	 {
	    found=1;
	    break;
	 }
      }

      fclose (pdb);
   }

   if (!found)
   {
      if ((pdb = fopen (LOGDIR "/pet.db", "a")) != NULL)
      {
	 fwrite (spawn, sizeof(petStruct), 1, pdb);
	 fclose (pdb);
      }
   }   
}

QString print_races (uint16_t races)
{
  QString race_str;

  if (races == 0x0000)
    return "NONE";

  // This needs to change when new races are added
  if (races == 0x3fff)
    return "ALL";

  if (races & 1)
    race_str += "HUM ";
  if (races & 2)
    race_str += "BAR ";
  if (races & 4)
    race_str += "ERU ";
  if (races & 8)
    race_str += "ELF ";
  if (races & 16)
    race_str += "HIE ";
  if (races & 32)
    race_str += "DEF ";
  if (races & 64)
    race_str += "HEF ";
  if (races & 128)
    race_str += "DWF ";
  if (races & 256)
    race_str += "TRL ";
  if (races & 512)
    race_str += "OGR ";
  if (races & 1024)
    race_str += "HFL ";
  if (races & 2048)
    race_str += "GNM ";
  if (races & 4096)
    race_str += "IKS ";
  if (races & 8192)
    race_str += "VAH ";

  if (races >= 16384) // 2^14 aka (1 << 14)
  {
    int new_race;
    for (int i = 14; i < 31; ++i)
    { 
      new_race = 1 << i;
      if (races & new_race)
      {
        race_str += "UNK0x";
        race_str += QString::number(new_race, 16);
        race_str += " ";
      }
    }
  }

  if (race_str.isEmpty())
    race_str = "";
   
  return race_str;
}

QString
print_classes (uint16_t classes)
{
  if (classes == 0x0000)
    return "NONE";

  if (classes == 0x7fff)
    return "ALL";

  QString class_str = "";

  if (classes & 1)
    class_str += "WAR ";
  if (classes & 2)
    class_str += "CLR ";
  if (classes & 4)
    class_str += "PAL ";
  if (classes & 8)
    class_str += "RNG ";
  if (classes & 16)
    class_str += "SHD ";
  if (classes & 32)
    class_str += "DRU ";
  if (classes & 64)
    class_str += "MNK ";
  if (classes & 128)
    class_str += "BRD ";
  if (classes & 256)
    class_str += "ROG ";
  if (classes & 512)
    class_str += "SHM ";
  if (classes & 1024)
    class_str += "NEC ";
  if (classes & 2048)
    class_str += "WIZ ";
  if (classes & 4096)
    class_str += "MAG ";
  if (classes & 8192)
    class_str += "ENC ";
  if (classes & 16384)
    class_str += "BST ";

  if (classes >= 32768) // 2^15 aka (1 << 15)
  {
    int new_class;
    for (int i = 15; i < 31; ++i)
    { 
      new_class = 1 << i;
      if (classes & new_class)
      {
        class_str += "UNK0x";
        class_str += QString::number(new_class, 16);
        class_str += " ";
      }
    }
  }

  return class_str;
}

QString
print_material (uint8_t material)
{
  // sparse array of material names, some are NULL
  static const char*  materialnames[] = 
  {
    "None",             // 0x00
    "Leather",          // 0x01
    "Ringmail",         // 0x02
    "Plate",            // 0x03
    "Cured Silk",       // 0x04
    "Chitin",           // 0x05
    NULL,               // 0x06 - Unknown Material
    "Scale/BlackIron?", // 0x07
    NULL,               // 0x08 - Unknown Material
    NULL,               // 0x09 - Unknown Material
    "ElementRobe",      // 0x0A
    "BlightedRobe",     // 0x0B
    "Crystalline",      // 0x0C
    "OracleRobe",       // 0x0D
    "KedgeRobe",        // 0x0E
    "MetallicRobe",     // 0x0F
    "Robe",             // 0x10
    "VeliousLeather",   // 0x11
    "VeliousChain",     // 0x12
    "PogPlate",         // 0x13
    "Ulthork/Tizmak",   // 0x14
    "Ry`Gorr",          // 0x15
    "Kael/Guardian",    // 0x16
    "VeliousMonk",      // 0x17
  };

  // assume no material name found
  const char *materialStr = NULL;

  // retrieve pointer to race name
  if (material < (sizeof(materialnames) / sizeof (char*)))
    materialStr = materialnames[material];

  // if race name exists, then return it, otherwise return a number string
  if (materialStr != NULL)
    return materialStr;
  else
  {
    QString mat_str;
    mat_str.sprintf("U%02x", material);
    return mat_str;
  }
}

QString
print_skill (uint8_t skill)
{
  switch (skill)
    {
    case 0x00:  // 0x00 (00)  - Generic 1H Slash
      return "1H Slash";
    case 0x01:  // 0x01 (01)  - Generic 2H Slash
      return "2H Slash";
    case 0x02:  // 0x02 (02)  - Generic Piecing
    case 0x23:  // 0x23 (35)  - As per Runed Othmir Spear/Grimy Lance
      return "Piercing";
    case 0x03:  // 0x03 (03)  - Generic 1H Blunt
      return "1H Blunt";
    case 0x04:  // 0x04 (04)  - Generic 2H Blunt
      return "2H Blunt";
    case 0x05:  // 0x05 (05)  - As per the bows
    case 0x1b:  // 0x1b (27)  - As per the arrows
      return "Archery";
    case 0x07:  // 0x07 (07)  - As per Throwing Spear (Small thrown stuff)
    case 0x13:  // 0x13 (19)  - As per Shurien (Large thrown stuff)
      return "Throwing";
    }

  QString skill_name;
  skill_name.sprintf("U0x%2.2x", skill);
  return skill_name;
}

QString
print_slot (uint32_t equipableSlots)
{
  if (equipableSlots == 0x00000000)
    return "NONE";

  QString slot_str;

  if (equipableSlots & 0x00000001)
    slot_str += "SLOT001 ";
  if (equipableSlots & 0x00000002)
    slot_str += "EAR1 ";
  if (equipableSlots & 0x00000004)
    slot_str += "HEAD ";
  if (equipableSlots & 0x00000008)
    slot_str += "FACE ";
  if (equipableSlots & 0x00000010)
    slot_str += "EAR2 ";
  if (equipableSlots & 0x00000020)
    slot_str += "NECK ";
  if (equipableSlots & 0x00000040)
    slot_str += "SHOULDERS ";
  if (equipableSlots & 0x00000080)
    slot_str += "ARMS ";
  if (equipableSlots & 0x00000100)
    slot_str += "BACK ";
  if (equipableSlots & 0x00000200)
    slot_str += "WRIST1 ";
  if (equipableSlots & 0x00000400)
    slot_str += "WRIST2 ";
  if (equipableSlots & 0x00000800)
    slot_str += "RANGE ";
  if (equipableSlots & 0x00001000)
    slot_str += "HANDS ";
  if (equipableSlots & 0x00002000)
    slot_str += "MELEE1 ";
  if (equipableSlots & 0x00004000)
    slot_str += "MELEE2 ";
  if (equipableSlots & 0x00008000)
    slot_str += "FINGER1 ";
  if (equipableSlots & 0x00010000)
    slot_str += "FINGER2 ";
  if (equipableSlots & 0x00020000)
    slot_str += "CHEST ";
  if (equipableSlots & 0x00040000)
    slot_str += "LEGS ";
  if (equipableSlots & 0x00080000)
    slot_str += "FEET ";
  if (equipableSlots & 0x00100000)
    slot_str += "WAIST ";
  if (equipableSlots & 0x00200000)
    slot_str += "AMMO ";
  if (equipableSlots >= 0x00400000)
    {
      QString tmp;
      // generate a string containing only the unknown flags
      tmp.sprintf("U0x%8.8x", (equipableSlots & (~  0x003fffff)));
      slot_str += tmp;
    }

  return slot_str;
}

QString
print_faction (int32_t faction)
{
  static const char* factionnames[] = 
  {
    "regards you as an ally",          // 1
    "looks upon you warmly",           // 2
    "kindly considers you",            // 3
    "judges you amiably",              // 4
    "regards you indifferently",       // 5
    "scowls at you, ready to attack!", // 6
    "glares at you threateningly",     // 7
    "glowers at you dubiously",        // 8
    "looks your way apprehensively",   // 9
  };

  uint32_t lookup = faction - 1;

  // return faction name from list if it's within range
  if (lookup < (sizeof(factionnames) / sizeof (char*)))
    return factionnames[lookup];
  else
  {
    QString tmp;
    tmp.sprintf("Unknown faction: %d", faction);
    return tmp;
  }
}

uint32_t calc_exp (int level, uint16_t race, uint8_t class_)
{

	float exp=level*level*level;
	if (level<30)       exp*=10;
	else if (level<50)	exp*=(10.0 + ((level - 29) * 0.2));
	else if (level<51)	exp*=14;
	else if (level<52)	exp*=15;
	else if (level<53)	exp*=16;
	else if (level<54)	exp*=17;
	else if (level<55)	exp*=19;
	else if (level<56)	exp*=21;
	else if (level<57)	exp*=23;
	else if (level<58)	exp*=25;
	else if (level<59)	exp*=27;
	else                exp*=30;

	// Do the race mod

	switch (race)
	{
	case 1 :   exp*=10;   break; // human
	case 2 :   exp*=10.5; break; // barbarian
	case 3 :   exp*=10;   break; // erudite
	case 4 :   exp*=10;   break; // wood elf
	case 5 :   exp*=10;   break; // high elf
	case 6 :   exp*=10;   break; // dark elf
	case 7 :   exp*=10;   break; // half elf
	case 8 :   exp*=10;   break; // dwarf
	case 9 :   exp*=12;   break; // troll
	case 10 :  exp*=11.5; break; // ogre
	case 11 :  exp*=9.5;  break; // halfling
	case 12 :  exp*=10;   break; // gnome
	case 128 : exp*=12;   break; // iksar
	default :  exp*=10;
	}

	// Do the class mod

	switch (class_)
	{
	case 1 :  exp*=9;    break; // warrior
	case 2 :  exp*=10;   break; // cleric
	case 3 :  exp*=14;   break; // paladin
	case 4 :  exp*=14;   break; // ranger
	case 5 :  exp*=14;   break; // shadowknight
	case 6 :  exp*=10;   break; // druid
	case 7 :  exp*=12;   break; // monk
	case 8 :  exp*=14;   break; // bard
	case 9 :  exp*=9.05; break; // rogue
	case 10 : exp*=10;   break; // shaman
	case 11 : exp*=11;   break; // necromancer
	case 12 : exp*=11;   break; // wizard
	case 13 : exp*=11;   break; // magician
	case 14 : exp*=11;   break; // enchanter
	default : exp*=10;
	}


	return (unsigned long)(exp);
}

//
// mTime
//
// return time in miliseconds
//
int mTime(void)
{
  static long basetime = 0;
  struct timeval TimeNow;
  struct timezone Zone;
  int rt;

  gettimeofday(&TimeNow, &Zone);

  if (basetime == 0)
    basetime = TimeNow.tv_sec;

  rt = (TimeNow.tv_sec - basetime) * 1000 + TimeNow.tv_usec / 1000L;

  return rt;
} // end mTime

int calcMaxMana(int INT, int WIS, int daclass, int level){
	switch (daclass)
	{
	case 2 : // cleric
	case 3 : // paladin
	case 6 : // druid
	case 10 : // shaman
            return ((WIS / 5) +2) * level;

	case 8 : // bard
	case 5 : // shadowknight
	case 11 : // necromancer
	case 12 : //wizard
	case 13 : // magician
	case 14 : // enchanter
            return ((INT / 5) + 2) * level;
	case 1 : // warrior
	case 4 : // ranger
	case 9 : // rogue
	case 7 : // monk
	default :
            return 0;

	}

}//end calc mana

QString reformatMoney (unsigned int uiCopper)
{
  QString qsMoney = "";
  bool bNeedComma = false;

  // if there is no money, just cut to the chase
  if (uiCopper == 0)
    return "0 Copper";

  // should probably get rid of all the needless checks of uiDivisor
  // when the value is known and/or it's not being used again
  unsigned int uiDivisor = 1000;

  if (uiDivisor == 1000 && (uiCopper / uiDivisor) > 0)
  {
    qsMoney.sprintf("%d Platinum", uiCopper / uiDivisor);
    bNeedComma = true;
    uiCopper -= ((uiCopper / uiDivisor) * uiDivisor);
  }

  uiDivisor /= 10;

  if (uiDivisor == 100 && (uiCopper / uiDivisor) > 0)
  {
    qsMoney.sprintf("%s%s%d Gold", qsMoney.ascii(), bNeedComma ? ", " : "", uiCopper / uiDivisor);
    bNeedComma = true;
    uiCopper -= ((uiCopper / uiDivisor) * uiDivisor);
  }

  uiDivisor /= 10;

  if (uiDivisor == 10 && (uiCopper / uiDivisor) > 0)
  {
    qsMoney.sprintf("%s%s%d Silver", qsMoney.ascii(), bNeedComma ? ", " : "", uiCopper / uiDivisor);
    bNeedComma = true;
    uiCopper -= ((uiCopper / uiDivisor) * uiDivisor);
  }

  uiDivisor /= 10;

  if (uiDivisor == 1 && uiCopper != 0)
    qsMoney.sprintf("%s%s%d Copper", qsMoney.ascii(), bNeedComma ? ", " : "", uiCopper);

  return ( qsMoney );
}

// prints up the passed in data to the file pointer
void fprintData(FILE* fp,
		uint32_t len,
		const uint8_t* data
		)
{
  if (fp == NULL)
    return;

  char hex[128];
  char asc[128];
  char tmp[32];
  
  hex[0] = 0;
  asc[0] = 0;
  unsigned int c;
  
  for (c = 0; c < len; c ++)
    {
      if ((!(c % 16)) && c)
	{
	  fprintf (fp, "%03d | %s | %s \n", c - 16, hex, asc);
	  hex[0] = 0;
	  asc[0] = 0;
	}
      
      sprintf (tmp, "%02x ", data[c]);
      strcat (hex, tmp);
      
      if ((data[c] >= 32) && (data[c] <= 126))
	sprintf (tmp, "%c", data[c]);
      
      else
	strcpy (tmp, ".");
      
      strcat (asc, tmp);
      
    }
  
  if (c % 16)
    c = c - (c % 16);
  
  else
    c -= 16;
  
  fprintf (fp, "%03d | %-48s | %s \n\n", c, hex, asc);
}

void diagFileWriteFail(QString filename)
{
  // Get information about the file, if there is one
  QFileInfo fileInfo(filename);

  // Get information about the directory the file should be in
  QFileInfo dirInfo(fileInfo.dirPath());
  
  // Check out the directory
  if (dirInfo.exists())
  {
    // if what's supposed to be a directory isn't somethings wierd.
    if (!dirInfo.isDir())
      fprintf(stderr, "\tDirectory '%s' isn't a directory!\n",
	      (const char*)dirInfo.absFilePath());
    else
    {
      // if the directory isn't writable, that might explain it
      if (!dirInfo.isWritable())
	fprintf(stderr, "\tCan't write to directory: %s\n",
		(const char*)dirInfo.absFilePath());
      // if the directory isn't readable, that might explain it
      if (!dirInfo.isReadable())
	fprintf(stderr, "\tCan't read directory: %s\n",
		(const char*)dirInfo.absFilePath());
      // is the directory executable (listable),
      if (!dirInfo.isExecutable())
	fprintf(stderr, "\tCan't execute/access directory: %s\n",
		(const char*)dirInfo.absFilePath());
    }
  }
  else // directory doesn't exist
    fprintf(stderr, "\tDirectory '%s' doesn't exist!\n",
	    (const char*)dirInfo.absFilePath());
  
  
  // Check out the file
  if (fileInfo.exists())
  {
    // The file exists, but is it writable
    if (!fileInfo.isWritable())
      fprintf(stderr, "\tCan't write to file: %s\n",
	      (const char*)fileInfo.absFilePath());

    // Is the file really a file (or did someone do something wierd)
    if (!fileInfo.isFile())
      fprintf(stderr, "\tNot a file:'%s'!",
	      (const char*)fileInfo.absFilePath());
  }
}

void diagFileReadFail(QString filename)
{
  // Get information about the file, if there is one
  QFileInfo fileInfo(filename);

  // Get information about the directory the file should be in
  QFileInfo dirInfo(fileInfo.dirPath());
  
  // Check out the directory
  if (dirInfo.exists())
  {
    // if what's supposed to be a directory isn't somethings wierd.
    if (!dirInfo.isDir())
      fprintf(stderr, "\tDirectory '%s' isn't a directory!\n",
	      (const char*)dirInfo.absFilePath());
    else
    {
      // if the directory isn't readable, that might explain it
      if (!dirInfo.isReadable())
	fprintf(stderr, "\tCan't read directory: %s\n",
		(const char*)dirInfo.absFilePath());
      // is the directory executable (listable),
      if (!dirInfo.isExecutable())
	fprintf(stderr, "\tCan't execute/access directory: %s\n",
		(const char*)dirInfo.absFilePath());
    }
  }
  else // directory doesn't exist
    fprintf(stderr, "\tDirectory '%s' doesn't exist!\n",
	    (const char*)dirInfo.absFilePath());
  
  
  // Check out the file
  if (fileInfo.exists())
  {
    // The file exists, but is it writable
    if (!fileInfo.isReadable())
      fprintf(stderr, "\tCan't read to file: %s\n",
	      (const char*)fileInfo.absFilePath());

    // Is the file really a file (or did someone do something wierd)
    if (!fileInfo.isFile())
      fprintf(stderr, "\tNot a file:'%s'!",
	      (const char*)fileInfo.absFilePath());
  }
  else
    fprintf(stderr, "\tFile '%s' doesn't exist.\n",
	    (const char*)fileInfo.absFilePath());
}


uint32_t calcCRC32(uint8_t* p,
		   uint32_t length)
{
#ifdef RUNTIME_CRCTAB
  // by default the generated crctab doesn't exist
  static bool crctabInited = false;
  static uint32_t crctab[256];

  // make sure the crctab only gets initialized once
  if (!crctabInited)
  {
    uint32_t i, j;
    uint32_t c;

    // initialize all 256 table entries
    for (i = 0; i < 256; i++)
    {
      c = i;
      for (j = 0; j < 8; j++)
      {
	// this crc table uses generating polynomial crctab[0x80]=0xEDB88320
	if (c & 1)
	  c = 0xEDB88320 ^ (c >> 1);
	else
	  c = c >> 1;
      }

      // save the result for the current position
      crctab[i] = c;
    }

#ifdef GENERATE_CRCTAB_H
    // open a file to store the table into
    FILE* fp = fopen("/tmp/crctab.h", "w");

    if (fp != NULL)
    {
      // file was successfully opened, generate the header file
      fprintf(fp, "static uint32_t crctab[256] =\n"
	      "{\n"
	      "  0x0,\n");
      for (int i = 1; i < 256; i += 5)
	fprintf(fp, "  0x%08X, 0x%08X, 0x%08X, 0x%08x, 0x%08x,\n",
		crctab[i], crctab[i + 1], crctab[i + 2], crctab[i + 3], 
		crctab[i + 4]);
      fprintf(fp, "};\n");

      // finished generating the header file, close the file
      fclose(fp);
    }
#endif // GENERATE_CRCTAB_H

    crctabInited = true;
  }
#else // RUNTIME_CRCTAB
  // use the pre-generated crctab from the generated crctab.h file
#include "crctab.h"
#endif // RUNTIME_CRCTAB
  // seed the crc
  uint32_t crc = 0xFFFFFFFF;

  // iterate over the packet, updating the crc as we go...
  while(length--)
    crc = crctab[(crc ^ *(p++)) & 0xFF] ^ (crc >> 8);
  
  // return the crc after performing the step
  return crc ^ 0xFFFFFFFF;
}

EQTime::EQTime()
{
    epoch = 686510748; // if eq time is computed directly from
                       // real time then this value will always
                       // close to correct. But we reset it when
                       // we get a time sync packet anyway.
}

eqtime_t
EQTime::eptime(const timeOfDayStruct *date)
{
    // convert to epoch relative time (minutes since epoch)
    // epoch = 1 AM, Jan 1, 3000

    return( ((date->year-3000) * 483840) + ((date->month-1) * 40320) +
            ((date->day-1) * 1440) + ((date->hour-1) * 60) + date->minute );
}

void
EQTime::setepoch(time_t rt, const timeOfDayStruct *date)
{
    // 1 EQ Minute = 3 Seconds
    //  so eqtime * 3 = number of seconds since eqepoch

    epoch = rt - (eptime(date) * 3);
    fprintf(stderr,"EQ EPOCH OCCURRED AT %u SECONDS POST UNIX EPOCH\n",
            (unsigned int)epoch);
    return;
}

timeOfDayStruct
EQTime::epdate(eqtime_t et)
{
    timeOfDayStruct date;

    date.year = 3000 + (et / 483840);
    et %= 483840;

    date.month = 1+(et / 40320);
    et %= 40320;

    date.day = 1+(et / 1440);
    et %= 1440;

    date.hour = 1+(et / 60);
    et %= 60;

    date.minute = et;

    return(date);
}

eqtime_t
EQTime::eqtime(time_t rt)
{
    return( (rt - epoch) / 3 );
}

timeOfDayStruct
EQTime::eqdate(time_t rt)
{
    return(epdate(eqtime(rt)));
}

bool findFile( QString& filename )
{
  bool                    found = false;
  
  QFileInfo               fi( filename );
  QDir                    dir( fi.dirPath( true ) );
  QString                 fileLower = fi.fileName().lower();
  
  QStringList             dirEntries = dir.entryList();
  
  for ( QStringList::Iterator it = dirEntries.begin();
	it != dirEntries.end();
	++it )
  {
    QString entry( (*it).lower() );
    if ( entry == fileLower )
    {
      filename = fi.dirPath( true );
      filename += "/";
      filename += *it;
      found = true;
      break;
    }
  }
  return found;
}
