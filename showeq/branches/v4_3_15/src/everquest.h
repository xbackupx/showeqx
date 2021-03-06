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

#include "../conf.h"

#ifdef __FreeBSD__
#include <sys/types.h>
#else
#include <stdint.h>
#endif

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
#define DEITY_BERT			201	
#define DEITY_RODCET			212
#define DEITY_VEESHAN			216

//Team numbers for Deity teams
#define DTEAM_GOOD			1
#define DTEAM_NEUTRAL			2
#define DTEAM_EVIL			3
#define DTEAM_OTHER			5

//Team numbers for Race teams
#define RTEAM_HUMAN			1
#define RTEAM_ELF			2
#define RTEAM_DARK			3
#define RTEAM_SHORT			4
#define RTEAM_OTHER			5

//Maximum limits of certain types of data
#define MAX_KNOWN_SKILLS                74
#define MAX_KNOWN_LANGS                 25
#define MAX_SPELLBOOK_SLOTS             400

//Item Flags
#define ITEM_NORMAL                     0x0000
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
#define ITEM_VERSION                    0xFFFF

// Item spellId no spell value
#define ITEM_SPELLID_NOSPELL            0xffff

// Item Field Count
#define ITEM_FIELD_SEPERATOR_COUNT      117
#define ITEM_CMN_FIELD_SEPERATOR_COUNT  102

//Combat Flags
#define COMBAT_MISS						0
#define COMBAT_BLOCK					-1
#define COMBAT_PARRY					-2
#define COMBAT_RIPOSTE					-3
#define COMBAT_DODGE					-4

#define PLAYER_CLASSES     15
#define PLAYER_RACES       14

enum ItemPacketType
{
  ItemPacketViewLink		= 0x00,
  ItemPacketMerchant		= 0x64,
  ItemPacketLoot		= 0x66,
  ItemPacketTrade		= 0x67,
  ItemPacketCharInventory	= 0x69,
  ItemPacketSummonItem		= 0x6a,
  ItemPacketWorldContainer       = 0x6b
};

/*
** Item types
**
*/
enum ItemType
{
	ItemTypeCommon		= 0,
	ItemTypeContainer	= 1,
	ItemTypeBook		= 2
};

/*
** Compiler override to ensure
** byte aligned structures
*/
#pragma pack(1)

/*
**            Generic Structures used in specific
**                      structures below
*/

// OpCode stuff (all kinda silly, but until we stop including the OpCode everywhere)...
struct opCodeStruct
{
    int16_t opCode;

  // kinda silly -- this is required for us to be able to stuff them in a QValueList
  bool operator== ( const opCodeStruct t ) const
  {
    return ( opCode == t.opCode);
  }
  bool operator== ( uint16_t opCode2 ) const
  {
    return ( opCode == opCode2 );
  }
};


/* 
 * Used in charProfileStruct
 * Size: 4 bytes
 */
struct Color_Struct
{
  union
  {
    struct
    {
      uint8_t red;
      uint8_t green;
      uint8_t blue;
      uint8_t unused;
    } rgb;
    uint32_t color;
  };
};

/*
** Buffs
** Length: 16 Octets
** Used in:
**    charProfileStruct(2d20)
*/

struct spellBuff
{
/*0000*/  int8_t      slotid;             //
/*0001*/  int8_t      level;              // Level of person who casted buff
/*0002*/  int8_t      unknown0002[2];     //
/*0004*/  uint16_t    spellid;            // Spell
/*0006*/  int8_t      packing0006[2];     // Purely packing
/*0008*/  int32_t     duration;           // Duration in ticks
/*0012*/  int32_t     unknown0012;        //
/*0016*/  
};

/*
** Item Packet Struct - Works on a variety of item operations
** Packet Types: See ItemPacketType enum
** Length: Variable
** OpCode: ItemCode
*/
struct itemPacketStruct
{
/*000*/	ItemPacketType	packetType;       // See ItemPacketType for more info.
/*004*/	char		serializedItem[0];
/*xx*/
};

/*
** Item Info Request Struct 
** OpCode: ItemInfoCode
*/
struct itemInfoReqStruct
{
/*000*/ uint32_t itemNr;                  // ItemNr 
/*005*/ uint32_t requestSeq;              // Request sequence number
/*008*/ char     name[64];                // Item name
/*072*/
};

/*
** Item Info Response Struct
** Length: Variable
** OpCode: ItemInfoCode
*/
struct itemInfoStruct
{
/*000*/	uint32_t	requestSeq;       // Corresponds to sequence # in req
/*004*/	char		serializedItem[0];
/*xxx*/
};

/*
** Simple Spawn Update
** Length: 14 Octets
** OpCode: MobUpdateCode
*/

struct spawnPositionUpdate 
{
/*0000*/ int16_t  spawnId;
/*0002*/ int64_t  y:19, z:19, x:19, u3:7;
         unsigned heading:12;
         signed unused2:4;
/*0010*/
};


/* 
** Generic Spawn Struct 
** Length: 255 Octets 
** Used in: 
**   spawnZoneStruct
**   dbSpawnStruct
**   petStruct
**   newSpawnStruct
*/ 

