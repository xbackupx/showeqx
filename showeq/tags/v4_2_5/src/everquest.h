/*
 *  everquest.h
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 */


/*
** Please be kind and remember to correctly re-order
** the values in here whenever you add a new item,
** thanks.  - Andon
*/


/*
** Structures used in the network layer of Everquest
*/

#ifndef EQSTRUCT_H
#define EQSTRUCT_H

#include <stdint.h>

/*
** ShowEQ specific definitions
*/
// Statistical list defines
#define LIST_HP                         0
#define LIST_MANA                       1
#define LIST_STAM                       2
#define LIST_EXP                        3
#define LIST_FOOD                       4
#define LIST_WATR                       5
#define LIST_STR                        6
#define LIST_STA                        7
#define LIST_CHA                        8
#define LIST_DEX                        9
#define LIST_INT                        10
#define LIST_AGI                        11
#define LIST_WIS                        12
#define LIST_MR                         13
#define LIST_FR                         14
#define LIST_CR                         15
#define LIST_DR                         16
#define LIST_PR                         17
#define LIST_AC                         18
#define LIST_ALTEXP                     19
#define LIST_MAXLIST                    20 

// direction the data is coming from
#define DIR_CLIENT 1
#define DIR_SERVER 2

/*
** MOB Spawn Type
*/
#define SPAWN_PLAYER                    0
#define SPAWN_NPC                       1
#define SPAWN_PC_CORPSE                 2
#define SPAWN_NPC_CORPSE                3
#define SPAWN_NPC_UNKNOWN               4
#define SPAWN_COINS                     5
#define SPAWN_DROP                      6
#define SPAWN_DOOR                      7
#define SPAWN_SELF                      10

/* 
** Diety List
*/
#define DEITY_UNKNOWN                   0
#define DEITY_AGNOSTIC			396
#define DEITY_BRELL			202
#define DEITY_CAZIC			203
#define DEITY_EROL			204
#define DEITY_BRISTLE			205
#define DEITY_INNY			206
#define DEITY_KARANA			207
#define DEITY_MITH			208
#define DEITY_PREXUS			209
#define DEITY_QUELLIOUS			210
#define DEITY_RALLOS			211
#define DEITY_SOLUSEK			213
#define DEITY_TRIBUNAL			214
#define DEITY_TUNARE			215

//Guessed:
#define DEITY_BERT			201	
#define DEITY_RODCET			212
#define DEITY_VEESHAN			216

//Team numbers for Deity teams
#define DTEAM_GOOD			1
#define DTEAM_NEUTRAL			0
#define DTEAM_EVIL			-1
#define DTEAM_OTHER			2

//Team numbers for Race teams
#define RTEAM_HUMAN			1
#define RTEAM_ELF			2
#define RTEAM_DARK			3
#define RTEAM_SHORT			4
#define RTEAM_OTHER			5

//Maximum limits of certain types of data
#define MAX_KNOWN_SKILLS                74
#define MAX_KNOWN_LANGS                 25

//Item Flags
#define ITEM_NORMAL1                    0x0031
#define ITEM_NORMAL2                    0x0036
#define ITEM_NORMAL3                    0x315f
#define ITEM_NORMAL4                    0x3336
#define ITEM_NORMAL5                    0x0032
#define ITEM_NORMAL6                    0x0033
#define ITEM_NORMAL7                    0x0034
#define ITEM_NORMAL8                    0x0039
#define ITEM_CONTAINER                  0x7900
#define ITEM_CONTAINER_PLAIN            0x7953
#define ITEM_BOOK                       0x7379

// Item spellId no spell value
#define ITEM_SPELLID_NOSPELL            0xffff

//Combat Flags
#define COMBAT_MISS						0
#define COMBAT_BLOCK					-1
#define COMBAT_PARRY					-2
#define COMBAT_RIPOSTE					-3
#define COMBAT_DODGE					-4

#define PLAYER_CLASSES     15
#define PLAYER_RACES       14
/*
** Compiler override to ensure
** byte aligned structures
*/
#pragma pack(1)

/*
**            Generic Structures used in specific
**                      structures below
*/

// OpCode stuff (For SINS Migrations)
struct opCode
{
  uint8_t code;
  uint8_t version;
	
  // kinda silly -- this is required for us to be able to stuff them in a QValueList
  bool operator== ( const struct opCode t ) const
  {
    return ( code == t.code && version == t.version );
  }
  bool operator== ( uint16_t opCode ) const
  {
    return ( *((uint16_t*)&code) == opCode );
  }
};
typedef struct opCode OpCode;

/*
** Buffs
** Length: 10 Octets
** Used in:
**    charProfileStruct(2d20)
*/
struct spellBuff
{
/*0000*/ int8_t   unknown0000;            // ***Placeholder
/*0001*/ int8_t   level;                  // Level of person who casted buff
/*0002*/ int8_t   unknown0002;            // ***Placeholder
/*0003*/ int8_t   unknown0003;            // ***Placeholder
/*0004*/ int16_t  spell;                  // Spell
/*0006*/ int32_t  duration;               // Duration in ticks
};

/*
** Generic Item structure
** Length: 244 Octets
** Used in:
**    itemShopStruct(0c20), tradeItemInStruct(5220),
**    playerItemStruct(6421), playerBookStruct(6521),
**    playerContainerStruct(6621), summonedItemStruct(7821),
**    tradeItemInStruct(df20),
*/

struct itemStruct
{
/*0000*/ char      name[35];        // Name of item
/*0035*/ char      lore[60];        // Lore text
/*0095*/ char      idfile[6];       // Not sure what this is used for
/*0101*/ int16_t   flag;            // Flag value indicating type of item:
  // 0x0031 - Normal Item - Only seen once on GM summoned food
  // 0x0036 - Normal Item (all scribed spells, Velium proc weapons, and misc.)
  // 0x315f - Normal Item
  // 0x3336 - Normal Item
  // 0x7900 - Container (Combine, Player made, Weight Reducing, etc...)
  // 0x7953 - Container, plain ordinary newbie containers
  // 0x7379 - Book item 
/*0103*/ uint8_t   unknown0103[22]; // Placeholder
/*0125*/ uint8_t   weight;          // Weight of item
/*0126*/ int8_t    nosave;          // Nosave flag 1=normal, 0=nosave, -1=spell?
/*0127*/ int8_t    nodrop;          // Nodrop flag 1=normal, 0=nodrop, -1=??
/*0128*/ uint8_t   size;            // Size of item
/*0129*/ uint8_t   unknown0129;     // ***Placeholder
/*0130*/ uint16_t  itemNr;          // Unique Item number
/*0132*/ uint16_t  iconNr;          // Icon Number
/*0134*/ int16_t   equipSlot;       // Current Equip slot
/*0136*/ uint32_t  equipableSlots;  // Slots where this item goes
/*0140*/ int32_t   cost;            // Item cost in copper
/*0144*/ uint8_t   unknown0144[28]; // ***Placeholder

  union // 0172-291 have different meanings depending on flags
  {
    // note, each of the following 2 structures must be kept of equal size

