/*
** opcodes.h
**
** ShowEQ Distributed under GPL
** http://sourceforge.net/projects/seq/
**/

#ifndef OPCODES_H
#define OPCODES_H

/*
** Please be kind and remember to correctly re-order
** the values in here whenever you add a new item,
** thanks.  - Andon
*/

/* World Server OpCodes */

#define ZoneServerInfo              0x0480

/* Zone Server OpCodes */
#define DropCoinsCode               0x0720

#define ChannelMessageCode          0x0721

#define RemoveCoinsCode             0x0820

#define OpenVendorCode              0x0b20

#define ItemInShopCode              0x0c20

#define SysMsgCode                  0x1420

#define EmoteTextCode               0x1520

#define PlayerAACode                0x1522

#define CorpseLocCode               0x2120

#define AltExpUpdateCode            0x2322 

#define GroupInfoCode               0x2640

#define MoneyThingCode              0x2820

#define ZoneEntryCode               0x2920

#define DeleteSpawnCode             0x2a20

#define RemDropCode                 0x2c20

#define MakeDropCode                0x2d20

#define CharInfo                    0x2e20

#define TradeItemInCode             0x3120

#define CharProfileCode             0x3620

#define FormattedMessageCode        0x3622

#define ConsiderCode                0x3721

#define MoneyUpdateCode             0x3d21

#define CloseGMCode                 0x4321

#define CloseVendorCode             0x4521

#define CastOnCode                  0x4620

#define NewSpawnCode                0x4921

#define NewCorpseCode               0x4a20

#define MoneyOnCorpseCode           0x5020

#define ItemOnCorpseCode            0x5220

#define CharUpdateCode              0x5521

#define StaminaCode                 0x5721

#define ActionCode                  0x5820

#define NewZoneCode                 0x5b20

#define ZoneSpawnsCode              0x6121

#define ClientTargetCode            0x6221

#define PlayerItemCode              0x6421

#define PlayerBookCode              0x6521

#define PlayerContainerCode         0x6621

#define SummonedItemCode            0x7821

#define NewGuildInZoneCode          0x7b21

#define StartCastCode               0x7e21

#define ManaDecrementCode           0x7f21

#define SPMesgCode                  0x8021

#define MemSpellCode                0x8221

#define BindWoundCode               0x8321

#define SkillIncCode                0x8921

#define DoorOpenCode                0x8e20

#define IllusionCode                0x9120

#define WearChangeCode              0x9220

#define DoorSpawnCode               0x9520

#define LevelUpUpdateCode           0x9821

#define ExpUpdateCode               0x9921

#define OpenGMCode                  0x9c20

#define MobUpdateCode               0x9f20

#define Attack2Code                 0xa120

#define ZoneChangeCode              0xa320

#define BeginCastCode               0xa920

#define HPUpdateCode                0xb220

#define InspectDataCode             0xb620

#define BookTextCode                0xce20

#define BadCastCode                 0xd321

#define TradeItemOutCode            0xdf20

#define RandomCode                  0xe721

#define TimeOfDayCode               0xf220

#define PlayerPosCode               0xf320

#define SpawnAppearanceCode         0xf520

#define CPlayerItemsCode            0xf621

#define CDoorSpawnsCode             0xf721

#endif // OPCODES_H