struct spawnStruct
{
/*000*/ int32_t  race;             // race
/*004*/ int8_t   eyecolor1;        // left eye color
/*005*/ int8_t   eyecolor2;        // right eye color
/*006*/ union
        {
	  /*006*/ int8_t  face;             // face style
	  /*006*/ int8_t  woad;             // Barbarian-only WOAD
        };
/*007*/ int8_t   aa_title;         // 0=none,1=general,2=archetype,3=class
/*008*/ uint8_t  class_;		  // class
/*009*/ uint8_t  level;
/*010*/ char     unknown010[2];   // *** Placeholder
/*012*/ int8_t   curHp;
/*013*/ uint8_t  afk;             // 0=not afk, 1=afk
/*014*/ union
        {
	  /*014*/ uint8_t equip_chest2;
	  /*014*/ uint8_t mount_color; // drogmor: 0=white,1=black,2=green,3=red
	                               // horse: 0=brown,1=white,2=black,3=tan
        }; 
/*015*/ uint8_t  NPC;             // 0=player,1=npc,2=pc corpse,3=npc corpse,
                                  // 4=???,5=unknown spawn,10=self
/*016*/ uint8_t  beard;           // beard style
/*017*/ uint8_t  beardcolor;      // beard color
/*018*/ uint8_t  hairstyle;       // hair style
/*019*/ uint8_t  haircolor;       // hair color
/*020*/ uint8_t  invis;           // 0=visible,1=invisible
/*021*/ uint8_t  unknown021;      // *** Placeholder
/*022*/ int8_t   maxHp;           // max hp
/*023*/ uint8_t  pvp;             // 0=Not pvp,1=pvp
/*024*/ uint8_t  light;           // Light intensity
/*025*/ uint8_t  unknown025;      // *** Placeholder
/*026*/ uint8_t  lfg;             // 0=Not lfg,1=lfg
/*027*/ uint16_t heading;         // spawn heading
/*029*/ uint8_t  deltaHeading;    // change in heading
/*030*/ uint8_t  animation;       // animation id
/*031*/ signed   deltaX:13;
        signed   x:19;
/*035*/ signed   y:19;
        signed   deltaZ:13;
/*039*/ signed   deltaY:13;
        signed   z:19;
/*043*/ uint8_t  anon;            // 0=normal,1=anon,2=roleplaying
/*044*/ uint8_t  gender;          // 0=male,1=female,2=other
/*045*/ uint16_t spawnId;         // Id of spawn
/*047*/ char     unknown047[3];
/*050*/ char     lastName[32];    // lastname
/*082*/ int32_t  equipment[9];	  // 0=helm, 1=chest, 2=arm, 3=bracer
                                  // 4=hand, 5=leg 6=boot, 7=melee1, 8=melee2
/*118*/ char     name[64];        // name
/*182*/ int32_t  dye_rgb[7];      // armor dye colors
/*210*/ uint8_t  unknown210[8];
/*218*/ float    size;            // Size
/*222*/ uint8_t  unknown222[5];
/*227*/ uint8_t  gm;              // 0=not GM,1=GM
/*228*/ uint8_t  unknown228[4];   // *** Placeholder
/*232*/ uint32_t guildID;         // GuildID
/*236*/ uint8_t  linkdead;        // 0=Not LD, 1=LD
/*237*/ uint32_t bodytype;        // Bodytype
/*241*/ int8_t   guild_rank;      // 0=member,1=officer,2=leader
/*242*/ uint8_t  unknown242[3];
/*245*/ uint32_t petOwnerId;      // If pet, the pet owner spawn id
/*249*/ int16_t  deity;           // deity
/*251*/ char     unknown251[7];
/*258*/
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
/*0000*/ spawnStruct spawn;
};

/*
** Generic Door Struct
** Length: 52 Octets
** Used in: 
**    cDoorSpawnsStruct(f721)
**
*/

struct doorStruct
{
/*0000*/ char     name[16];        // Filename of Door?
/*0016*/ float    y;               // y loc
/*0020*/ float    x;               // x loc
/*0024*/ float    z;               // z loc
/*0028*/ float    heading;         // heading
/*0032*/ uint8_t  unknown0028[7]; // ***Placeholder
/*0039*/ int8_t   auto_return;
/*0040*/ uint8_t  initialState;
/*0041*/ uint16_t holdstateforever;
/*0043*/ uint8_t  unknown043;
/*0044*/ uint8_t  doorId;          // door's id #
/*0045*/ uint8_t  opentypee;       
/*0046*/ uint8_t  unknown046; 
/*0047*/ uint8_t  size;           // size of door
/*0048*/ uint8_t  unknown040[8]; // ***Placeholder
/*0056*/
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
/*0258*/ char   zoneName[40];           // Zone Information
};

/*
** Pet spawn struct (pets pet and owner in one struct)
*/

struct petStruct
{
/*0000*/ struct spawnStruct owner;      // Pet Owner Information
/*0258*/ struct spawnStruct pet;        // Pet Infromation
};

/*
** Channel Message received or sent
** Length: 71 Octets + Variable Length + 4 Octets
** OpCode: ChannelMessageCode
*/

struct channelMessageStruct
{
/*0000*/ char     target[64];             // the target characters name
/*0064*/ char     sender[64];             // The senders name 
/*0128*/ uint8_t  language;               // Language
/*0129*/ uint8_t  unknown0129[3];         // ***Placeholder
/*0132*/ uint8_t  chanNum;                // Channel
/*0133*/ int8_t   unknown0133[11];            // ***Placeholder
/*0144*/ char     message[0];             // Variable length message
};

/*
** Compressed Item In Shop
** Length: variable
** OpCode cItemInShop
*/

struct cItemInShopStruct
{
/*0000*/ uint16_t count;                  // number of items in shop
/*0003*/ uint8_t  compressedData[0];      // All the packets compressed together
};

/*
** Server System Message
** Length: Variable Length
** OpCode: SysMsgCode
*/

struct sysMsgStruct
{
/*0000*/ char     message[0];             // Variable length message
};

/*
** Emote text
** Length: Variable Text
** OpCode: emoteTextCode
*/

struct emoteTextStruct
{
/*0000*/ uint8_t  unknown0002[2];         // ***Placeholder
/*0002*/ char     text[0];                // Emote `Text
};

/*
** Simple text messages
** Length: Variable Text
** OpCode: SimpleMessageCode
*/

struct simpleMessageStruct
{
/*0000*/ uint32_t messageFormat;          // Indicates the message format
/*0005*/ uint32_t color;                  // Message color
/*0008*/ uint32_t unknown;                // ***Placeholder
/*0012*/
};

/*
** Formatted text messages
** Length: Variable Text
** OpCode: emoteTextCode
*/

struct formattedMessageStruct
{
/*0000*/ uint8_t  unknown0002[4];         // ***Placeholder
/*0004*/ uint32_t messageFormat;          // Indicates the message format
/*0008*/ uint8_t  unknown0010[4];         // ***Placeholder (arguments?)
/*0012*/ char     messages[0];            // messages(NULL delimited)
/*0???*/ uint8_t  unknownXXXX[8];         // ***Placeholder
};

/*
** Corpse location
** Length: 18 Octets
** OpCode: corpseLocCode
*/

struct corpseLocStruct
{
/*0000*/ uint32_t spawnId;
/*0004*/ float    x;
/*0008*/ float    y;
/*0012*/ float    z;
};