    struct // Common Item Structure (everything but books (flag != 0x7669)
    {
      /*0172*/ int8_t   STR;              // Strength
      /*0173*/ int8_t   STA;              // Stamina
      /*0174*/ int8_t   CHA;              // Charisma
      /*0175*/ int8_t   DEX;              // Dexterity
      /*0176*/ int8_t   INT;              // Intelligence
      /*0177*/ int8_t   AGI;              // Agility
      /*0178*/ int8_t   WIS;              // Wisdom
      /*0179*/ int8_t   MR;               // Magic Resistance
      /*0180*/ int8_t   FR;               // Fire Resistance
      /*0181*/ int8_t   CR;               // Cold Resistance
      /*0182*/ int8_t   DR;               // Disease Resistance
      /*0183*/ int8_t   PR;               // Poison Resistance
      /*0184*/ int8_t   HP;               // Hitpoints
      /*0185*/ int8_t   MANA;             // Mana
      /*0186*/ int8_t   AC;               // Armor Class
      /*0187*/ uint8_t  unknown0187[2];   // ***Placeholder
      /*0189*/ uint8_t  light;            // Light effect of this item
      /*0190*/ uint8_t  delay;            // Weapon Delay
      /*0191*/ uint8_t  damage;           // Weapon Damage
      /*0192*/ uint8_t  unknown0192;      // ***Placeholder
      /*0193*/ uint8_t  range;            // Range of weapon
      /*0194*/ uint8_t  skill;            // Skill of this weapon
      /*0195*/ int8_t   magic;            // Magic flag
                        //   00  (0000)  =   ???
                        //   01  (0001)  =  magic
                        //   12  (1100)  =   ???
                        //   14  (1110)  =   ???
                        //   15  (1111)  =   ???
      /*0196*/ int8_t   level0;           // Casting level
      /*0197*/ uint8_t  material;         // Material?
      /*0198*/ uint8_t  unknown0198[2];   // ***Placeholder
      /*0200*/ uint32_t color;            // Amounts of RGB in original color
      /*0204*/ uint8_t  unknown0204[2];   // ***Placeholder
      /*0206*/ uint16_t spellId0;         // SpellID of special effect
      /*0208*/ uint16_t classes;          // Classes that can use this item
      /*0210*/ uint8_t  unknown0210[2];   // ***Placeholder
      /*0212*/ uint16_t races;            // Races that can use this item
      /*0214*/ int8_t   unknown0214[3];   // ***Placeholder
      /*0217*/ uint8_t  level;            // Casting level

      union // 0218 has different meanings depending on an unknown indicator
      {
        /*0218*/ int8_t   number;          // Number of items in stack
        /*0218*/ int8_t   charges;         // Number of charges (-1 = unlimited)
      };

      /*0219*/ int8_t   unknown0219;       // ***Placeholder
      /*0220*/ uint16_t spellId;           // spellId of special effect
      /*0222*/ uint8_t  unknown0222[70];   // ***Placeholder
    } common;
    struct // Book Structure (flag == 0x7379)
    {
      /*0172*/ uint8_t  unknown0172[3];      // ***Placeholder
      /*0175*/ char     file[15];            // Filename of book text on server
      /*0190*/ uint8_t  unknown0190[102];    // ***Placeholder
    } book;
    struct // Container Structure (flag == 0x7900 || flag == 0x7953
    {
      /*0172*/ int8_t   unknown0191[41];     // ***Placeholder
      /*0213*/ uint8_t  numSlots;            // number of slots in container
      /*0214*/ int8_t   unknown0214;         // ***Placeholder
      /*0215*/ int8_t   sizeCapacity;        // Maximum size item container can hold
      /*0216*/ uint8_t  weightReduction;     // % weight reduction of container
      /*0217*/ uint8_t  unknown0217[75];     // ***Placeholder
    } container;
  };
};

// Convenience inlines for itemStruct

inline bool isItemBook(const struct itemStruct& i) 
    { return (i.flag == ITEM_BOOK); }

inline bool isItemContainer(const struct itemStruct& i)
    { return ((i.flag == ITEM_CONTAINER) || (i.flag == ITEM_CONTAINER_PLAIN)); }

/*
** Generic Item Properties
** Length: 10 Octets
** Used in: charProfile
**
*/

struct itemPropertiesStruct
{
/*000*/ uint8_t    unknown01[2];
/*002*/ int8_t     charges;
/*003*/ uint8_t    unknown02[7];
};


/*
** Generic Spawn Update
** Length: 15 Octets
** Used in:
**
*/

struct spawnPositionUpdate 
{
/*0000*/ uint16_t spawnId;                // Id of spawn to update
/*0002*/ int8_t   animation;              // Animation spawn is currently using
/*0003*/ int8_t   heading;                // Heading
/*0004*/ int8_t   deltaHeading;           // Heading Change
/*0005*/ int16_t  yPos;                   // New Y position of spawn
/*0007*/ int16_t  xPos;                   // New X position of spawn
/*0009*/ int16_t  zPos;                   // New Z position of spawn
/*0011*/ signed   deltaY:10;              // Y Velocity
         unsigned spacer1:1;              // ***Placeholder
         signed   deltaZ:10;              // Z Velocity
         unsigned spacer2:1;              // ***Placeholder
         signed   deltaX:10;              // Z Velocity
};

/* 
** Generic Spawn Struct 
** Length: 220 Octets 
** Used in: 
**   spawnZoneStruct
**   dbSpawnStruct
**   petStruct
**   newSpawnStruct
*/ 
struct spawnStruct 
{ 
/*0000*/ uint8_t  unknown0000[48];        // Placeholder 
/*0048*/ uint8_t  animation;              // Animation spawn is currently using
/*0049*/ int8_t   heading;                // Current Heading 
/*0050*/ int8_t   deltaHeading;           // Delta Heading 
/*0051*/ int16_t  yPos;                   // Y Position 
/*0053*/ int16_t  xPos;                   // X Position 
/*0055*/ int16_t  zPos;                   // Z Position 
/*0057*/ signed   deltaY:10;              // Velocity Y 
         unsigned spacer1:1;              // Placeholder 
         signed   deltaZ:10;              // Velocity Z 
         unsigned spacer2:1;              // ***Placeholder 
         signed   deltaX:10;              // Velocity X 
/*0061*/ uint8_t  unknown0061;            // ***Placeholder 
/*0062*/ uint16_t spawnId;                // Id of new spawn 
/*0064*/ int8_t   typeflag;               // 65 is disarmable trap, 
                                          // 66 and 67 are invis triggers/traps
/*0065*/ uint8_t  unknown0065;            // ***Placeholder 
/*0066*/ uint16_t petOwnerId;             // Id of pet owner (0 if not a pet) 
/*0068*/ int16_t  maxHp;                  // Maximum hp 
/*0070*/ int16_t  curHp;                  // Current hp // GuildID now?
/*0072*/ uint16_t race;                   // Race 
/*0074*/ uint8_t  NPC;                    // NPC type: 0=Player, 1=NPC,
                                          // 2=Player Corpse, 3=Monster Corpse,
                                          // 4=???, 5=Unknown Spawn, 10=Self 
/*0075*/ uint8_t  class_;                 // Class 
/*0076*/ uint8_t  gender;                 // gender, 0=Male, 1=Female, 2=Other 
/*0077*/ uint8_t  level;                  // Level of spawn (might be one byte) 
/*0078*/ uint8_t  invis;
/*0079*/ uint8_t  unknown0079;
/*0080*/ uint8_t  pvp;
/*0081*/ uint8_t  anim_type;
/*0082*/ uint8_t  light;                  // light source
/*0083*/ uint8_t  anon;
/*0084*/ uint8_t  afk;
/*0085*/ uint8_t  unknown0085;
/*0086*/ uint8_t  linkdead;
/*0087*/ uint8_t  gm;
/*0088*/ uint8_t  unknown0088;
/*0089*/ uint8_t  npc_armor_graphic;      // 0xFF=Player, 0=none, 1=leather, 2=chain, 3=plate
/*0090*/ uint8_t  npc_helm_graphic;       // 0xFF=Player, 0=none, 1=leather, 2=chain, 3=plate
/*0091*/ uint8_t unknown0091;

/*0092*/ uint16_t equipment[9];           // equipment
/*0110*/ char     name[64];               // Name of spawn (len is 64 or less) 
/*0174*/ char     lastname[20];           // Last Name of player 
/*0194*/ uint8_t  unknown0194[8];         // ***Placeholder 
/*0202*/ uint8_t  unknown0202[6];         // ***Placeholder 
/*0208*/ uint16_t deity;                  // deity
/*0210*/ uint8_t  unknown0210[8];         // ***Placeholder 
/*0218*/ uint8_t  unknown0218[2];         // ***Placeholder 
};  


/* 
** Zone Spawn Struct 
** Length: 176 Octets 
** Used in: 
**    zoneSpawnStruct
**
*/ 

struct spawnZoneStruct
{
/*0000*/ uint8_t     unknown0000[4];
/*0004*/ spawnStruct spawn;
};

/*
** Generic Door Struct
** Length: 44 Octets
** Used in: 
**    cDoorSpawnsStruct(f721)
**
*/

struct doorStruct
{
/*0000*/ char    name[8];            // Filename of Door?
/*0008*/ uint8_t unknown0008[8];     // ****Placeholder
/*0016*/ float   yPos;               // y loc
/*0020*/ float   xPos;               // x loc
/*0024*/ float   zPos;               // z loc
/*0028*/ uint8_t unknown0028[10];    // ***Placeholder
/*0038*/ uint8_t doorId;             // door's id #
/*0039*/ uint8_t size;               // guess..
/*0040*/ uint8_t unknown0040[4];     // ***Placeholder
};

/*
**                 ShowEQ Specific Structures
*/

/*
** DB spawn struct (adds zone spawn was in)
*/

struct dbSpawnStruct
{
/*0000*/ struct spawnStruct spawn;      // Spawn Information
/*0156*/ char   zoneName[40];           // Zone Information
};

/*
** Pet spawn struct (pets pet and owner in one struct)
*/

struct petStruct
{
/*0000*/ struct spawnStruct owner;      // Pet Owner Information
/*0156*/ struct spawnStruct pet;        // Pet Infromation
};

/*
**                 Specific Structures defining OpCodes
*/

/*
** Drop Coins
** Length: 114 Octets
** OpCode: dropCoinsCode
*/

struct dropCoinsStruct
{
/*0000*/ uint8_t  opCode;                 // 0x07
/*0001*/ uint8_t  version;		  // 0x20
/*0002*/ uint8_t  unknown0002[24];        // ***Placeholder
/*0026*/ uint16_t dropId;                 // Drop ID
/*0028*/ uint8_t  unknown0028[22];        // ***Placeholder
/*0050*/ uint32_t amount;                 // Total Dropped
/*0054*/ uint8_t  unknown0054[4];         // ***Placeholder
/*0058*/ float    yPos;                   // Y Position
/*0062*/ float    xPos;                   // X Position
/*0066*/ float    zPos;                   // Z Position
/*0070*/ uint8_t  unknown0070[12];        // blank space
/*0082*/ int8_t   type[15];               // silver gold whatever
/*0097*/ uint8_t  unknown0097[17];        // ***Placeholder
};

/*
** Channel Message received or sent
** Length: 71 Octets + Variable Length + 4 Octets
** OpCode: ChannelMessageCode
*/

struct channelMessageStruct
{
/*0000*/ uint8_t  opCode;                 // 0x07
/*0001*/ uint8_t  version;                // 0x21
/*0002*/ char     target[64];             // the target characters name
/*0066*/ char     sender[64];             // The senders name 
/*0130*/ uint8_t  language;               // Language
/*0131*/ uint8_t  unknown0131;         // ***Placeholder
/*0132*/ uint8_t  chanNum;                // Channel
/*0133*/ int8_t   unknown0133[5];            // ***Placeholder
/*0138*/ char     message[0];             // Variable length message
};

/*
** Remove Coins
** Length: 10 Octets
** OpCode: removeCoinsCode
*/

struct removeCoinsStruct
{
/*0000*/ uint8_t  opCode;                 // 0x08
/*0001*/ uint8_t  version;                // 0x20
/*0002*/ uint16_t dropId;                 // Drop ID - Guess
/*0004*/ uint8_t  unknown0004[2];         // ***Placeholder
/*0006*/ uint16_t spawnId;                // Spawn Pickup
/*0008*/ uint8_t  unknown0008[2];         // ***Placeholder
};

/*
** Item In Shop
** Length: 255 Octets
** OpCode: ItemInShopCode
*/

struct itemInShopStruct
{
/*0000*/ uint8_t  opCode;                 // 0x0c
/*0001*/ uint8_t  version;                // 0x20
/*0002*/ uint16_t playerid;               // player ID
/*0004*/ int8_t   itemType;               // 0 - item, 1 - container, 2 - book
/*0005*/ struct   itemStruct item;        // Refer to itemStruct for members
/*0297*/ uint8_t  unknown0297[6];         // ***Placeholder
};

/*
** Server System Message
** Length: Variable Length
** OpCode: SysMsgCode
*/

struct sysMsgStruct
{
/*0000*/ uint8_t  opCode;                 // 0x14
/*0001*/ uint8_t  version;                // 0x20
/*0002*/ char     message[0];             // Variable length message
};

/*
** Emote text
** Length: Variable Text
** OpCode: emoteTextCode
*/

struct emoteTextStruct
{
/*0000*/ uint8_t  opCode;                 // 0x15
/*0001*/ uint8_t  version;                // 0x20
/*0002*/ uint8_t  unknown0002[2];         // ***Placeholder
/*0004*/ char     text[0];                // Emote `Text
};

/*
** Formatted text messages
** Length: Variable Text
** OpCode: emoteTextCode
*/

struct formattedMessageStruct
{
/*0000*/ uint8_t  opCode;                 // 0x15
/*0001*/ uint8_t  version;                // 0x20
/*0002*/ uint8_t  unknown0002[2];         // ***Placeholder
/*0003*/ uint16_t messageFormat;          // Indicates the message format
/*0005*/ uint8_t  unknown0004[2];         // ***Placeholder (arguments?)
/*0007*/ char     messages[0];            // messages(NULL delimited)
/*0???*/ uint8_t  unknownXXXX[8];         // ***Placeholder
};

/*
** Corpse location
** Length: 18 Octets
** OpCode: corpseLocCode
*/

struct corpseLocStruct
{
/*0000*/ uint8_t  opCode;                 // 0x21
/*0001*/ uint8_t  version;                // 0x20
/*0002*/ uint16_t spawnId;
/*0004*/ uint16_t unknown0004;
/*0006*/ float    xPos;
/*0010*/ float    yPos;
/*0014*/ float    zPos;
};

/*
** Grouping Infromation
** Length: 230 Octets
** OpCode: groupinfoCode
*/

struct groupInfoStruct
{
/*0000*/ uint8_t  opCode;                 // 0x26
/*0001*/ uint8_t  version;                // 0x40
/*0002*/ char     yourname[32];           // Player Name
/*0034*/ char     membername[32];         // Goup Member Name
/*0066*/ uint8_t  unknown0066[35];        // ***Placeholder
/*0101*/ uint8_t  bgARC;                  // Add = 2, Remove = 3, 
                                          //     Clear = 0- Bad Guess-ATB
/*0102*/ uint8_t  unknown0102[83];        // ***Placeholder
/*0185*/ int8_t   oper;                   // Add = 4, Remove = 3
/*0186*/ int8_t   ARC2;                   // ?? -  Add = c8, remove 1 = c5, 
                                          //     clear = 01
/*0187*/ uint8_t  unknown0187[43];        // ***Placeholder
};
typedef struct groupInfoStruct groupMemberStruct; // new form

/*
** Client Zone Entry struct
** Length: 70 Octets
** OpCode: ZoneEntryCode (when direction == client)
*/

struct ClientZoneEntryStruct
{
/*0000*/ uint8_t  opCode;                 // 0x29
/*0001*/ uint8_t  version;                // 0x20
/*0002*/ uint32_t unknown0002;            // ***Placeholder
/*0006*/ char     name[32];               // Player firstname
/*0038*/ uint8_t  unknown0036[28];        // ***Placeholder
/*0066*/ uint32_t unknown0066;            // unknown
};

/*
** Server Zone Entry struct
** Length: 358 Octets
** OpCode: ZoneEntryCode (when direction == server)
*/