/*
** Grouping Infromation
** Length: 138 Octets
** OpCode: groupinfoCode
*/

struct groupInfoStruct
{
/*0000*/ uint8_t  unknown0002[4];
/*0004*/ char     yourname[64];           // Player Name
/*0068*/ char     membername[64];         // Goup Member Name
/*0132*/ uint8_t  unknown0130[4];        // ***Placeholder
};
typedef struct groupInfoStruct groupMemberStruct; // new form

/*
** Grouping Invite
** Length 195 Octets
** Opcode GroupInviteCode
*/

struct groupInviteStruct
{
/*0000*/ char     yourname[64];           // Player Name
/*0064*/ char     membername[64];         // Invited Member Name
/*0128*/ uint8_t  unknown0130[65];        // ***Placeholder
};

/*
** Grouping Invite Answer - Decline
** Length 131 Octets
** Opcode GroupDeclineCode
*/

struct groupDeclineStruct
{
/*0000*/ char     yourname[64];           // Player Name
/*0064*/ char     membername[64];         // Invited Member Name
/*0128*/ uint8_t  reason;                 // Already in Group = 1, Declined Invite = 3
};

/*
** Grouping Invite Answer - Accept 
** Length 130 Octets
** Opcode GroupAcceptCode
*/

struct groupAcceptStruct
{
/*0000*/ char     yourname[64];           // Player Name
/*0064*/ char     membername[64];         // Invited Member Name
};

/*
** Grouping Removal
** Length 130 Octets
** Opcode GroupDeleteCode
*/

struct groupDeleteStruct
{
/*0000*/ char     yourname[64];           // Player Name
/*0064*/ char     membername[64];         // Invited Member Name
};

/*
** Client Zone Entry struct
** Length: 70 Octets
** OpCode: ZoneEntryCode (when direction == client)
*/

struct ClientZoneEntryStruct
{
/*0000*/ uint32_t unknown0000;            // ***Placeholder
/*0004*/ char     name[32];               // Player firstname
/*0036*/ uint8_t  unknown0036[28];        // ***Placeholder
/*0064*/ uint32_t unknown0064;            // unknown
};

/*
** Server Zone Entry struct
** Length: 390 Octets
** OpCode: ZoneEntryCode (when direction == server)
*/

struct ServerZoneEntryStruct
{
/*0000*/ uint32_t checksum;      // some kind of checksum
/*0004*/ uint8_t  gm;            // GM flag 0/1
/*0005*/ char	  name[64];      // Player name
/*0069*/ uint8_t  unknown069[7]; 
/*0076*/ float	  y;
/*0080*/ float	  x;
/*0084*/ float    z;
/*0088*/ float	  unknown088;
/*0092*/ float    unknown092;
/*0096*/ float    unknown096;
/*0100*/ uint8_t  unknown100[4];
/*0104*/ float    heading;
/*0108*/ float    unknown108;
/*0112*/ uint8_t  unknown112[88];
/*0200*/ uint8_t  walk_mode;     // 0=not walking,non-zero=walking
/*0201*/ uint8_t  linkdead;      // 1=LD,0=Not LD
/*0202*/ uint8_t  unknown202; 
/*0203*/ uint8_t  lfg;           // 1=LFG,0=Not FLG
/*0204*/ uint8_t  unknown204[4];
/*0208*/ union 
         {
	   struct 
	   {
	     /*0208*/ Color_Struct color_helmet;    // Color of helmet item
	     /*0212*/ Color_Struct color_chest;     // Color of chest item
	     /*0216*/ Color_Struct color_arms;      // Color of arms item
	     /*0220*/ Color_Struct color_bracers;   // Color of bracers item
	     /*0224*/ Color_Struct color_hands;     // Color of hands item
	     /*0228*/ Color_Struct color_legs;      // Color of legs item
	     /*0232*/ Color_Struct color_feet;      // Color of feet item
	     /*0236*/ Color_Struct color_primary;   // Color of primary item
	     /*0240*/ Color_Struct color_secondary; // Color of secondary item
	   } equipment_colors;
	   /*0208*/ uint32_t colors[9];             // Array elements correspond to struct equipment_colors above
         };
/*244*/  union 
         {
	   struct 
	   {
	     /*0244*/ uint32_t equip_helmet;    // Equipment: Helmet Visual
	     /*0248*/ uint32_t equip_chest;     // Equipment: Chest Visual
	     /*0252*/ uint32_t equip_arms;      // Equipment: Arms Visual
	     /*0256*/ uint32_t equip_bracers;   // Equipment: Bracers Visual
	     /*0260*/ uint32_t equip_hands;     // Equipment: Hands Visual
	     /*0264*/ uint32_t equip_legs;      // Equipment: Legs Visual
	     /*0268*/ uint32_t equip_feet;      // Equipment: Feet Visual
	     /*0272*/ uint32_t equip_primary;   // Equipment: Primary Visual
	     /*0276*/ uint32_t equip_secondary; // Equipment: Secondary Visual
	   } equipment;
	   /*0244*/ uint32_t equip[9];            // Array elements correspond to struct equipment above
         };
/*0280*/ uint16_t zoneId;
/*0282*/ uint16_t zoneInstance;
/*0284*/ uint8_t  unknown284[24];
/*0308*/ char     lastName[32];
/*0340*/ uint8_t  unknown340[8];
										// They control movement
/*0348*/ float	runspeed;          // Speed when running
/*0352*/ float	unknown0352[2];    //
/*0360*/ float	size;              // Size of character
										// Changing size works, but then movement stops!
/*0364*/ float    walkspeed;       // Speed when walking
/*0368*/ uint8_t  unknown0368;     //
/*0369*/ uint8_t  haircolor;       // Hair color
/*0370*/ uint8_t  beardcolor;      // Beard color
/*0371*/ uint8_t  eyecolor1;       // Left eye color
/*0372*/ uint8_t  eyecolor2;       // Right eye color
/*0373*/ uint8_t  hairstyle;       // Hair style
/*0374*/ uint8_t  beard;           // Beard type
/*0375*/ uint8_t  face;            // Face type 
/*0376*/ uint8_t  level;           // Player's Level
/*0377*/ uint8_t  unknown0377;     //
/*0378*/ uint8_t  gender;          // 0=male, 1=female, 2=neutral
/*0379*/ uint8_t  pvp;             // PVP=1, No PVP=0
/*0380*/ uint8_t  invis;           // 1=invis, other=not invis (not sure about this..)
/*0381*/ uint8_t  unknown0381;     //
/*0382*/ uint8_t  class_;          // Player's Class
/*0383*/ uint8_t  unknown383[13];  
/*0396*/ uint32_t race;            // Player's Race
/*0400*/ uint8_t  anon;            // 0=non-anon,1=anon,2=role,3=role/anon 
/*0400*/ uint8_t  unknown400[27];
/*0428*/ uint32_t deity;           // Player's Deity
/*0432*/ uint8_t  unknown432[4];
/*0436*/ uint32_t guildId;         // Current guild
/*0440*/ uint8_t  unknown440[12];
/*0452*/
};