struct ServerZoneEntryStruct
{
/*0000*/ uint8_t  opCode;                 // 0x29
/*0001*/ uint8_t  version;                // 0x20
/*0002*/ uint32_t checksum;               // some kind of checksum
/*0006*/ uint8_t  unknown0006;            // unknown
/*0007*/ char     name[64];               // Player first name
/*0071*/ uint8_t  unknown0037[3];         // unknown
/*0074*/ uint32_t zoneId;                 // zone number
/*0078*/ uint32_t unknown0078;
/*0082*/ float    xPos;
/*0086*/ float    yPos;
/*0090*/ float    zPos;
/*0094*/ float    heading;
/*0098*/ uint8_t  unknown0098[68];
/*0166*/ uint16_t guildId;
/*0168*/ uint8_t  unknown0169[7];
/*0175*/ uint8_t  class_;                 // Player's Class
/*0176*/ uint16_t race;                   // Player's Race
/*0178*/ uint8_t  unknown0177;            // ***Placeholder
/*0179*/ uint8_t  level;                  // Player's Level
/*0180*/ uint8_t  unknown0180[160];       // ***Placeholder 
/*0340*/ uint16_t deity;                  // Player's Deity
/*0342*/ uint8_t  unknown0310[8];         // ***Placeholder
/*0350*/ uint8_t  unknown0318[8];         // ***Placeholder
};

/*
** Delete Spawn
** Length: 4 Octets
** OpCode: DeleteSpawnCode
*/

struct deleteSpawnStruct
{
/*0000*/ uint8_t  opCode;                 // 0x2a
/*0001*/ uint8_t  version;                // 0x20
/*0002*/ uint16_t spawnId;                // Spawn ID to delete
};

/*
** Player Profile
** Length: 8246 Octets
** OpCode: CharProfileCode
*/

struct charProfileStruct 
{
/*0000*/ uint8_t  opCode;                 // 0x36
/*0001*/ uint8_t  version;                // 0x20
/*0002*/ uint32_t checksum;               // some pre-encryption checksum
/*0006*/ uint8_t  unknown0006[2];         // ***Placeholder
/*0008*/ char     name[64];               // Name of player sizes not right
/*0072*/ char     lastName[70];           // Last name of player sizes not right
/*0142*/ uint8_t  unknown0142[2];
/*0144*/ uint16_t race;                   // Player race
/*0146*/ uint8_t  class_;                 // Player class
/*0147*/ uint8_t  unknown0147;
/*0148*/ uint8_t  gender;                 // Player gender
/*0149*/ uint8_t  unknown0149;
/*0150*/ uint8_t  level;                  // Level of player (might be one byte)
/*0151*/ uint8_t  unknown0151;            // ***Placeholder
/*0152*/ uint16_t unknown0152;            // ***Placeholder
/*0154*/ uint32_t exp;                    // Current Experience
/*0158*/ uint16_t face;                   // ***Placeholder
/*0160*/ uint16_t MANA;                   // MANA
/*0162*/ uint16_t curHp;                  // current hp
/*0164*/ uint16_t unknown0124;            // ***Placeholder
/*0166*/ uint16_t STR;                    // Strength
/*0168*/ uint16_t STA;                    // Stamina
/*0170*/ uint16_t CHA;                    // Charisma
/*0172*/ uint16_t DEX;                    // Dexterity
/*0174*/ uint16_t INT;                    // Intelligence
/*0176*/ uint16_t AGI;                    // Agility
/*0178*/ uint16_t WIS;                    // Wisdom
/*0180*/ uint16_t unknown0180;    
/*0182*/ uint8_t  unknown0182[44];
/*0226*/ uint16_t inventory[30];          // inventory images?
/*0286*/ uint8_t  languages[25];          // List of languages (MAX_KNOWN_LANGS)
/*0311*/ uint8_t  unknown0311[7];         // unknown ?more languages?
/*0318*/ struct itemPropertiesStruct invitemprops[30]; 
                                          // these correlate with inventory[30]
/*0618*/ struct spellBuff buffs[15];      // Buffs currently on the player
/*0768*/ uint16_t containerinv[80];       // player items in containers
/*0928*/ uint16_t cursorinv[10];          // player items on cursor
/*0948*/ struct itemPropertiesStruct containeritemprops[80]; 
                                          //just like invitemprops[]
/*1748*/ struct itemPropertiesStruct cursoritemprops[10];
                                          //just like invitemprops[]
/*1848*/ int16_t  sSpellBook[256];        // List of the Spells 
                                          //    scribed in the spell book
/*2360*/ int16_t  sMemSpells[8];          // List of spells memorized
/*2376*/ uint8_t  unknown2376[496];      
/*2872*/ uint8_t  unknown2872[32];
/*2904*/ uint8_t  unknown2904[2];        
/*2906*/ float    yPos;
/*2910*/ float    xPos;
/*2914*/ float    zPos;
/*2918*/ float    heading;
/*2922*/ uint8_t  unknown2922[4];
/*2926*/ uint32_t platinum;               // Platinum Pieces on player
/*2930*/ uint32_t gold;                   // Gold Pieces on player
/*2934*/ uint32_t silver;                 // Silver Pieces on player
/*2938*/ uint32_t copper;                 // Copper Pieces on player
/*2942*/ uint32_t platinumBank;           // Platinum Pieces in Bank
/*2946*/ uint32_t goldBank;               // Gold Pieces in Bank
/*2950*/ uint32_t silverBank;             // Silver Pieces in Bank
/*2954*/ uint32_t copperBank;             // Copper Pieces in Bank
/*2958*/ uint8_t  unknown2958[32];

/*2990*/ uint16_t skills[74];             // List of skills (MAX_KNOWN_SKILLS)
/*3138*/ uint8_t  unknown3138[306];       // ***Placeholder
/*3444*/ uint32_t zoneId;                 // see zones.h
/*3448*/ uint8_t  unknown3448[128];
/*3576*/ uint32_t bindzoneId[5];          // bound to bindzoneId[0]
/*3596*/ float    sYpos[5];               // starting locs per bindzoneId
/*3616*/ float    sXpos[5];               // starting locs per bindzoneId
/*3636*/ float    sZpos[5];               // starting locs per bindzoneId
/*3656*/ uint8_t  unknown3658[1080];
/*4736*/ uint16_t deity;                  // deity
/*4738*/ char     GroupMembers[5][48];    // List of all the members in group
/*4978*/ uint32_t altexp;                 // alternate exp pool 0 - ~15,000,000 
/*4982*/ uint8_t  aapoints;               // number of ability points? 
/*4983*/ uint8_t  unknown4983[3263];
};


#if 0
struct playerAAStruct {
/*    0 */  uint8 unknown0;
  union {
    uint8 unnamed[17];
    struct _named {  
/*    1 */  uint8 innate_strength;
/*    2 */  uint8 innate_stamina;
/*    3 */  uint8 innate_agility;
/*    4 */  uint8 innate_dexterity;
/*    5 */  uint8 innate_intelligence;
/*    6 */  uint8 innate_wisdom;
/*    7 */  uint8 innate_charisma;
/*    8 */  uint8 innate_fire_protection;
/*    9 */  uint8 innate_cold_protection;
/*   10 */  uint8 innate_magic_protection;
/*   11 */  uint8 innate_poison_protection;
/*   12 */  uint8 innate_disease_protection;
/*   13 */  uint8 innate_run_speed;
/*   14 */  uint8 innate_regeneration;
/*   15 */  uint8 innate_metabolism;
/*   16 */  uint8 innate_lung_capacity;
/*   17 */  uint8 first_aid;
    } named;
  } general_skills;
  union {
    uint8 unnamed[17];
    struct _named {
/*   18 */  uint8 healing_adept;
/*   19 */  uint8 healing_gift;
/*   20 */  uint8 unknown20;
/*   21 */  uint8 spell_casting_reinforcement;
/*   22 */  uint8 mental_clarity;
/*   23 */  uint8 spell_casting_fury;
/*   24 */  uint8 chanelling_focus;
/*   25 */  uint8 unknown25;
/*   26 */  uint8 unknown26;
/*   27 */  uint8 unknown27;
/*   28 */  uint8 natural_durability;
/*   29 */  uint8 natural_healing;
/*   30 */  uint8 combat_fury;
/*   31 */  uint8 fear_resistance;
/*   32 */  uint8 finishing_blow;
/*   33 */  uint8 combat_stability;
/*   34 */  uint8 combat_agility;
    } named;
  } archetype_skills;
  union {
    uint8 unnamed[93];
    struct _name {
/*   35 */  uint8 mass_group_buff; // All group-buff-casting classes(?)
// ===== Cleric =====
/*   36 */  uint8 divine_resurrection;
/*   37 */  uint8 innate_invis_to_undead; // cleric, necromancer
/*   38 */  uint8 celestial_regeneration;
/*   39 */  uint8 bestow_divine_aura;
/*   40 */  uint8 turn_undead;
/*   41 */  uint8 purify_soul;
// ===== Druid =====
/*   42 */  uint8 quick_evacuation; // wizard, druid
/*   43 */  uint8 exodus; // wizard, druid
/*   44 */  uint8 quick_damage; // wizard, druid
/*   45 */  uint8 enhanced_root; // druid
/*   46 */  uint8 dire_charm; // enchanter, druid, necromancer
// ===== Shaman =====
/*   47 */  uint8 cannibalization;
/*   48 */  uint8 quick_buff; // shaman, enchanter
/*   49 */  uint8 alchemy_mastery;
/*   50 */  uint8 rabid_bear;
// ===== Wizard =====
/*   51 */  uint8 mana_burn;
/*   52 */  uint8 improved_familiar;
/*   53 */  uint8 nexus_gate;
// ===== Enchanter  =====
/*   54 */  uint8 unknown54;
/*   55 */  uint8 permanent_illusion;
/*   56 */  uint8 jewel_craft_mastery;
/*   57 */  uint8 gather_mana;
// ===== Mage =====
/*   58 */  uint8 mend_companion; // mage, necromancer
/*   59 */  uint8 quick_summoning;
/*   60 */  uint8 frenzied_burnout;
/*   61 */  uint8 elemental_form_fire;
/*   62 */  uint8 elemental_form_water;
/*   63 */  uint8 elemental_form_earth;
/*   64 */  uint8 elemental_form_air;
/*   65 */  uint8 improved_reclaim_energy;
/*   66 */  uint8 turn_summoned;
/*   67 */  uint8 elemental_pact;
// ===== Necromancer =====
/*   68 */  uint8 life_burn;
/*   69 */  uint8 dead_mesmerization;
/*   70 */  uint8 fearstorm;
/*   71 */  uint8 flesh_to_bone;
/*   72 */  uint8 call_to_corpse;
// ===== Paladin =====
/*   73 */  uint8 divine_stun;
/*   74 */  uint8 improved_lay_of_hands;
/*   75 */  uint8 slay_undead;
/*   76 */  uint8 act_of_valor;
/*   77 */  uint8 holy_steed;
/*   78 */  uint8 fearless; // paladin, shadowknight

/*   79 */  uint8 two_hand_bash; // paladin, shadowknight
// ===== Ranger =====
/*   80 */  uint8 innate_camouflage; // ranger, druid
/*   81 */  uint8 ambidexterity; // all "dual-wield" users
/*   82 */  uint8 archery_mastery; // ranger
/*   83 */  uint8 unknown83;
/*   84 */  uint8 endless_quiver; // ranger
// ===== Shadow Knight =====
/*   85 */  uint8 unholy_steed;
/*   86 */  uint8 improved_harm_touch;
/*   87 */  uint8 leech_touch;
/*   88 */  uint8 unknown88;
/*   89 */  uint8 soul_abrasion;
// ===== Bard =====
/*   90 */  uint8 instrument_mastery;
/*   91 */  uint8 unknown91;
/*   92 */  uint8 unknown92;
/*   93 */  uint8 unknown93;
/*   94 */  uint8 jam_fest;
/*   95 */  uint8 unknown95;
/*   96 */  uint8 unknown96;
// ===== Monk =====
/*   97 */  uint8 critical_mend;
/*   98 */  uint8 purify_body;
/*   99 */  uint8 unknown99;
/*  100 */  uint8 rapid_feign;
/*  101 */  uint8 return_kick;
// ===== Rogue =====
/*  102 */  uint8 escape;
/*  103 */  uint8 poison_mastery;
/*  104 */  uint8 double_riposte; // all "riposte" users
/*  105 */  uint8 unknown105;
/*  106 */  uint8 unknown106;
/*  107 */  uint8 purge_poison; // rogue
// ===== Warrior =====
/*  108 */  uint8 flurry;
/*  109 */  uint8 rampage;
/*  110 */  uint8 area_taunt;
/*  111 */  uint8 warcry;
/*  112 */  uint8 bandage_wound;
// ===== (Other) =====
/*  113 */  uint8 spell_casting_reinforcement_mastery; // all "pure" casters
/*  114 */  uint8 unknown114;
/*  115 */  uint8 extended_notes; // bard
/*  116 */  uint8 dragon_punch; // monk
/*  117 */  uint8 strong_root; // wizard
/*  118 */  uint8 singing_mastery; // bard
/*  119 */  uint8 body_and_mind_rejuvenation; // paladin, ranger, bard
/*  120 */  uint8 physical_enhancement; // paladin, ranger, bard
/*  121 */  uint8 adv_trap_negotiation; // rogue, bard
/*  122 */  uint8 acrobatics; // all "safe-fall" users
/*  123 */  uint8 scribble_notes; // bard
/*  124 */  uint8 chaotic_stab; // rogue
/*  125 */  uint8 pet_discipline; // all pet classes except enchanter
/*  126 */  uint8 unknown126;
/*  127 */  uint8 unknown127;
    } named;
  } class_skills;
};
#endif


/*
** Drop Item On Ground
** Length: 226 Octets
** OpCode: MakeDropCode
*/

struct makeDropStruct
{
/*0000*/ uint8_t  opCode;                 // 0x2d
/*0001*/ uint8_t  version;                // 0x20
/*0002*/ uint8_t  unknown0002[8];         // ***Placeholder
/*0010*/ uint16_t itemNr;                 // Item ID
/*0012*/ uint8_t  unknown0012[2];         // ***Placeholder
/*0014*/ uint16_t dropId;                 // DropID
/*0016*/ uint8_t  unknown0016[10];        // ***Placeholder
/*0026*/ float    yPos;                   // Y Position
/*0030*/ float    xPos;                   // X Position
/*0034*/ float    zPos;                   // Z Position
/*0038*/ uint8_t  unknown0054[4];         // ***Placeholder
/*0042*/ char     idFile[16];             // ACTOR ID
/*0058*/ uint8_t  unknown0074[168];       // ***Placeholder
};

/*
** Remove Drop Item On Ground
** Length: 10 Octets
** OpCode: RemDropCode
*/

struct remDropStruct
{
/*0000*/ uint8_t  opCode;                 // 0x2c
/*0001*/ uint8_t  version;                // 0x20
/*0002*/ uint16_t dropId;                 // DropID - Guess
/*0004*/ uint8_t  unknown0004[2];         // ***Placeholder
/*0006*/ uint16_t spawnId;                // Pickup ID - Guess
/*0008*/ uint8_t  unknown0008[2];         // ***Placeholder
};

/*
** Consider Struct
** Length: 26 Octets
** OpCode: considerCode
*/

struct considerStruct{
/*0000*/ uint8_t  opCode;                 // 0x37
/*0001*/ uint8_t  version;                // 0x21
/*0002*/ uint16_t playerid;               // PlayerID
/*0004*/ //uint8_t  unknown0004[2];         // ***Placeholder - Removed Feb 13, 2002
/*0006*/ uint16_t targetid;               // TargetID
/*0008*/ //uint8_t  unknown0008[2];         // ***Placeholder
/*0010*/ int32_t  faction;                // Faction
/*0014*/ int32_t  level;                  // Level
/*0018*/ int32_t  curHp;                  // Current Hitpoints
/*0022*/ int32_t  maxHp;                  // Maximum Hitpoints
/*0026*/ int32_t  unknown0026;            // unknown
};