/*
** Delete Spawn
** Length: 4 Octets
** OpCode: DeleteSpawnCode
*/

struct deleteSpawnStruct
{
/*0000*/ uint32_t spawnId;                // Spawn ID to delete
};

/* 
 * Used in charProfileStruct
 * Size: 2 octets
 */
struct AA_Array
{
/*000*/ uint8_t AA;
/*001*/ uint8_t value;
};

/*
** Player Profile
** Length: 8454 Octets
** OpCode: CharProfileCode
*/

struct charProfileStruct
{
/*0000*/ uint32_t  checksum;           // 
/*0004*/ char      name[64];           // Name of player sizes not right
/*0068*/ char      lastName[32];       // Last name of player sizes not right
/*0100*/ uint32_t  gender;             // Player Gender - 0 Male, 1 Female
/*0104*/ uint32_t  race;               // Player race
/*0108*/ uint32_t  class_;             // Player class
/*0112*/ uint32_t  aapoints;           // unspent ability points? (wrong?)
/*0116*/ uint32_t  level;              // Level of player (might be one byte)
/*0120*/ uint32_t  bind_zone_id;       // Zone player is bound in
/*0124*/ float     bind_x;             // Bind loc x coord
/*0128*/ float	   bind_y;             // Bind loc y coord
/*0132*/ float	   bind_z;             // Bind loc z coord
/*0136*/ uint8_t   unknown0136[4];     // *** PLaceholder
/*0140*/ uint32_t  deity;              // deity
/*0144*/ uint32_t  guildID;            // guildID
/*0148*/ uint32_t  birthdayTime;       // character birthday
/*0152*/ uint32_t  lastSaveTime;       // character last save time
/*0156*/ uint32_t  timePlayedMin;      // time character played
/*0160*/ uint8_t   fatigue;            // Sta bar % depleted (ie. 30 = 70% sta)
/*0161*/ uint8_t   pvp;                // 1=pvp, 0=not pvp
/*0162*/ uint8_t   unknown162;         // *** Placeholder
/*0163*/ uint8_t   anon;               // 2=roleplay, 1=anon, 0=not anon
/*0164*/ uint8_t   gm;                 // 1=gm, 0=not gm
/*0165*/ uint8_t   unknown0165[47];    // *** Placeholder
/*0212*/ uint8_t   haircolor;          // Player hair color
/*0213*/ uint8_t   beardcolor;         // Player beard color
/*0214*/ uint8_t   eyecolor1;          // Player left eye color
/*0215*/ uint8_t   eyecolor2;          // Player right eye color
/*0216*/ uint8_t   hairstyle;          // Player hair style
/*0217*/ uint8_t   beard;              // Player beard type
/*0218*/ uint8_t   unknown0218[2];     // *** Placeholder
/*0220*/ uint32_t  item_material[9];   // Item texture/material of worn/held items
/*0256*/ uint8_t   unknown0256[88];    // *** Placeholder
/*0344*/ AA_Array  aa_array[122];      // Length may not be right
/*0588*/ char 	   servername[64];     // length probably not right
/*0652*/ uint32_t  altexp;  	       // aaxp? (wrong?
/*0656*/ uint32_t  exp;                // Current Experience
/*0660*/ uint32_t  points;             // Unspent Practice points
/*0664*/ uint32_t  MANA;               // MANA
/*0668*/ uint32_t  curHp;              // current hp
/*0672*/ uint32_t  unknown0674;        // 0x05
/*0676*/ uint32_t  STR;                // Strength
/*0680*/ uint32_t  STA;                // Stamina
/*0684*/ uint32_t  CHA;                // Charisma
/*0688*/ uint32_t  DEX;                // Dexterity
/*0692*/ uint32_t  INT;                // Intelligence
/*0696*/ uint32_t  AGI;                // Agility
/*0700*/ uint32_t  WIS;                // Wisdom
/*0704*/ uint8_t   face;               // Player face
/*0705*/ uint8_t   unknown0705[11];    // *** Placeholder
/*0716*/ Color_Struct item_tint[9];    // RR GG BB 00
/*0752*/ uint8_t   languages[28];      // List of languages (MAX_KNOWN_LANGS)
/*0780*/ uint8_t   unknown0780[4];     // All 0x00
/*0784*/ int32_t   sSpellBook[400];    // List of the Spells in spellbook
/*2384*/ uint8_t   unknown2384[448];   // all 0xff after last spell    
/*2832*/ int32_t   sMemSpells[8];      // List of spells memorized
/*2864*/ uint8_t   unknown2864[36];    // *** Placeholder
/*2900*/ float     x;                  // Players x position
/*2904*/ float     y;                  // Players y position
/*2908*/ float     z;                  // Players z position
/*2912*/ float     heading;            // Players heading   
/*2916*/ uint8_t   unknown2918[4];     //*** Placeholder    
/*2920*/ uint32_t  platinum;           // Platinum Pieces on player
/*2924*/ uint32_t  gold;               // Gold Pieces on player
/*2928*/ uint32_t  silver;             // Silver Pieces on player
/*2932*/ uint32_t  copper;             // Copper Pieces on player
/*2936*/ uint32_t  platinum_bank;      // Platinum Pieces in Bank
/*2940*/ uint32_t  gold_bank;          // Gold Pieces in Bank
/*2944*/ uint32_t  silver_bank;        // Silver Pieces in Bank
/*2948*/ uint32_t  copper_bank;        // Copper Pieces in Bank
/*2952*/ uint32_t  platinum_cursor;    // Platinum Pieces on cursor
/*2956*/ uint32_t  gold_cursor;        // Gold Pieces on cursor
/*2960*/ uint32_t  silver_cursor;      // Silver Pieces on cursor
/*2964*/ uint32_t  copper_cursor;      // Copper Pieces on cursor
/*2968*/ uint32_t  platinum_shared;    // Shared platinum pieces
/*2972*/ uint8_t   unknown2972[20];    // Unknown - all zero
/*2992*/ uint32_t  skills[74];         // List of skills (MAX_KNOWN_SKILLS)
/*3288*/ uint8_t   unknown3288[412];   //
/*3700*/ uint16_t  zoneId;             // see zones.h
/*3702*/ uint16_t  zoneInstance;       // 
/*3704*/ spellBuff buffs[15];          // Buffs currently on the player
/*3944*/ char      groupMembers[6][64];// all the members in group, including self 
/*4328*/ uint8_t   unknown4330[4];     // *** Placeholder
/*4332*/ uint32_t  ldon_guk_points;    // Earned Deepest Guk points
/*4336*/ uint32_t  ldon_mir_points;    // Earned Deepest Guk points
/*4340*/ uint32_t  ldon_mmc_points;    // Earned Deepest Guk points
/*4344*/ uint32_t  ldon_ruj_points;    // Earned Deepest Guk points
/*4348*/ uint32_t  ldon_tak_points;    // Earned Deepest Guk points
/*4352*/ uint8_t   unknown4354[24];    // *** Placeholder
/*4376*/ uint32_t  ldon_avail_points;  // Available LDON points
/*4380*/ uint8_t   unknown4382[4];     // *** Placeholder
/*4384*/	
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
** Length: 94 Octets
** OpCode: MakeDropCode
*/

struct makeDropStruct
{
/*0000*/ uint8_t  unknown0000[8];         // ***Placeholder
/*0008*/ uint32_t itemNr;                 // Item ID? (wrong)
/*0012*/ uint32_t dropId;                 // DropID
/*0014*/ uint8_t  unknown0018[16];        // ***Placeholder
/*0032*/ float    z;                      // Z Position
/*0036*/ float    x;                      // X Position
/*0040*/ float    y;                      // Y Position
/*0044*/ char     idFile[16];             // ACTOR ID
/*0060*/ uint8_t  unknown0062[32];        // ***Placeholder
/*0092*/
};

/*
** Remove Drop Item On Ground
** Length: 10 Octets
** OpCode: RemDropCode
*/

struct remDropStruct
{
/*0000*/ uint16_t dropId;                 // DropID - Guess
/*0002*/ uint8_t  unknown0004[2];         // ***Placeholder
/*0004*/ uint16_t spawnId;                // Pickup ID - Guess
/*0006*/ uint8_t  unknown0008[2];         // ***Placeholder
};

/*
** Consider Struct
** Length: 30 Octets
** OpCode: considerCode
*/

struct considerStruct
{
/*0000*/ uint32_t playerid;               // PlayerID
/*0004*/ uint32_t targetid;               // TargetID
/*0008*/ int32_t  faction;                // Faction
/*0012*/ int32_t  level;                  // Level
/*0016*/ int32_t  curHp;                  // Current Hitpoints
/*0020*/ int32_t  maxHp;                  // Maximum Hitpoints
/*0024*/ int8_t   pvpCon;                 // Pvp con flag 0/1
/*0025*/ int8_t   unknown0025[3];         // unknown
/*0028*/
};

/*
** Spell Casted On
** Length: 38 Octets
** OpCode: castOnCode
*/

struct castOnStruct
{
/*0000*/ uint16_t targetId;               // Target ID
/*0002*/ uint8_t  unknown0002[2];         // ***Placeholder
/*0004*/ int16_t  sourceId;                // ***Source ID
/*0006*/ uint8_t  unknown0006[2];         // ***Placeholder
/*0008*/ uint8_t  unknown0008[24];        // might be some spell info?
/*0032*/ uint16_t spellId;                // Spell Id
/*0034*/ uint8_t  unknown0034[2];         // ***Placeholder
};

/*
** New Spawn
** Length: xxx Octets
** OpCode: NewSpawnCode
*/

struct newSpawnStruct
{
/*0000*/ struct spawnStruct spawn;       // Spawn Information
}; // 255

/*
** Spawn Death Blow
** Length: 18 Octets
** OpCode: NewCorpseCode
*/

struct newCorpseStruct
{
/*0000*/ uint32_t spawnId;                // Id of spawn that died
/*0004*/ uint32_t killerId;               // Killer
/*0008*/ uint32_t corpseid;               // corpses id
/*0012*/ int32_t  type;                   // corpse type?  
/*0016*/ uint32_t spellId;                // ID of Spell
/*0020*/ uint16_t zoneId;                 // Bind zone id
/*0022*/ uint16_t zoneInstance;           // Bind zone instance
/*0024*/ uint32_t damage;                 // Damage
/*0028*/ uint8_t  unknown0028[4];         // ***Placeholder
/*0032*/
};

/*
** Money Loot
** Length: 22 Octets
** OpCode: MoneyOnCorpseCode
*/

struct moneyOnCorpseStruct
{
/*0000*/ uint8_t  unknown0002[4];         // ***Placeholder
/*0004*/ uint32_t platinum;               // Platinum Pieces
/*0008*/ uint32_t gold;                   // Gold Pieces
/*0012*/ uint32_t silver;                 // Silver Pieces
/*0016*/ uint32_t copper;                 // Copper Pieces
/*0020*/
};

/*
** Stamina
** Length: 14 Octets
** OpCode: staminaCode
*/

struct staminaStruct 
{
/*0000*/ uint32_t food;                     // (low more hungry 127-0)
/*0004*/ uint32_t water;                    // (low more thirsty 127-0)
/*0008*/ uint32_t fatigue;                  // (high more fatigued 0-100)
/*0012*/
};

/*
** Battle Code
** Length: 34 Octets
** OpCode: ActionCode
*/

// This can be used to gather info on spells cast on us
struct action2Struct
{
/*0000*/ uint16_t target;               // Target ID
/*0002*/ uint16_t source;               // Source ID
/*0004*/ uint8_t  type;                 // Bash, kick, cast, etc.
/*0005*/ int16_t  spell;                // SpellID
/*0007*/ int32_t  damage;
/*0011*/ uint8_t  unknown0011[12];  // ***Placeholder
/*0023*/
};

// This can be used to gather info on spells cast on us
struct actionStruct
{
/*0000*/ uint16_t target;                 // Target ID
/*0002*/ uint16_t source;                 // SourceID
/*0004*/ uint8_t  level;                  // Caster level
/*0005*/ uint8_t  unknown0005[17];        // ***Placeholder
/*0022*/ uint8_t  type;                   // Casts, Falls, Bashes, etc...
/*0012*/ int32_t  damage;                 // Amount of Damage
/*0027*/ int16_t  spell;                  // SpellID
/*0029*/ uint8_t  unknown0029[2];         // ***Placeholder
/*0031*/
};

/*
** New Zone Code
** Length: 590 Octets
** OpCode: NewZoneCode
*/

struct newZoneStruct
{
/*0000*/ char    name[64];                 // Character name
/*0064*/ char    shortName[32];            // Zone Short Name
/*0096*/ char    longName[278];            // Zone Long Name
/*0310*/ uint8_t ztype;                    // Zone type
/*0311*/ uint8_t fog_red[4];               // Zone fog (red)
/*0315*/ uint8_t fog_green[4];             // Zone fog (green)
/*0319*/ uint8_t fog_blue[4];              // Zone fog (blue)
/*0374*/ uint8_t unknown0374[87];          // *** Placeholder
/*0474*/ uint8_t sky;                      // Zone sky
/*0475*/ uint8_t unknown0475[13];          // *** Placeholder
/*0488*/ float   zone_exp_multiplier;      // Experience Multiplier
/*0492*/ float   safe_y;                   // Zone Safe Y
/*0496*/ float   safe_x;                   // Zone Safe X
/*0500*/ float   safe_z;                   // Zone Safe Z
/*0504*/ float   unknown0504;              // *** Placeholder
/*0508*/ float   underworld;               // Underworld
/*0512*/ float   minclip;                  // Minimum view distance
/*0516*/ float   maxclip;                  // Maximum view distance
/*0520*/ uint8_t unknown0520[68];          // *** Placeholder
/*0588*/
};

/*
** Zone Spawns
** Length: 6Octets + Variable Length Spawn Data
** OpCode: ZoneSpawnsCode
*/

struct zoneSpawnsStruct
{
/*0000*/ struct spawnZoneStruct spawn[0];    // Variable number of spawns
};

/*
** client changes target struct
** Length: 4 Octets
** OpCode: clientTargetCode
*/

struct clientTargetStruct
{
/*0000*/ uint32_t newTarget;              // Target ID
/*0004*/ 
};

/*
** Info sent when you start to cast a spell
** Length: 18 Octets
** OpCode: StartCastCode
*/

struct startCastStruct 
{
/*0000*/ int32_t  unknown0002;            // ***Placeholder
/*0004*/ uint32_t spellId;                // Spell ID
/*0008*/ int32_t  unknown0010;            // ***Placeholder
/*0012*/ uint32_t targetId;               // The current selected target
/*0016*/ uint32_t unknown0018;            // ***Placeholder 
/*0020*/
};

/*
** New Mana Amount
** Length: 10 Octets
** OpCode: manaDecrementCode
*/

struct manaDecrementStruct
{
/*0000*/ int32_t newMana;                  // New Mana AMount
/*0004*/ int32_t spellId;                  // Last Spell Cast
/*0008*/
};

/*
** Special Message
** Length: 6 Octets + Variable Text Length
** OpCode: SPMesgCode
*/
struct spMesgStruct
{
/*0000*/ int32_t msgType;                 // Type of message
/*0004*/ char    message[0];              // Message, followed by four Octets?
};

/*
** Spell Fade Struct
** Length: 10 Octets
** OpCode: SpellFadedCode
*/
struct spellFadedStruct
{
/*0000*/ uint32_t color;                  // color of the spell fade message
/*0004*/ char     message[0];             // fade message
/*0???*/ uint8_t  paddingXXX[3];          // always 0's 
};

/*
** Spell Action Struct
** Length: 10 Octets
** OpCode: BeginCastCode
*/
struct beginCastStruct
{
/*0000*/ uint16_t spawnId;                // Id of who is casting
/*0002*/ uint16_t spellId;                // Id of spell
/*0004*/ int16_t  param1;                 // Paramater 1
/*0006*/ int16_t  param2;                 // Paramater 2
/*0008*/
};

/*
** Spell Action Struct
** Length: 14 Octets
** OpCode: MemSpellCode
*/

struct memSpellStruct
{
/*0000*/ uint32_t slotId;                // Id of who is casting
/*0004*/ uint32_t spellId;                // Id of spell
/*0008*/ int16_t  param1;                 // Paramater 1
/*0010*/ int16_t  param2;                 // Paramater 2
};

/*
** Train Skill
** Length: 10 Octets
** OpCode: SkillTrainCode
*/

struct skillTrainStruct
{
/*0000*/ int32_t  playerid;               // player doing the training
/*0004*/ int32_t  type;                   // type of training?
/*0008*/ uint32_t skillId;                // Id of skill
/*0012*/
};

/*
** Skill Increment
** Length: 10 Octets
** OpCode: SkillIncCode
*/

struct skillIncStruct
{
/*0000*/ uint32_t skillId;                // Id of skill
/*0004*/ int32_t  value;                  // New value of skill
/*0008*/
};

/*
** When somebody changes what they're wearing
**      or give a pet a weapon (model changes)
** Length: 18 Octets
** Opcode: WearChangeCode
*/

struct wearChangeStruct
{
/*0000*/ uint32_t spawnId;                // SpawnID
/*0004*/ uint8_t  wearSlotId;             // Slot ID
/*0005*/ uint8_t  unknown0005[3];            // unknown
/*0006*/ uint16_t newItemId;              // Item ID see weaponsX.h or util.cpp
/*0008*/ uint8_t  unknown0008[2];         // unknown
/*0010*/ uint32_t color;                  // color
/*0014*/
};

/*
** Level Update
** Length: 14 Octets
** OpCode: LevelUpUpdateCode
*/

struct levelUpUpdateStruct
{
/*0000*/ uint32_t level;                  // New level
/*0004*/ uint32_t levelOld;               // Old level
/*0008*/ uint32_t exp;                    // Current Experience
/*0012*/
};

/*
** Experience Update
** Length: 6 Octets
** OpCode: ExpUpdateCode
*/

struct expUpdateStruct
{
/*0000*/ uint32_t exp;                    // experience value  x/330
/*0004*/ uint32_t unknown0004;            // ***Place Holder
/*0008*/
};

/*
** Alternate Experience Update
** Length: 14 Octets
** OpCode: AltExpUpdateCode
*/
struct altExpUpdateStruct
{
/*0000*/ uint32_t altexp;                 // alt exp x/330
/*0004*/ uint32_t aapoints;               // current number of AA points
/*0008*/ uint8_t  percent;                // percentage in integer form
/*0009*/ uint8_t  unknown0009[3];            // ***Place Holder
/*0012*/
};

/*
** Type:   Zone Change Request (before hand)
** Length: 70 Octets
** OpCode: ZoneChangeCode
*/

struct zoneChangeStruct
{
/*0000*/ char     name[64];		// Character Name
/*0064*/ uint16_t zoneId;               // zone Id
/*0066*/ uint16_t zoneInstance;         // zone Instance
/*0068*/ uint8_t unknown[8];            // unknown
/*0076*/
};

/*
** Player Spawn Update
** Length: 10 Octets
** OpCode: SpawnUpdateCode
*/

struct SpawnUpdateStruct
{
/*0000*/ uint16_t spawnId;                // Id of spawn to update
/*0002*/ uint16_t subcommand;             // some sort of subcommand type
/*0004*/ int16_t  arg1;                   // first option
/*0006*/ int16_t  arg2;                   // second option
/*0008*/ uint8_t  arg3;                   // third option?
/*0009*/
};

/*
** NPC Hp Update
** Length: 5 Octets
** Opcode NpcHpUpdateCode
*/

struct hpNpcUpdateStruct
{
/*0002*/ uint16_t spawnId;
/*0004*/ uint16_t maxHP;
/*0006*/ uint16_t curHP;
/*0008*/ 
}; 

/*
** Inspecting Information
** Length: 1746 Octets
** OpCode: InspectDataCode
*/

struct inspectDataStruct
{
/*0000*/ uint8_t  unknown0002[72];        // ***Placeholder
/*0068*/ char     itemNames[21][64];      // 21 items with names 
                                          //    64 characters long.
/*1416*/ uint8_t  unknown1416[46];         // ***placeholder
/*1462*/ int16_t  icons[21];              // Icon Information
/*1504*/ char     mytext[200];            // Player Defined Text Info
/*1704*/ uint8_t  unknown0958[88];        // ***Placeholder
/*1792*/
};

/*
** Reading Book Information
** Length: Variable Length Octets
** OpCode: BookTextCode
*/

struct bookTextStruct
{
/*0000*/ uint16_t unknown0000;
/*0002*/ char     text[0];                  // Text of item reading
};

/*
** Interrupt Casting
** Length: 6 Octets + Variable Length Octets
** Opcode: BadCastCode
*/

struct badCastStruct
{
/*0000*/ uint32_t spawnId;                  // Id of who is casting
/*0004*/ char     message[0];               // Text Message
};

/*
** Random Number Request
** Length: 10 Octets
** OpCode: RandomCode
*/
struct randomReqStruct 
{
/*0000*/ uint32_t bottom;                 // Low number
/*0004*/ uint32_t top;                    // High number
};

/*
** Random Number Result
** Length: 78 Octets
** OpCode: RandomCode
*/
struct randomStruct 
{
/*0000*/ uint32_t bottom;                 // Low number
/*0004*/ uint32_t top;                    // High number
/*0008*/ uint32_t result;                 // result number
/*0012*/ char     name[64];               // name rolled by
/*0076*/
};

/*
** Time of Day
** Length: 8 Octets
** OpCode: TimeOfDayCode
*/

struct timeOfDayStruct
{
/*0000*/ uint8_t  hour;                   // Hour (1-24)
/*0001*/ uint8_t  minute;                 // Minute (0-59)
/*0002*/ uint8_t  day;                    // Day (1-28)
/*0003*/ uint8_t  month;                  // Month (1-12)
/*0004*/ uint16_t year;                   // Year
/*0006*/ uint16_t unknown0016;            // Placeholder
/*0008*/
};

/*
** Player Position Update Base
** Length: 8 Octets
** OpCode: PlayerPosCode
*/
struct playerPosStruct
{
/*0000*/ uint16_t spawnId;
/*0002*/ unsigned heading:12;
         signed   deltaHeading:10;
         unsigned animation:10;
/*0006*/
};

/*
** Player Position Update
** Length: 20 Octets
** OpCode: PlayerPosCode
*/

struct playerSpawnPosStruct : public playerPosStruct
{
/*0006*/ signed   deltaX:13;
         signed   x:19;
/*0010*/ signed   y:19;
         signed   deltaZ:13;
/*0014*/ signed   deltaY:13;
         signed   z:19;
/*0018*/
};

/*
** Self Position Update
** Length: 32 Octets
** OpCode: PlayerPosCode
*/

struct playerSelfPosStruct : public playerPosStruct
{
/*0006*/ float    y;
/*0010*/ float    x;
/*0014*/ float    z;
/*0018*/ float    deltaY;
/*0022*/ float    deltaX;
/*0026*/ float    deltaZ;
/*0030*/
};


/*
** Spawn Appearance
** Length: 10 Octets
** OpCode: spawnAppearanceCode
*/

struct spawnAppearanceStruct
{
/*0000*/ uint16_t spawnId;                // ID of the spawn
/*0002*/ uint16_t type;                   // Type of data sent
/*0004*/ uint32_t paramter;               // Values associated with the type
/*0008*/
};

/*
** Combined Door Struct
** Length: 4 + (count * sizeof(doorStruct)) Octets
** OpCode: DoorSpawnCode
*/

struct DoorSpawnsStruct
{
/*0000*/ struct doorStruct doors[0];     // door structures
};
typedef struct DoorSpawnsStruct doorSpawnsStruct; // alias

/*
**               Structures that are not being currently used
 *               (except for logging)
*/

struct bindWoundStruct
{
/*0000*/ uint16_t playerid;             // TargetID
/*0002*/ uint8_t  unknown0002[2];       // ***Placeholder
/*0004*/ uint32_t hpmaybe;              // Hitpoints -- Guess
/*0008*/
};

struct inspectedStruct
{
/*0000*/ uint16_t inspectorid;         // Source ID
/*0002*/ uint8_t  unknown0002[2];      // ***Placeholder
/*0004*/ uint16_t inspectedid;         // Target ID - Should be you
/*0006*/ uint8_t  unknown0006[2];      // ***Placeholder
/*0008*/
};

struct attack1Struct
{
/*0000*/ uint16_t spawnId;                // Spawn ID
/*0002*/ int16_t  param1;                 // ***Placeholder
/*0004*/ int16_t  param2;                 // ***Placeholder
/*0006*/ int16_t  param3;                 // ***Placeholder
/*0008*/ int16_t  param4;                 // ***Placeholder
/*0010*/ int16_t  param5;                 // ***Placeholder
/*0012*/
};

struct attack2Struct
{
/*0000*/ uint16_t spawnId;                // Spawn ID
/*0002*/ int16_t  param1;                 // ***Placeholder
/*0004*/ int16_t  param2;                 // ***Placeholder
/*0006*/ int16_t  param3;                 // ***Placeholder
/*0008*/ int16_t  param4;                 // ***Placeholder
/*0010*/ int16_t  param5;                 // ***Placeholder
/*0012*/
};

struct newGuildInZoneStruct
{
/*0000*/ uint8_t  unknown0000[8];         // ***Placeholder
/*0008*/ char     guildname[56];          // Guildname
/*0064*/
};

struct moneyUpdateStruct
{
/*0000*/ uint16_t unknown0002;            // ***Placeholder
/*0004*/ uint8_t  cointype;               // Coin Type
/*0005*/ uint8_t  unknown0007[3];         // ***Placeholder
/*0008*/ uint32_t amount;                 // Amount
/*0012*/
};
typedef struct moneyUpdateStruct moneyThingStruct; 

/* Memorize slot operations, mem, forget, etc */

struct memorizeSlotStruct
{
/*0000*/ uint32_t slot;                     // Memorization slot (0-7)
/*0004*/ uint32_t spellId;                  // Id of spell 
                                            // (offset of spell in spdat.eff)
/*0008*/ uint32_t action;                   // 1-memming,0-scribing,2-forget
/*0012*/
};

struct cRunToggleStruct
{
/*0000*/ uint32_t status;                   //01=run  00=walk
};

struct cChatFiltersStruct
{
/*0000*/ uint32_t DamageShields;   //00=on  01=off
/*0004*/ uint32_t NPCSpells;       //00=on  01=off
/*0008*/ uint32_t PCSpells;        //00=all 01=off 02=grp
/*0012*/ uint32_t BardSongs;       //00=all 01=me  02=grp 03=off
/*0016*/ uint32_t Unused;
/*0020*/ uint32_t GuildChat;       //00=off 01=on
/*0024*/ uint32_t Socials;         //00=off 01=on
/*0028*/ uint32_t GroupChat;       //00=off 01=on
/*0032*/ uint32_t Shouts;          //00=off 01=on
/*0036*/ uint32_t Auctions;        //00=off 01=on
/*0040*/ uint32_t OOC;             //00=off 01=on
/*0044*/ uint32_t MyMisses;        //00=off 01=on
/*0048*/ uint32_t OthersMisses;    //00=off 01=on
/*0052*/ uint32_t OthersHits;      //00=off 01=on
/*0056*/ uint32_t AttackerMisses;  //00=off 01=on
/*0060*/ uint32_t CriticalSpells;  //00=all 01=me  02=off
/*0064*/ uint32_t CriticalMelee;   //00=all 01=me  02=off
/*0068*/
};

struct cOpenSpellBookStruct
{
/*0000*/ int32_t status; //01=open 00=close
/*0004*/
};

struct tradeSpellBookSlotsStruct
{
/*0000*/ uint32_t slot1;
/*0004*/ uint32_t slot2;
/*0008*/
};


/*
** serverLFGStruct
** Length: 10 Octets
** signifies LFG, maybe afk, role, ld, etc
*/

struct serverLFGStruct
{
/*0000*/ uint16_t spawnID;
/*0002*/ uint16_t unknown0004;
/*0004*/ uint16_t LFG;             //1=LFG
/*0006*/ uint16_t unknown0008;
/*0008*/
};

/*
** clientLFGStruct
** Length: 70 Octets
** signifies LFG, maybe afk, role, ld, etc
*/

struct clientLFGStruct
{
/*0000*/ uint8_t  name[64];
/*0064*/ uint16_t LFG;             //1=LFG
/*0066*/ uint16_t unknown0008;
};

/*
** buffStruct
** Length: 28 Octets
** 
*/

struct buffStruct
{
/*0000*/ uint32_t spawnid;        //spawn id
/*0004*/ uint8_t  unknown0004[4]; 
/*0008*/ uint32_t spellid;        // spellid
/*0012*/ uint32_t duration;       // duration
/*0016*/ uint8_t  unknown0012[4];
/*0020*/ uint32_t spellslot;      // spellslot
/*0024*/ uint32_t changetype;     // 1=buff fading,2=buff duration
/*0028*/
};


/*******************************/
/* World Server Structs        */

/*
** Guild List (from world server)
** Length: 96 Octets
** used in: worldGuildList
*/

struct guildListStruct
{
/*0000*/ char     guildName[64];
};

/*
** Guild List (from world server)
** Length: 49158 Octets
** OpCode: GuildListCode 0x9221
*/
#define MAXGUILDS 512
struct worldGuildListStruct
{
/*000*/ guildListStruct dummy;
/*064*/ guildListStruct guilds[MAXGUILDS];
};

struct worldMOTDStruct
{
  /*002*/ char    message[0];
  /*???*/ uint8_t unknownXXX[3];
};

// Restore structure packing to default
#pragma pack()

#endif // EQSTRUCT_H

//. .7...6....,X....D4.M.\.....P.v..>..W....
//123456789012345678901234567890123456789012
//000000000111111111122222222223333333333444