/*
** Spell Casted On
** Length: 38 Octets
** OpCode: castOnCode
*/

struct castOnStruct
{
/*0000*/ uint8_t  opCode;                 // 0x46
/*0001*/ uint8_t  version;                // 0x20
/*0002*/ uint16_t targetId;               // Target ID
/*0004*/ uint8_t  unknown0004[2];         // ***Placeholder
/*0006*/ int16_t  sourceId;                // ***Source ID
/*0008*/ uint8_t  unknown0008[2];         // ***Placeholder
/*0010*/ uint8_t  unknown0010[24];        // might be some spell info?
/*0034*/ uint16_t spellId;                // Spell Id
/*0036*/ uint8_t  unknown0036[2];         // ***Placeholder
};

/*
** New Spawn
** Length: xxx Octets
** OpCode: NewSpawnCode
*/

struct newSpawnStruct
{
/*0000*/ uint8_t opCode;                 // 0x49
/*0001*/ uint8_t version;                // 0x21
/*0002*/ int32_t unknown0002;            // ***Placeholder
/*0006*/ struct spawnStruct spawn;       // Spawn Information
};

/*
** Spawn Death Blow
** Length: 18 Octets
** OpCode: NewCorpseCode
*/

struct newCorpseStruct
{
/*0000*/ uint8_t  opCode;                 // 0x4a
/*0001*/ uint8_t  version;                // 0x20
/*0002*/ uint16_t spawnId;                // Id of spawn that died
/*0004*/ uint16_t killerId;               // Killer
/*0006*/ uint8_t  unknown0006[4];         // ***Placeholder
/*0010*/ uint16_t spellId;                // ID of Spell
/*0012*/ int8_t   type;                   // Spell, Bash, Hit, etc...
/*0013*/ uint8_t  unknown0013;            // ***Placeholder
/*0014*/ uint16_t damage;                 // Damage
/*0016*/ uint8_t  unknown0016[2];         // ***Placeholder
/*0018*/ uint8_t  unknown0018[4];         // ***Placeholder
};

/*
** Money Loot
** Length: 22 Octets
** OpCode: MoneyOnCorpseCode
*/

struct moneyOnCorpseStruct
{
/*0000*/ uint8_t  opCode;                 // 0x50
/*0001*/ uint8_t  version;                // 0x20
/*0002*/ uint8_t  unknown0002[4];         // ***Placeholder
/*0006*/ uint32_t platinum;               // Platinum Pieces
/*0010*/ uint32_t gold;                   // Gold Pieces
/*0014*/ uint32_t silver;                 // Silver Pieces
/*0018*/ uint32_t copper;                 // Copper Pieces
};

/*
** Item received by the player
** Length: 246 Octets
** OpCode: ItemOnCorpseCode and ItemTradeCode
*/

struct tradeItemInStruct
{
/*0000*/ uint8_t  opCode;                 // 0x52
/*0001*/ uint8_t  version;                // 0x20
/*0002*/ struct itemStruct item;          // Refer to itemStruct for members
};

struct itemOnCorpseStruct
{
/*0000*/ uint8_t  opCode;                 // 0x52
/*0001*/ uint8_t  version;                // 0x20
/*0002*/ struct itemStruct item;          // Refer to itemStruct for members
};

/*
** Stamina
** Length: 8 Octets
** OpCode: staminaCode
*/

struct staminaStruct {
/*0000*/ uint8_t opCode;                   // 0x57
/*0001*/ uint8_t version;                  // 0x21
/*0002*/ int16_t food;                     // (low more hungry 127-0)
/*0004*/ int16_t water;                    // (low more thirsty 127-0)
/*0006*/ int16_t fatigue;                  // (high more fatigued 0-100)
};

/*
** Battle Code
** Length: 30 Octets
** OpCode: ActionCode
*/

struct action2Struct
{
  /*0000*/ uint8_t  opCode;               // 0x58
  /*0001*/ uint8_t  version;              // 0x20
  /*0002*/ uint16_t target;               // Target ID
  /*0004*/ uint16_t source;               // Source ID
  /*0006*/ uint8_t  type;                 // Bash, kick, cast, etc.
  /*0007*/ uint8_t  unknown0007;          // ***Placeholder
  /*0008*/ int16_t  spell;                // SpellID
  /*0010*/ int32_t  damage;               // Amount of Damage
  /*0014*/ uint8_t  unknown0014[16];      // ***Placeholder
};


struct actionStruct
{
/*0000*/ uint8_t  opCode;                 // 0x58
/*0001*/ uint8_t  version;                // 0x20
/*0002*/ uint16_t target;                 // Target ID
/*0004*/ uint8_t  unknown0004[2];         // ***Placeholder
/*0006*/ uint16_t source;                 // SourceID
/*0008*/ uint8_t  unknown0008[2];         // ***Placeholder
/*0010*/ int8_t   type;                   // Casts, Falls, Bashes, etc...
/*0011*/ uint8_t  unknown0011;            // ***Placeholder
/*0012*/ int16_t  spell;                  // SpellID
/*0014*/ int32_t  damage;                 // Amount of Damage
/*0018*/ uint8_t  unknown0018[12];        // ***Placeholder
};

/*
** New Zone Code
** Length: 450 Octets
** OpCode: NewZoneCode
*/

struct newZoneStruct
{
/*0000*/ uint8_t opCode;                   // 0x5b
/*0001*/ uint8_t version;                  // 0x20
/*0002*/ char    charName[30];             // Character name
/*0032*/ uint8_t unknown0032[34];          // unknown
/*0066*/ char    shortName[32];            // Zone Short Name
/*0098*/ char    longName[180];            // Zone Long Name
/*0278*/ uint8_t unknown0278[172];         // *** Placeholder
};

/*
** Zone Spawns
** Length: 6Octets + Variable Length Spawn Data
** OpCode: ZoneSpawnsCode
*/

struct zoneSpawnsStruct
{
/*0000*/ uint8_t opCode;                     // 0x61
/*0001*/ uint8_t version;                    // 0x21
/*0002*/ struct spawnZoneStruct spawn[0];    // Variable number of spawns
};

/*
** client changes target struct
** Length: 4 Octets
** OpCode: clientTargetCode
*/

struct clientTargetStruct
{
/*0000*/ uint8_t  opCode;                 // 0x62
/*0001*/ uint8_t  version;                // 0x21
/*0002*/ uint16_t newTarget;              // Target ID
/*0004*/ //uint16_t unknown0004;          // ***Placeholder - Removed Feb 13, 2002
};

/*
** Item belonging to a player
** Length: 246 Octets
** OpCode: PlayerItemCode
*/

struct playerItemStruct
{
/*0000*/ uint8_t  opCode;                 // 0x64
/*0001*/ uint8_t  version;                // 0x21
/*0002*/ struct itemStruct item;          // Refer to itemStruct for members
};

/*
** Book belonging to player
** Length: 205 Octets
** OpCode: PlayerBookCode
*/

struct playerBookStruct
{
/*0000*/ uint8_t  opCode;                 // 0x65
/*0001*/ uint8_t  version;                // 0x21
/*0002*/ struct itemStruct item;          // Refer to itemStruct for members
};

/*
** Container Struct
** Length: 216 Octets
** OpCode: PlayerContainerCode
**
*/

struct playerContainerStruct
{
/*0000*/ uint8_t  opCode;                 // 0x66
/*0001*/ uint8_t  version;                // 0x21
/*0002*/ struct itemStruct item;          // Refer to itemStruct for members
};

/*
** Summoned Item - Player Made Item?
** Length: 244 Octets
** OpCode: summonedItemCode
*/

struct summonedItemStruct
{
/*0000*/ uint8_t  opCode;                 // 0x78
/*0001*/ uint8_t  version;                // 0x21
/*0002*/ struct itemStruct item;          // Refer to itemStruct for members
};

/*
** Info sent when you start to cast a spell
** Length: 18 Octets
** OpCode: StartCastCode
*/

struct startCastStruct 
{
/*0000*/ uint8_t  opCode;                 // 0x7e
/*0001*/ uint8_t  version;                // 0x21
/*0002*/ int16_t  unknown0002;            // ***Placeholder
/*0004*/ uint16_t spellId;                // Spell ID
/*0006*/ int16_t  unknown0006;            // ***Placeholder
/*0008*/ //int16_t  unknown0008;            // ***Placeholder
/*0010*/ uint16_t targetId;               // The current selected target
/*0014*/ uint8_t  unknown0014[4];         // ***Placeholder 
};

/*
** New Mana Amount
** Length: 18 Octets
** OpCode: manaDecrementCode
*/

struct manaDecrementStruct
{
/*0000*/ uint8_t opCode;                   // 0x7f
/*0001*/ uint8_t version;		   // 0x21
/*0002*/ int16_t newMana;                  // New Mana AMount
/*0004*/ int16_t spellID;                  // Last Spell Cast
};

/*
** Special Message
** Length: 6 Octets + Variable Text Length
** OpCode: SPMesgCode
*/
struct spMesgStruct
{
/*0000*/ uint8_t opCode;                  // 0x80
/*0001*/ uint8_t version;                 // 0x21
/*0002*/ int32_t msgType;                 // Type of message
/*0006*/ char    message[0];              // Message, followed by four Octets?
};

/*
** Spell Action Struct
** Length: 10 Octets
** OpCode: BeginCastCode
*/
struct beginCastStruct
{
/*0000*/ uint8_t  opCode;                 // 0x82
/*0001*/ uint8_t  version;                // 0x21
/*0002*/ uint16_t spawnId;                // Id of who is casting
/*0004*/ uint16_t spellId;                // Id of spell
/*0006*/ int16_t  param1;                 // Paramater 1
/*0008*/ int16_t  param2;                 // Paramater 2
};

/*
** Spell Action Struct
** Length: 14 Octets
** OpCode: MemSpellCode
*/

struct memSpellStruct
{
/*0000*/ uint8_t  opCode;                 // 0x82
/*0001*/ uint8_t  version;                // 0x21
/*0002*/ uint16_t spawnId;                // Id of who is casting
/*0004*/ int16_t  unknown0004;            // ***Placeholder
/*0006*/ uint16_t spellId;                // Id of spell
/*0008*/ int16_t  param1;                 // Paramater 1
/*0010*/ int16_t  param2;                 // Paramater 2
/*0012*/ int16_t  param3;                 // Parameter 3
};

/*
** Skill Increment
** Length: 10 Octets
** OpCode: SkillIncCode
*/

struct skillIncStruct
{
/*0000*/ uint8_t  opCode;                 // 0x89
/*0001*/ uint8_t  version;                // 0x21
/*0002*/ uint16_t skillId;                // Id of skill
/*0004*/ uint8_t  unknown0004[2];         // ***Placeholder
/*0006*/ int16_t  value;                  // New value of skill
/*0008*/ uint8_t  unknown0008[2];         // ***Placeholder
};

/*
** When somebody changes what they're wearing
**      or give a pet a weapon (model changes)
** Length: 18 Octets
** Opcode: WearChangeCode
*/

struct wearChangeStruct
{
/*0000*/ uint8_t  opCode;                 // 0x92
/*0001*/ uint8_t  version;                // 0x20
/*0002*/ uint16_t spawnId;                // SpawnID
/*0004*/ uint8_t  wearSlotId;             // Slot ID
/*0005*/ uint8_t  unknown0005[1];            // unknown
/*0006*/ uint16_t newItemId;              // Item ID see weaponsX.h or util.cpp
/*0008*/ uint8_t  unknown0008[2];         // unknown
/*0010*/ uint32_t color;                  // color
};

/*
** Level Update
** Length: 14 Octets
** OpCode: LevelUpUpdateCode
*/

struct levelUpUpdateStruct
{
/*0000*/ uint8_t  opCode;                 // 0x98
/*0001*/ uint8_t  version;                // 0x21
/*0002*/ uint32_t level;                  // New level
/*0006*/ uint32_t levelOld;               // Old level
/*0010*/ uint32_t exp;                    // Current Experience
};

/*
** Experience Update
** Length: 6 Octets
** OpCode: ExpUpdateCode
*/

struct expUpdateStruct
{
/*0000*/ uint8_t  opCode;                 // 0x99
/*0001*/ uint8_t  version;                // 0x21
/*0002*/ uint16_t exp;                    // experience value  x/330
/*0004*/ uint16_t unknown0004;            // ***Place Holder
};

/*
** Alternate Experience Update
** Length: 10 Octets
** OpCode: AltExpUpdateCode
*/
struct altExpUpdateStruct
{
/*0000*/ uint8_t  opCode;                 // 0x23
/*0001*/ uint8_t  version;                // 0x22
/*0002*/ uint16_t altexp;                 // alt exp x/330
/*0004*/ uint16_t unknown0004;            // ***Place Holder
/*0006*/ uint16_t aapoints;               // current number of AA points
/*0008*/ uint8_t  percent;                // percentage in integer form
/*0009*/ uint8_t  unknown0009;            // ***Place Holder
};

/*
** Spawn Position Update
** Length: 6 Octets + Number of Updates * 15 Octets
** OpCode: MobUpdateCode
*/

struct mobUpdateStruct
{
/*0000*/ uint8_t  opCode;                 // 0xa1 - Used to be 0x85
/*0001*/ uint8_t  version;                // 0x20
/*0002*/ int32_t  numUpdates;             // Number of SpawnUpdates
/*0006*/ struct spawnPositionUpdate       // Spawn Position Update
                     spawnUpdate[0];
};

/*
** Type:   Zone Change Request (before hand)
** Length: 70 Octets
** OpCode: ZoneChangeCode
*/

struct zoneChangeStruct
{
/*0000*/ uint8_t  opCode;               // 0xa3
/*0001*/ uint8_t  version;              // 0x20
/*0002*/ char     charName[32];		// Character Name
/*0034*/ uint8_t  unknown0050[32];	// *** Placeholder
/*0066*/ uint32_t zoneId;               // zone Id
/*0070*/ uint32_t unknown;              // unknown
};

/*
** Spawn HP Update
** Length: 14 Octets
** OpCode: HPUpdateCode
*/

struct hpUpdateStruct
{
/*0000*/ uint8_t  opCode;                 // 0xb2
/*0001*/ uint8_t  version;                // 0x20
/*0002*/ uint16_t spawnId;                // Id of spawn to update
/*0004*/ int16_t  unknown0004;            // ***Placeholder
/*0006*/ int16_t  curHp;                  // Current hp of spawn
/*0008*/ int16_t  unknown0008;            // ***Placeholder
/*0010*/ int16_t  maxHp;                  // Maximum hp of spawn
/*0012*/ int16_t  unknown0012;            // ***Placeholder
};

/*
** Inspecting Information
** Length: 1042 Octets
** OpCode: InspectDataCode
*/

struct inspectDataStruct
{
/*0000*/ uint8_t  opCode;                 // 0xb6
/*0001*/ uint8_t  version;                // 0x20
/*0002*/ uint8_t  unknown0002[36];        // ***Placeholder
/*0038*/ char     itemNames[21][32];      // 21 items with names 
                                          //    32 characters long.
/*0710*/ int16_t  icons[21];              // Icon Information
/*0752*/ uint8_t  unknown0756[2];         // ***Placeholder
/*0754*/ char     mytext[200];            // Player Defined Text Info
/*0954*/ uint8_t  unknown0958[88];        // ***Placeholder
};

/*
** Reading Book Information
** Length: Variable Length Octets
** OpCode: BookTextCode
*/

struct bookTextStruct
{
/*0000*/ uint8_t opCode;                   // 0xce
/*0001*/ uint8_t version;                  // 0x20
/*0002*/ char    text[0];                  // Text of item reading
};

/*
** Interrupt Casting
** Length: 6 Octets + Variable Length Octets
** Opcode: BadCastCode
*/

struct badCastStruct
{
/*0000*/ uint8_t  opCode;                   // 0xd3
/*0001*/ uint8_t  version;                  // 0x21
/*0002*/ uint16_t spawnId;                  // Id of who is casting
/*0004*/ char     message[0];               // Text Message
};

/*
** Info sent when trading an item
** Length: 258 Octets
** OpCode: tradeItemOutCode
*/

struct tradeItemOutStruct
{
/*0000*/ uint8_t  opCode;                 // 0xdf
/*0001*/ uint8_t  version;                // 0x20
/*0002*/ uint8_t  unknown0002[4];         // ***Placeholder
/*0008*/ int8_t   itemtype;               // Type of item
/*0009*/ struct itemStruct item;          // Refer to itemStruct for members
/*0253*/ uint8_t  unknown0253[5];         // ***Placeholder
};

/*
** Random Number Request
** Length: 10 Octets
** OpCode: RandomCode
*/
struct randomStruct 
{
/*0000*/ uint8_t  opCode;                 // 0xe7
/*0001*/ uint8_t  version;                // 0x21
/*0002*/ uint32_t bottom;                 // Low number
/*0006*/ uint32_t top;                    // High number
};

/*
** Time of Day
** Length: 8 Octets
** OpCode: TimeOfDayCode
*/

struct timeOfDayStruct
{
/*0000*/ uint8_t  opCode;                 // 0xf2
/*0001*/ uint8_t  version;                // 0x20
/*0002*/ int8_t   hour;                   // Hour (1-24)
/*0003*/ int8_t   minute;                 // Minute (0-59)
/*0004*/ int8_t   day;                    // Day (1-28)
/*0005*/ int8_t   month;                  // Month (1-12)
/*0006*/ uint16_t year;                   // Year
};

/*
** Player Position Update
** Length: 17 Octets
** OpCode: PlayerPosCode
*/

struct playerPosStruct
{
/*0000*/ uint8_t  opCode;                 // 0xf3
/*0001*/ uint8_t  version;                // 0x20
/*0002*/ uint16_t spawnId;                // Id of player
/*0004*/ uint8_t  unknown0004[1];         // ***Placeholder
/*0005*/ int8_t   heading;                // Current heading of player
/*0006*/ int8_t   deltaHeading;           // Heading Change
/*0007*/ int16_t  yPos;                   // Players Y Position
/*0009*/ int16_t  xPos;                   // Players X Position
/*0011*/ int16_t  zPos;                   // Players Z Position
/*0013*/ signed   deltaY:10;              // Y Velocity
         unsigned spacer1:1;              // ***Placeholder
         signed   deltaZ:10;              // Z Velocity
         unsigned spacer2:1;              // ***Placeholder
         signed   deltaX:10;              // X Velocity
};

/*
** Spawn Appearance
** Length: 14 Octets
** OpCode: spawnAppearanceCode
*/

struct spawnAppearanceStruct
{
/*0000*/ uint8_t  opCode;                 // 0xf5
/*0001*/ uint8_t  version;                // 0x20
/*0002*/ uint16_t spawnId;                // ID of the spawn
/*0004*/ int16_t  unknown0004;            // ***Placeholder
/*0006*/ int16_t  type;                   // Type of data sent
/*0008*/ int16_t  unknown0008;            // ***Placeholder
/*0010*/ uint32_t paramter;               // Values associated with the type
};

/*
** Compressed Player Items Struct
** Length: Variable Octets
** Opcodes: CPlayerItemCode
*/

struct cPlayerItemsStruct
{
/*0000*/ uint8_t  opCode;                 // 0xf6 or 0xf7
/*0001*/ uint8_t  version;                // 0x21
/*0002*/ uint16_t count;                  // Number of packets contained
/*0004*/ uint8_t  compressedData[0];      // All the packets compressed together
};

/*
** Compressed Door Struct
** Length: 4 + (count * sizeof(doorStruct)) Octets
** OpCode: CompressedDoorSpawnCode
*/

struct cDoorSpawnsStruct
{
/*0000*/ uint8_t  opCode;                // 0xf7
/*0001*/ uint8_t  version;               // 0x21
/*0002*/ uint16_t count;                 // number of doors
/*0004*/ struct doorStruct doors[0];     // door structures
};
typedef struct cDoorSpawnsStruct doorSpawnsStruct; // alias

/*
**               Structures that are not being currently used
 *               (except for logging)
*/

struct bindWoundStruct
{
/*0000*/ uint8_t  opCode;               // ????
/*0001*/ uint8_t  version;		// ????
/*0002*/ uint16_t playerid;             // TargetID
/*0004*/ uint8_t  unknown0004[2];       // ***Placeholder
/*0006*/ uint32_t hpmaybe;              // Hitpoints -- Guess
};

struct inspectedStruct
{
/*0000*/ uint8_t  opCode;              // ????
/*0001*/ uint8_t  version;             // ????
/*0002*/ uint16_t inspectorid;         // Source ID
/*0004*/ uint8_t  unknown0004[2];      // ***Placeholder
/*0006*/ uint16_t inspectedid;         // Target ID - Should be you
/*0008*/ uint8_t  unknown0008[2];      // ***Placeholder
};

struct attack1Struct
{
/*0000*/ uint8_t  opCode;                 // ????
/*0001*/ uint8_t  version;                // ????
/*0002*/ uint16_t spawnId;                // Spawn ID
/*0004*/ int16_t  param1;                 // ***Placeholder
/*0004*/ int16_t  param2;                 // ***Placeholder
/*0004*/ int16_t  param3;                 // ***Placeholder
/*0004*/ int16_t  param4;                 // ***Placeholder
/*0004*/ int16_t  param5;                 // ***Placeholder
};

struct attack2Struct{
/*0000*/ uint8_t  opCode;                 // ????
/*0001*/ uint8_t  version;                // ????
/*0002*/ uint16_t spawnId;                // Spawn ID
/*0004*/ int16_t  param1;                 // ***Placeholder
/*0004*/ int16_t  param2;                 // ***Placeholder
/*0004*/ int16_t  param3;                 // ***Placeholder
/*0004*/ int16_t  param4;                 // ***Placeholder
/*0004*/ int16_t  param5;                 // ***Placeholder
};

struct newGuildInZoneStruct
{
/*0000*/ uint8_t  opCode;                 // ????
/*0001*/ uint8_t  version;                // ????
/*0002*/ uint8_t  unknown0002[8];         // ***Placeholder
/*0010*/ char     guildname[56];          // Guildname
};

struct moneyUpdateStruct{
/*0000*/ uint8_t  opCode;                 // ????
/*0001*/ uint8_t  version;                // ????
/*0002*/ uint16_t unknown0002;            // ***Placeholder
/*0006*/ uint8_t  cointype;               // Coin Type
/*0007*/ uint8_t  unknown0007[3];         // ***Placeholder
/*0010*/ uint32_t amount;                 // Amount
};
typedef struct moneyUpdateStruct moneyThingStruct; 

/* Memorize slot operations, mem, forget, etc */

struct memorizeSlotStruct
{
/*0000*/ uint8_t  opCode;                   // ????
/*0001*/ uint8_t  version;                  // ????
/*0002*/ int8_t   slot;                     // Memorization slot (0-7)
/*0003*/ uint8_t  unknown0003[3];           // ***Placeholder
/*0006*/ uint16_t spellId;                  // Id of spell 
                                            // (offset of spell in spdat.eff)
/*0008*/ uint8_t  unknown0008[6];           // ***Placeholder 00,00,01,00,00,00
};

// Restore structure packing to default
#pragma pack()

#endif // EQSTRUCT_H

//. .7...6....,X....D4.M.\.....P.v..>..W....
//123456789012345678901234567890123456789012
//000000000111111111122222222223333333333444
