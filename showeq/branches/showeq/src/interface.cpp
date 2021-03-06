/*
 * interface.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "interface.h"
#include "util.h"
#include "main.h"
#include "editor.h"

#include <qfont.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qvaluelist.h>
#include <qstatusbar.h>
#include <qvaluelist.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qaccel.h>

// this define is used to diagnose the order with which zone packets are rcvd
#define ZONE_ORDER_DIAG

#if ((QT_VERSION >= 210) && !defined (SEQ_OVERRIDE_STYLES))
//#if (QT_VERSION >= 300)
//	#define SEQ_OVERRIDE_STYLES
//#endif /* QT_VERSION */

  #include <qwindowsstyle.h>
  #include <qplatinumstyle.h>
  #include <qmotifstyle.h>
  #include <qcdestyle.h>
  #include <qsgistyle.h>
#endif /* QT_VERSION */

#undef DEBUG

/* Globalized to fix QT 2.0.1 "issues"... */
QPopupMenu* pCharMenu;
QPopupMenu* pCharClassMenu;
QPopupMenu* pCharRaceMenu;
QPopupMenu* pStatWinMenu;
QPopupMenu* pSkillWinMenu;
QPopupMenu* pSpawnListMenu;
QPopupMenu* pDockedWinMenu;
// JohnQ -- I changed the interface widget from a QWidget to 
//          A QMainWindow.. this way we get the menubar for free
//          And we can add toolbars and status bars in the future
//          For free as well.

/* The main interface widget */
EQInterface::EQInterface (QWidget * parent, const char *name):
QMainWindow (parent, name)
{
   int x,y,w,h;
   char tempStr[256];
   QString section = "Interface";
   
   // connect this applications slots to the QApp's signals
   connect(qApp, SIGNAL(aboutToQuit()),
	   this, SLOT(savePrefs()));

   // The first call to menuBar() makes it exist
   menuBar()->setSeparator(QMenuBar::InWindowsStyle);

   // The first call to statusBar() makes it exist
   statusBar()->clear();

   // Create our player object
   m_player = new EQPlayer();

   // Create the filter manager
   m_filterMgr = new FilterMgr();

   // Create the Category manager
   m_categoryMgr = new CategoryMgr();

   // Create the spawn shell
   m_spawnShell = new SpawnShell(*m_filterMgr, m_player);

   // Create the map manager
   m_mapMgr = new MapMgr(m_spawnShell, m_player, this);

   // Create the spell shell
   m_spellShell = new SpellShell(m_player, m_spawnShell);

   // Create the packet object
   m_packet = new EQPacket (m_player, this, "packet");

   // Create the Group Manager
   m_groupMgr = new GroupMgr(m_spawnShell, m_player, "groupmgr");

   // connect interface slots to Packet signals
   connect(m_packet, SIGNAL(clientTarget(const clientTargetStruct*)),
	   this, SLOT(clientTarget(const clientTargetStruct*)));
   connect(m_packet, SIGNAL(attack1Hand1(const attack1Struct *)), 
	   this, SLOT(attack1Hand1(const attack1Struct *)));
   connect(m_packet, SIGNAL(attack2Hand1(const attack2Struct *)), 
	    this, SLOT(attack2Hand1(const attack2Struct *)));
   connect(m_packet, SIGNAL(wearItem(const itemPlayerStruct*)),
	   this, SLOT(wearItem(const itemPlayerStruct*)));
   connect(m_packet, SIGNAL(itemShop(const itemShopStruct*)),
	   this, SLOT(itemShop(const itemShopStruct*)));
   connect(m_packet, SIGNAL(moneyOnCorpse(const moneyOnCorpseStruct*)),
	   this, SLOT(moneyOnCorpse(const moneyOnCorpseStruct*)));
   connect(m_packet, SIGNAL(itemPlayerReceived(const itemReceivedPlayerStruct*)),
	   this, SLOT(itemPlayerReceived(const itemReceivedPlayerStruct*)));
   connect(m_packet, SIGNAL(tradeItemOut(const tradeItemStruct*)),
	   this, SLOT(tradeItemOut(const tradeItemStruct*)));
   connect(m_packet, SIGNAL(tradeItemIn(const itemReceivedPlayerStruct*)),
	   this, SLOT(tradeItemIn(const itemReceivedPlayerStruct*)));
   connect(m_packet, SIGNAL(channelMessage(const channelMessageStruct*, bool)),
	   this, SLOT(channelMessage(const channelMessageStruct*, bool)));
   connect(m_packet, SIGNAL(random(const randomStruct*)),
	   this, SLOT(random(const randomStruct*)));
   connect(m_packet, SIGNAL(emoteText(const emoteTextStruct*)),
	   this, SLOT(emoteText(const emoteTextStruct*)));
   connect(m_packet, SIGNAL(playerBook(const bookPlayerStruct*)),
	   this, SLOT(playerBook(const bookPlayerStruct*)));
   connect(m_packet, SIGNAL(playerContainer(const containerPlayerStruct*)),
	   this, SLOT(playerContainer(const containerPlayerStruct*)));
   connect(m_packet, SIGNAL(inspectData(const inspectingStruct*)),
	   this, SLOT(inspectData(const inspectingStruct*)));
   connect(m_packet, SIGNAL(spMessage(const spMesgStruct*)),
	   this, SLOT(spMessage(const spMesgStruct*)));
   connect(m_packet, SIGNAL(handleSpell(const spellCastStruct*, bool)),
	   this, SLOT(handleSpell(const spellCastStruct*, bool)));
   connect(m_packet, SIGNAL(beginCast(const beginCastStruct*, bool)),
	   this, SLOT(beginCast(const beginCastStruct*)));
   connect(m_packet, SIGNAL(interruptSpellCast(const interruptCastStruct *)),
	   this, SLOT(interruptSpellCast(const interruptCastStruct *)));
   connect(m_packet, SIGNAL(startCast(const castStruct*)),
	   this, SLOT(startCast(const castStruct*)));
   connect(m_packet, SIGNAL(systemMessage(const systemMessageStruct*)),
	   this, SLOT(systemMessage(const systemMessageStruct*)));
   connect(m_packet, SIGNAL(moneyUpdate(const moneyUpdateStruct*)),
	   this, SLOT(moneyUpdate(const moneyUpdateStruct*)));
   connect(m_packet, SIGNAL(moneyThing(const moneyUpdateStruct*)),
	   this, SLOT(moneyThing(const moneyUpdateStruct*)));
   connect(m_packet, SIGNAL(groupInfo(const groupMemberStruct*)),
	   this, SLOT(groupInfo(const groupMemberStruct*)));
   connect(m_packet, SIGNAL(summonedItem(const summonedItemStruct*)),
	   this, SLOT(summonedItem(const summonedItemStruct*)));
   connect(m_packet, SIGNAL(zoneEntry(const ClientZoneEntryStruct*)),
	   this, SLOT(zoneEntry(const ClientZoneEntryStruct*)));
   connect(m_packet, SIGNAL(zoneEntry(const ServerZoneEntryStruct*)),
	   this, SLOT(zoneEntry(const ServerZoneEntryStruct*)));
   connect(m_packet, SIGNAL(zoneChange(const zoneChangeStruct*, bool)),
	   this, SLOT(zoneChange(const zoneChangeStruct*, bool)));
   connect(m_packet, SIGNAL(zoneNew(const newZoneStruct*, bool)),
	   this, SLOT(zoneNew(const newZoneStruct*, bool)));

   // connect FilterMgr slots to Packet signals
   connect(m_packet, SIGNAL(zoneNew(const newZoneStruct*, bool)),
	   m_filterMgr, SLOT(zoneNew(const newZoneStruct*, bool)));

   // connect MapMgr slots to interface signals
   connect(this, SIGNAL(saveAllPrefs(void)),
	   m_mapMgr, SLOT(savePrefs(void)));

   // connect GroupMgr slots to EQPacket signals
   connect(m_packet, SIGNAL(groupInfo(const groupMemberStruct*)),
	   m_groupMgr, SLOT(handleGroupInfo(const groupMemberStruct*)));

   // connect GroupMgr slots to SpawnShell signals
   connect(m_spawnShell, SIGNAL(delItem(const Item*)),
	   m_groupMgr, SLOT(delItem(const Item*)));

   m_stsbarStatus = 0;
   QString statusBarSection = section + "_StatusBar";
   if (pSEQPrefs->getPrefBool("ShowStatus", statusBarSection, 0))
   {
     m_stsbarStatus = new QLabel(statusBar(), "Status");
     m_stsbarStatus->setFrameStyle(QFrame::Panel | QFrame::Sunken);
     m_stsbarStatus->setFont(QFont("Helvetica", showeq_params->statusfontsize));
     m_stsbarStatus->setText("Initializing...");
     m_stsbarStatus->setMinimumHeight(m_stsbarStatus->sizeHint().height());
     m_stsbarStatus->setText("");
     m_stsbarStatus->setMinimumWidth(100);
     statusBar()->addWidget(m_stsbarStatus, 8);
   }

   m_stsbarZone = 0;
   if (pSEQPrefs->getPrefBool("ShowZone", statusBarSection, 0))
   {
     m_stsbarZone = new QLabel(statusBar(), "Zone");
     m_stsbarZone->setFrameStyle(QFrame::Panel | QFrame::Sunken);
     m_stsbarZone->setFont(QFont("Helvetica", showeq_params->statusfontsize));
     m_stsbarZone->setText("Zone: [unknown]");
     m_stsbarZone->setMinimumHeight(m_stsbarZone->sizeHint().height());
     statusBar()->addWidget(m_stsbarZone, 2, TRUE);
   }
   m_stsbarSpawns = 0;
   if (pSEQPrefs->getPrefBool("ShowSpawns", statusBarSection, 0))
   {
     m_stsbarSpawns = new QLabel(statusBar(), "Mobs");
     m_stsbarSpawns->setFrameStyle(QFrame::Panel | QFrame::Sunken);
     m_stsbarSpawns->setFont(QFont("Helvetica", showeq_params->statusfontsize));
     m_stsbarSpawns->setText("Mobs:");
     m_stsbarSpawns->setMinimumHeight(m_stsbarSpawns->sizeHint().height());
     statusBar()->addWidget(m_stsbarSpawns, 1, TRUE);
   }

   m_stsbarExp = 0;
   if (pSEQPrefs->getPrefBool("ShowExp", statusBarSection, 0))
   {
     m_stsbarExp = new QLabel(statusBar(), "Exp");
     m_stsbarExp->setFrameStyle(QFrame::Panel | QFrame::Sunken);
     m_stsbarExp->setFont(QFont("Helvetica", showeq_params->statusfontsize));
     m_stsbarExp->setText("Exp [unknown]");
     m_stsbarExp->setMinimumHeight(m_stsbarExp->sizeHint().height());
     statusBar()->addWidget(m_stsbarExp, 1, TRUE);
   }

   m_stsbarPkt = 0;
   if (pSEQPrefs->getPrefBool("ShowPacketCounter", statusBarSection, 0))
   {
     m_stsbarPkt = new QLabel(statusBar(), "Pkt");
     m_stsbarPkt->setFrameStyle(QFrame::Panel | QFrame::Sunken);
     m_stsbarPkt->setFont(QFont("Helvetica", showeq_params->statusfontsize));
     m_stsbarPkt->setText("Pkt 0");
     m_stsbarPkt->setMinimumHeight(m_stsbarPkt->sizeHint().height());
     statusBar()->addWidget(m_stsbarPkt, 1, TRUE);
     m_lPacketStartTime = 0;
   }

   m_stsbarEQTime = 0;
   if (pSEQPrefs->getPrefBool("ShowEQTime", statusBarSection, 0))
   {
       m_stsbarEQTime = new QLabel(statusBar(), "EQTime");
       m_stsbarEQTime->setFrameStyle(QFrame::Panel | QFrame::Sunken);
       m_stsbarEQTime->setFont(QFont("Helvetica", showeq_params->statusfontsize));
       m_stsbarEQTime->setText("EQTime [UNKNOWN]");
       m_stsbarEQTime->setMinimumHeight(m_stsbarEQTime->sizeHint().height());
       statusBar()->addWidget(m_stsbarEQTime, 1, TRUE);
   }


   // The first call to tooTipGroup() makes it exist
//   toolTipGroup()->addWidget();


   //
   // Main widgets
   //

   // Make a VBox to use as central widget
   QVBox* pCentralBox = new QVBox(this);
   setCentralWidget(pCentralBox);
 
   // Make the horizontal splitter deviding the map from the other objects
   // and add it to the main window
   m_splitH =new QSplitter(QSplitter::Horizontal,pCentralBox,"SplitH");
   m_splitH->setOpaqueResize(TRUE);

   // make a splitter between the spawnlist and other objects
   m_splitV = new QSplitter(QSplitter::Vertical,m_splitH,"SplitV");
   m_splitV->setOpaqueResize(TRUE);

   // Make a horizontal splitter for the skilllist/statlist/compass
   m_splitT = new QSplitter(QSplitter::Horizontal,m_splitV,"SplitT");
   m_splitT->setOpaqueResize(TRUE);

   //
   // Create our compass object
   //

   // Create/display the Map(s)
   for (int i = 0; i < maxNumMaps; i++)
   {
     // first clear the variable
     m_map[i] = NULL;

     QString tmpPrefSuffix = "";
     if (i > 0)
       tmpPrefSuffix = QString::number(i + 1);
     
     // construct the preference name
     QString tmpPrefName = QString("DockedMap") + tmpPrefSuffix;

     // retrieve if the map should be docked
     m_isMapDocked[i] = pSEQPrefs->getPrefBool(tmpPrefName, section, (i == 0));

     // construct the preference name
     tmpPrefName = QString("ShowMap") + tmpPrefSuffix;

     // and as appropriate, craete the map
     if (pSEQPrefs->getPrefBool(tmpPrefName, section, (i == 0)))
       showMap(i);
   }
   
   // should the compass be docked if it's created
   m_isCompassDocked = 
     pSEQPrefs->getPrefBool("DockedCompass", section, 1);

   //
   // Create the compass as required
   //
   m_compass = NULL;
   if (pSEQPrefs->getPrefBool("ShowCompass", section, 1))
     showCompass();

   // should the spell list be docked if it's created
   m_isSpellListDocked = 
     pSEQPrefs->getPrefBool("DockedSpellList", section, 1);

   //
   // Create the spells listview as required
   //
   m_spellList = NULL;
   if (pSEQPrefs->getPrefBool("ShowSpellList", section, 1))
     showSpellList();

   // should the skill list be docked if it's created
   m_isSkillListDocked = 
     pSEQPrefs->getPrefBool("DockedPlayerSkills", section, 1);

   //
   // Create the skills listview as required
   //
   m_skillList = NULL;
   if (pSEQPrefs->getPrefBool("ShowPlayerSkills", section, 1))
     showSkillList();

   // should the player status window be docked if it's created
   m_isStatListDocked = 
     pSEQPrefs->getPrefBool("DockedPlayerStats", section, 1);

   //
   // Create the Player Status listview
   //
   m_statList = NULL;
   if (pSEQPrefs->getPrefBool("ShowPlayerStats", section, 1))
     showStatList();

   // should the spawn list be docked if it's created
   m_isSpawnListDocked = 
     pSEQPrefs->getPrefBool("DockedSpawnList", section, 1);
   
   //
   // Create the Spawn List listview
   //
   m_spawnList = NULL;
   if (pSEQPrefs->getPrefBool("ShowSpawnList", section, 1))
     showSpawnList();

   // connect MapMgr slots to Packet signals
   connect(m_packet, SIGNAL(zoneEntry(const ServerZoneEntryStruct*)),
	   m_mapMgr, SLOT(zoneEntry(const ServerZoneEntryStruct*)));
   connect(m_packet, SIGNAL(zoneChange(const zoneChangeStruct*, bool)),
	   m_mapMgr, SLOT(zoneChange(const zoneChangeStruct*, bool)));
   connect(m_packet, SIGNAL(zoneNew(const newZoneStruct*, bool)),
	   m_mapMgr, SLOT(zoneNew(const newZoneStruct*, bool)));

   // connect the SpawnShell slots to Packet signals
   connect(m_packet, SIGNAL(zoneEntry(const ServerZoneEntryStruct*)),
	   m_spawnShell, SLOT(clear(void)));
   connect(m_packet, SIGNAL(zoneEntry(const ClientZoneEntryStruct*)),
	   m_spawnShell, SLOT(clear(void)));
   connect(m_packet, SIGNAL(zoneChange(const zoneChangeStruct*, bool)),
	   m_spawnShell, SLOT(clear(void)));
   connect(m_packet, SIGNAL(newGroundItem(const dropThingOnGround*, bool)),
	   m_spawnShell, SLOT(newGroundItem(const dropThingOnGround *)));
   connect(m_packet, SIGNAL(removeGroundItem(const removeThingOnGround *)),
	   m_spawnShell, SLOT(removeGroundItem(const removeThingOnGround *)));
   connect(m_packet, SIGNAL(newCoinsItem(const dropCoinsStruct *)),
	   m_spawnShell, SLOT(newCoinsItem(const dropCoinsStruct *)));
   connect(m_packet, SIGNAL(removeCoinsItem(const removeCoinsStruct *)),
	   m_spawnShell, SLOT(removeCoinsItem(const removeCoinsStruct *)));
   connect(m_packet, SIGNAL(newSpawn(const newSpawnStruct*)),
	   m_spawnShell, SLOT(newSpawn(const newSpawnStruct*)));
   connect(m_packet, SIGNAL(updateSpawns(const spawnPositionUpdateStruct *)),
	   m_spawnShell, SLOT(updateSpawns(const spawnPositionUpdateStruct *)));
   connect(m_packet, SIGNAL(updateSpawnHP(const spawnHpUpdateStruct *)),
	   m_spawnShell, SLOT(updateSpawnHP(const spawnHpUpdateStruct *)));
   connect(m_packet, SIGNAL(deleteSpawn(const deleteSpawnStruct*)),
	   m_spawnShell, SLOT(deleteSpawn(const deleteSpawnStruct*)));
   connect(m_packet, SIGNAL(killSpawn(const spawnKilledStruct*)),
	   m_spawnShell, SLOT(killSpawn(const spawnKilledStruct*)));
   connect(m_packet, SIGNAL(backfillSpawn(const spawnStruct *)),
	   m_spawnShell, SLOT(backfillSpawn(const spawnStruct *)));
   connect (m_packet, SIGNAL(backfillPlayer(const playerProfileStruct*)),
	    m_spawnShell, SLOT(backfillPlayer(const playerProfileStruct*)));
   connect(m_packet, SIGNAL(spawnWearingUpdate(const wearChangeStruct*)),
	   m_spawnShell, SLOT(spawnWearingUpdate(const wearChangeStruct*)));
   connect(m_packet, SIGNAL(consRequest(const considerStruct*)),
	   m_spawnShell, SLOT(consRequest(const considerStruct*)));
   connect(m_packet, SIGNAL(consMessage(const considerStruct*)),
	   m_spawnShell, SLOT(consMessage(const considerStruct*)));
   connect(m_packet, SIGNAL(playerUpdate(const playerUpdateStruct*, bool)),
	   m_spawnShell, SLOT(playerUpdate(const playerUpdateStruct*, bool)));
   connect(m_packet, SIGNAL(corpseLoc(const corpseLocStruct*)),
	   m_spawnShell, SLOT(corpseLoc(const corpseLocStruct*)));
   connect(m_packet, SIGNAL(zoneSpawns(const zoneSpawnsStruct*, int)),
	   m_spawnShell, SLOT(zoneSpawns(const zoneSpawnsStruct*, int)));

   // connect EQInterface slots to SpawnShell signals
   connect(m_spawnShell, SIGNAL(spawnConsidered(const Item*)),
	   this, SLOT(spawnConsidered(const Item*)));
   connect(m_spawnShell, SIGNAL(delItem(const Item*)),
	   this, SLOT(delItem(const Item*)));
   connect(m_spawnShell, SIGNAL(killSpawn(const Item*)),
	   this, SLOT(killSpawn(const Item*)));
   connect(m_spawnShell, SIGNAL(changeItem(const Item*, uint32_t)),
	   this, SLOT(changeItem(const Item*)));

   // connect the SpellShell slots to EQPacket signals
   connect(m_packet, SIGNAL(startCast(const castStruct *)),
	   m_spellShell, SLOT(selfStartSpellCast(const castStruct *)));
   connect(m_packet, SIGNAL(handleSpell(const spellCastStruct *, bool)),
	   m_spellShell, SLOT(selfFinishSpellCast(const spellCastStruct *)));
//   connect(m_packet, SIGNAL(beginCast(struct beginCastStruct *)),
//      m_spellShell, SLOT(otherStartSpellCast(struct beginCastStruct *)));
   connect(m_packet, SIGNAL(interruptSpellCast(const interruptCastStruct *)),
	   m_spellShell, SLOT(interruptSpellCast(const interruptCastStruct *)));

   // connect the SpellShell slots to EQInterface signals
   connect(this, SIGNAL(spellMessage(QString&)),
	   m_spellShell, SLOT(spellMessage(QString&)));

   // connect EQPlayer slots to EQPacket signals
   connect(m_packet, SIGNAL(setPlayerName(const QString&)), 
	   m_player, SLOT(setPlayerName(const QString&)));
   connect(m_packet, SIGNAL(setPlayerDeity(uint8_t)), 
	   m_player, SLOT(setPlayerDeity(uint8_t)));
   connect(m_packet, SIGNAL(zoneEntry(const ClientZoneEntryStruct*)),
	   m_player, SLOT(zoneEntry(const ClientZoneEntryStruct*)));
   connect(m_packet, SIGNAL(zoneEntry(const ServerZoneEntryStruct*)),
	   m_player, SLOT(zoneEntry(const ServerZoneEntryStruct*)));
   connect(m_packet, SIGNAL(zoneChange(const zoneChangeStruct*, bool)),
	   m_player, SLOT(zoneChange(const zoneChangeStruct*, bool)));
   connect(m_packet, SIGNAL(zoneNew(const newZoneStruct*, bool)),
	   m_player, SLOT(zoneNew(const newZoneStruct*, bool)));
   connect(m_packet, SIGNAL(updateSpawnHP(const spawnHpUpdateStruct*)),
	   m_player, SLOT(updateSpawnHP(const spawnHpUpdateStruct*)));
   connect(m_packet, SIGNAL(setPlayerID(uint16_t)), 
	   m_player, SLOT(setPlayerID(uint16_t)));
   connect(m_packet, SIGNAL(backfillPlayer(const playerProfileStruct*)),
	   m_player, SLOT(backfill(const playerProfileStruct*)));
   connect(m_packet, SIGNAL(playerUpdate(const playerUpdateStruct*, bool)),
	   m_player, SLOT(playerUpdate(const playerUpdateStruct*, bool)));
   connect(m_packet, SIGNAL(updateLevel(const levelUpStruct*)),
	   m_player, SLOT(updateLevel(const levelUpStruct*)));
   connect(m_packet, SIGNAL(updateLevel(const levelUpStruct*)),
	   m_player, SLOT(updateLevel(const levelUpStruct*)));
   connect(m_packet, SIGNAL(resetPlayer()),
	   m_player, SLOT(reset()));
   connect(m_packet, SIGNAL(wearItem(const itemPlayerStruct*)),
	   m_player, SLOT(wearItem(const itemPlayerStruct*)));
   connect(m_packet, SIGNAL(updateExp(const expUpdateStruct*)),
	   m_player, SLOT(updateExp(const expUpdateStruct*)));
   connect(m_packet, SIGNAL(increaseSkill(const skillIncreaseStruct*)),
	   m_player, SLOT(increaseSkill(const skillIncreaseStruct*)));
   connect(m_packet, SIGNAL(manaChange(const manaDecrementStruct*)),
	   m_player, SLOT(manaChange(const manaDecrementStruct*)));
   connect(m_packet, SIGNAL(updateStamina(const staminaStruct*)),
	   m_player, SLOT(updateStamina(const staminaStruct*)));

   // Initialize the experience window;
   m_expWindow = new ExperienceWindow( m_packet );

   // Make the file menu
   QPopupMenu* pFileMenu = new QPopupMenu;
   pFileMenu->insertItem("&Open Map", m_mapMgr, SLOT(loadMap()), Key_F1);
   pFileMenu->insertItem("&Save Map", m_mapMgr, SLOT(saveMap()), Key_F2);

   pFileMenu->insertItem("&Reload Filters", 
			 m_filterMgr, SLOT(loadFilters()), Key_F3);


   pFileMenu->insertItem("&Save Filters",
                   m_filterMgr, SLOT(saveFilters()), Key_F4);


   pFileMenu->insertItem("Edit Filters", this, SLOT(launch_editor_filters()));
   if (pSEQPrefs->getPrefBool("Filters_UseOldFilters", section, 0))
    pFileMenu->insertItem("Edit Spawns", this, SLOT(launch_editor_spawns()));
   else
    pFileMenu->insertItem("Select Filter File", this, SLOT(select_filter_file()));

   pFileMenu->insertItem("Add Spawn Category",
			 this, SLOT(addCategory()) , ALT+Key_C);
   pFileMenu->insertItem("Rebuild SpawnList",
			 this, SLOT(rebuildSpawnList()) , ALT+Key_R);
   pFileMenu->insertItem("Reload Categories",
			 this, SLOT(reloadCategories()) , CTRL+Key_R);
   pFileMenu->insertItem("Create MessageBox", 
			 this, SLOT(createMessageBox()), Key_F11);
   pFileMenu->insertItem("Select Next", 
			 this, SLOT(selectNext()), CTRL+Key_Right);
   pFileMenu->insertItem("Select Prev", 
			 this, SLOT(selectPrev()), CTRL+Key_Left);
   if ( (showeq_params->playbackpackets)
       || (showeq_params->recordpackets)
      )
   {
     pFileMenu->insertItem("Inc Playback Speed", m_packet, SLOT(incPlayback()), CTRL+Key_X);
     pFileMenu->insertItem("Dec Playback Speed", m_packet, SLOT(decPlayback()), CTRL+Key_Z);
   }

   pFileMenu->insertItem("&Quit", qApp, SLOT(quit()));
   menuBar()->insertItem("&File", pFileMenu);

   QPopupMenu* pDebugMenu = new QPopupMenu;
   pDebugMenu->insertItem("List &Spawns", this, SLOT(listSpawns()), CTRL+Key_S);
   pDebugMenu->insertItem("List &Drops", this, SLOT(listDrops()), CTRL+Key_D);
   pDebugMenu->insertItem("List &Coins", this, SLOT(listCoins()), CTRL+Key_C);
   pDebugMenu->insertItem("List &Map Info", 
			  this, SLOT(listMapInfo()), CTRL+Key_M);
   pDebugMenu->insertItem("Dump &Spawns", this, SLOT(dumpSpawns()), CTRL+Key_S);
   pDebugMenu->insertItem("Dump &Drops", this, SLOT(dumpDrops()), CTRL+Key_D);
   pDebugMenu->insertItem("Dump &Coins", this, SLOT(dumpCoins()), CTRL+Key_C);
   pDebugMenu->insertItem("Dump Map &Info",
			  this, SLOT(dumpMapInfo()), CTRL+Key_I);
   
   pDebugMenu->insertItem("&List Filters", m_filterMgr, SLOT(listFilters()), ALT+Key_I);
    pDebugMenu->insertSeparator(-1);
    if (showeq_params->monitorOpCode_Usage == true)
        x = pDebugMenu->insertItem("Disable &OpCode Monitoring");
    else
        x = pDebugMenu->insertItem("Enable &OpCode Monitoring");
    pDebugMenu->connectItem(x, this, SLOT(ToggleOpCodeMonitoring(int)));
    pDebugMenu->setAccel(CTRL+Key_O, x);
    x = pDebugMenu->insertItem("&Reload Monitored OpCode List");
    pDebugMenu->connectItem(x, this, SLOT(ReloadMonitoredOpCodeList()));
    pDebugMenu->setAccel(CTRL+Key_R, x);
    menuBar()->insertItem("&Debug", pDebugMenu);

   // Make the log menu
   QPopupMenu* pLogMenu = new QPopupMenu;
   m_id_log_AllPackets = pLogMenu->insertItem("All Packets"      , 
                   this, SLOT(toggle_log_AllPackets())  , Key_F5);
   m_id_log_ZoneData   = pLogMenu->insertItem("Zone Data"        , 
                   this, SLOT(toggle_log_ZoneData())    , Key_F6);
   m_id_log_UnknownData= pLogMenu->insertItem("Unknown Zone Data" , 
                   this, SLOT(toggle_log_UnknownData()) , Key_F7);
   pLogMenu->setCheckable(true);
   menuBar()->insertItem("Lo&g", pLogMenu);
   menuBar()->setItemChecked (m_id_log_AllPackets , showeq_params->logAllPackets);
   menuBar()->setItemChecked (m_id_log_ZoneData   , showeq_params->logZonePackets);
   menuBar()->setItemChecked (m_id_log_UnknownData, showeq_params->logUnknownZonePackets);
   m_packet->setLogAllPackets  (showeq_params->logAllPackets);
   m_packet->setLogZoneData    (showeq_params->logZonePackets);
   m_packet->setLogUnknownData (showeq_params->logUnknownZonePackets);

   // Make the view menu
   QPopupMenu* pViewMenu = new QPopupMenu;
   pViewMenu->setCheckable(true);
   m_id_view_ChannelMsgs = pViewMenu->insertItem("Channel Messages" ,
                   this, SLOT(toggle_view_ChannelMsgs()));
   m_id_view_UnknownData = pViewMenu->insertItem("Unknown Data"      ,
                   this, SLOT(toggle_view_UnknownData()) , Key_F8);
   m_id_view_ExpWindow = pViewMenu->insertItem("Experience Window",
                   this, SLOT(toggle_view_ExpWindow()) );
   pViewMenu->insertSeparator(-1);
   m_id_view_SpellList = pViewMenu->insertItem("SpellList",
                   this, SLOT(toggle_view_SpellList()) );
   m_id_view_SpawnList = pViewMenu->insertItem("SpawnList",
                   this, SLOT(toggle_view_SpawnList()) );
   m_id_view_PlayerStats = pViewMenu->insertItem("PlayerStats",
                   this, SLOT(toggle_view_PlayerStats()) );
   m_id_view_PlayerSkills = pViewMenu->insertItem("PlayerSkills",
                   this,SLOT(toggle_view_PlayerSkills()) );
   m_id_view_Compass = pViewMenu->insertItem("Compass",
                   this, SLOT(toggle_view_Compass()) );

   for (int i = 0; i < maxNumMaps; i++)
   {     
     QString mapName = "Map ";
     if (i > 0)
       mapName += QString::number(i + 1);
     m_id_view_Map[i] = pViewMenu->insertItem(mapName,
					      this, 
					      SLOT(toggle_view_Map(int)));
     pViewMenu->setItemParameter(m_id_view_Map[i], i);
     menuBar()->setItemChecked(m_id_view_Map[i], (m_map[i] != NULL));
   }

    if (pStatWinMenu == NULL)
       pStatWinMenu = new QPopupMenu;

    pStatWinMenu->setCheckable(TRUE);

    m_id_view_PlayerStats_Stats[LIST_HP] = pStatWinMenu->insertItem("Hit Points");
    pStatWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_HP], 
				   LIST_HP);
    if (m_statList != NULL)
      pStatWinMenu->setItemChecked(m_id_view_PlayerStats_Stats[LIST_HP], 
				   m_statList->statShown(LIST_HP));
    
    m_id_view_PlayerStats_Stats[LIST_MANA] = pStatWinMenu->insertItem("Mana");
    pStatWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_MANA], 
				   LIST_MANA);
    if (m_statList != NULL)
      pStatWinMenu->setItemChecked(m_id_view_PlayerStats_Stats[LIST_MANA], m_statList->statShown(LIST_MANA));
    
    m_id_view_PlayerStats_Stats[LIST_STAM] = pStatWinMenu->insertItem("Stamina");
    pStatWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_STAM], 
				   LIST_STAM);
    if (m_statList != NULL)
      pStatWinMenu->setItemChecked(m_id_view_PlayerStats_Stats[LIST_STAM], m_statList->statShown(LIST_STAM));
    
    m_id_view_PlayerStats_Stats[LIST_EXP] = pStatWinMenu->insertItem("Experience");
    pStatWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_EXP], 
				   LIST_EXP);
    if (m_statList != NULL)
      pStatWinMenu->setItemChecked(m_id_view_PlayerStats_Stats[LIST_EXP], m_statList->statShown(LIST_EXP));
    
    m_id_view_PlayerStats_Stats[LIST_FOOD] = pStatWinMenu->insertItem("Food");
    pStatWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_FOOD], 
				   LIST_FOOD);
    if (m_statList != NULL)
      pStatWinMenu->setItemChecked(m_id_view_PlayerStats_Stats[LIST_FOOD], m_statList->statShown(LIST_FOOD));
    
    m_id_view_PlayerStats_Stats[LIST_WATR] = pStatWinMenu->insertItem("Water");
    pStatWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_WATR], 
				   LIST_WATR);
    if (m_statList != NULL)
      pStatWinMenu->setItemChecked(m_id_view_PlayerStats_Stats[LIST_WATR], m_statList->statShown(LIST_WATR));
    
    pStatWinMenu->insertSeparator(-1);
    
    m_id_view_PlayerStats_Stats[LIST_STR] = pStatWinMenu->insertItem("Strength");
    pStatWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_STR], 
				   LIST_STR);
    if (m_statList != NULL)
      pStatWinMenu->setItemChecked(m_id_view_PlayerStats_Stats[LIST_STR], m_statList->statShown(LIST_STR));
    
    m_id_view_PlayerStats_Stats[LIST_STA] = pStatWinMenu->insertItem("Stamina");
    pStatWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_STA], 
				   LIST_STA);
    if (m_statList != NULL)
      pStatWinMenu->setItemChecked(m_id_view_PlayerStats_Stats[LIST_STA], m_statList->statShown(LIST_STA));
    
    m_id_view_PlayerStats_Stats[LIST_CHA] = pStatWinMenu->insertItem("Charisma");
    pStatWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_CHA], 
				   LIST_CHA);
    if (m_statList != NULL)
      pStatWinMenu->setItemChecked(m_id_view_PlayerStats_Stats[LIST_CHA], m_statList->statShown(LIST_CHA));
    
    m_id_view_PlayerStats_Stats[LIST_DEX] = pStatWinMenu->insertItem("Dexterity");
    pStatWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_DEX], 
				   LIST_DEX);
    if (m_statList != NULL)
      pStatWinMenu->setItemChecked(m_id_view_PlayerStats_Stats[LIST_DEX], m_statList->statShown(LIST_DEX));
    
    m_id_view_PlayerStats_Stats[LIST_INT] = pStatWinMenu->insertItem("Intelligence");
    pStatWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_INT], 
				   LIST_INT);
    if (m_statList != NULL)
      pStatWinMenu->setItemChecked(m_id_view_PlayerStats_Stats[LIST_INT], m_statList->statShown(LIST_INT));
    
    m_id_view_PlayerStats_Stats[LIST_AGI] = pStatWinMenu->insertItem("Agility");
    pStatWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_AGI], 
				   LIST_AGI);
    if (m_statList != NULL)
      pStatWinMenu->setItemChecked(m_id_view_PlayerStats_Stats[LIST_AGI], m_statList->statShown(LIST_AGI));
    
    m_id_view_PlayerStats_Stats[LIST_WIS] = pStatWinMenu->insertItem("Wisdom");
    pStatWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_WIS], 
				   LIST_WIS);
    if (m_statList != NULL)
      pStatWinMenu->setItemChecked(m_id_view_PlayerStats_Stats[LIST_WIS], m_statList->statShown(LIST_WIS));
    
#if 1 // ZBTEMP: Currently don't know how to get these, so disable them
    pStatWinMenu->insertSeparator(-1);
    
    m_id_view_PlayerStats_Stats[LIST_MR] = pStatWinMenu->insertItem("Magic Res");
    pStatWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_MR], 
				   LIST_MR);
    pStatWinMenu->setItemEnabled(m_id_view_PlayerStats_Stats[LIST_MR], false);
    if (m_statList != NULL)
      pStatWinMenu->setItemChecked(m_id_view_PlayerStats_Stats[LIST_MR], m_statList->statShown(LIST_MR));
    
    m_id_view_PlayerStats_Stats[LIST_FR] = pStatWinMenu->insertItem("Fire Res");
    pStatWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_FR], 
				   LIST_FR);
    pStatWinMenu->setItemEnabled(m_id_view_PlayerStats_Stats[LIST_FR], false);
    if (m_statList != NULL)
      pStatWinMenu->setItemChecked(m_id_view_PlayerStats_Stats[LIST_FR], m_statList->statShown(LIST_FR));
    
    m_id_view_PlayerStats_Stats[LIST_CR] = pStatWinMenu->insertItem("Cold Res");
    pStatWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_CR], 
				   LIST_CR);
    pStatWinMenu->setItemEnabled(m_id_view_PlayerStats_Stats[LIST_CR], false);
    if (m_statList != NULL)
      pStatWinMenu->setItemChecked(m_id_view_PlayerStats_Stats[LIST_CR], m_statList->statShown(LIST_CR));
    
    m_id_view_PlayerStats_Stats[LIST_DR] = pStatWinMenu->insertItem("Disease Res");
    pStatWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_DR], 
				   LIST_DR);
    pStatWinMenu->setItemEnabled(m_id_view_PlayerStats_Stats[LIST_DR], false);
    if (m_statList != NULL)
      pStatWinMenu->setItemChecked(m_id_view_PlayerStats_Stats[LIST_DR], 
				   m_statList->statShown(LIST_DR));
    
    m_id_view_PlayerStats_Stats[LIST_PR] = pStatWinMenu->insertItem("Poison Res");
    pStatWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_PR], 
				   LIST_PR);
    pStatWinMenu->setItemEnabled(m_id_view_PlayerStats_Stats[LIST_PR], false);
    if (m_statList != NULL)
      pStatWinMenu->setItemChecked(m_id_view_PlayerStats_Stats[LIST_PR], 
				   m_statList->statShown(LIST_PR));

    pStatWinMenu->insertSeparator(-1);
    
    m_id_view_PlayerStats_Stats[LIST_AC] = pStatWinMenu->insertItem("Armor Class");
    pStatWinMenu->setItemParameter(m_id_view_PlayerStats_Stats[LIST_AC], 
				   LIST_AC);
    pStatWinMenu->setItemEnabled(m_id_view_PlayerStats_Stats[LIST_AC], false);
    if (m_statList != NULL)
      pStatWinMenu->setItemChecked(m_id_view_PlayerStats_Stats[LIST_AC], 
				 m_statList->statShown(LIST_AC));
#endif

    connect (pStatWinMenu, SIGNAL(activated(int)), this, 
             SLOT(toggle_view_StatWin(int)));

    pViewMenu->insertSeparator(-1);

    m_id_view_PlayerStats_Options = pViewMenu->insertItem( "&Player Stats", 
							  pStatWinMenu);

    if (m_statList == NULL)
      pViewMenu->setItemEnabled(m_id_view_PlayerStats_Options, false);
      

    if (pSkillWinMenu == NULL)
       pSkillWinMenu = new QPopupMenu;

    pSkillWinMenu->setCheckable(TRUE);

    m_id_view_PlayerSkills_Languages = pSkillWinMenu->insertItem("&Langauges");
    pSkillWinMenu->setItemParameter(m_id_view_PlayerSkills_Languages,0);

    if (m_skillList != NULL)
      pSkillWinMenu->setItemChecked(m_id_view_PlayerSkills_Languages, 
				    m_skillList->showLanguages());

    connect (pSkillWinMenu, SIGNAL(activated(int)), this, 
             SLOT(toggle_view_SkillWin(int)));

    m_id_view_PlayerSkills_Options = pViewMenu->insertItem( "Player &Skills", 
							   pSkillWinMenu);

    if (m_skillList == NULL)
      pViewMenu->setItemEnabled(m_id_view_PlayerSkills_Options, false);

    if (pSpawnListMenu == NULL)
       pSpawnListMenu = new QPopupMenu;

    pSpawnListMenu->setCheckable(TRUE);

    m_id_view_SpawnList_Cols[SPAWNCOL_NAME] = 
      pSpawnListMenu->insertItem("&Name");
    pSpawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[SPAWNCOL_NAME], 
				     SPAWNCOL_NAME);

    m_id_view_SpawnList_Cols[SPAWNCOL_LEVEL] 
      = pSpawnListMenu->insertItem("&Level");
    pSpawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[SPAWNCOL_LEVEL], 
				     SPAWNCOL_LEVEL);

    m_id_view_SpawnList_Cols[SPAWNCOL_HP] = 
      pSpawnListMenu->insertItem("&HP");
    pSpawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[SPAWNCOL_HP], 
				     SPAWNCOL_HP);

    m_id_view_SpawnList_Cols[SPAWNCOL_MAXHP] = 
      pSpawnListMenu->insertItem("&Max HP");
    pSpawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[SPAWNCOL_MAXHP], 
				     SPAWNCOL_MAXHP);

    m_id_view_SpawnList_Cols[SPAWNCOL_XPOS] = 
      pSpawnListMenu->insertItem("Coord &1");
    pSpawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[SPAWNCOL_XPOS], 
				     SPAWNCOL_XPOS);

    m_id_view_SpawnList_Cols[SPAWNCOL_YPOS] = 
      pSpawnListMenu->insertItem("Coord &2");
    pSpawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[SPAWNCOL_YPOS], 
				     SPAWNCOL_YPOS);

    m_id_view_SpawnList_Cols[SPAWNCOL_ZPOS] = 
      pSpawnListMenu->insertItem("Coord &3");
    pSpawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[SPAWNCOL_ZPOS], 
				     SPAWNCOL_ZPOS);

    m_id_view_SpawnList_Cols[SPAWNCOL_ID] = 
      pSpawnListMenu->insertItem("I&D");
    pSpawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[SPAWNCOL_ID], 
				     SPAWNCOL_ID);

    m_id_view_SpawnList_Cols[SPAWNCOL_DIST] = 
      pSpawnListMenu->insertItem("&Dist");
    pSpawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[SPAWNCOL_DIST], 
				     SPAWNCOL_DIST);

    m_id_view_SpawnList_Cols[SPAWNCOL_RACE] = 
      pSpawnListMenu->insertItem("&Race");
    pSpawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[SPAWNCOL_RACE], 
				     SPAWNCOL_RACE);

    m_id_view_SpawnList_Cols[SPAWNCOL_CLASS] = 
      pSpawnListMenu->insertItem("&Class");
    pSpawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[SPAWNCOL_CLASS], 
				     SPAWNCOL_CLASS);

    m_id_view_SpawnList_Cols[SPAWNCOL_INFO] = 
      pSpawnListMenu->insertItem("&Info");
    pSpawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[SPAWNCOL_INFO], 
				     SPAWNCOL_INFO);

    m_id_view_SpawnList_Cols[SPAWNCOL_SPAWNTIME] = 
      pSpawnListMenu->insertItem("Spawn &Time");
    pSpawnListMenu->setItemParameter(m_id_view_SpawnList_Cols[SPAWNCOL_SPAWNTIME], 
				     SPAWNCOL_SPAWNTIME);

    connect (pSpawnListMenu, SIGNAL(activated(int)), 
	     this, SLOT(toggle_view_SpawnListCol(int)));

    m_id_view_SpawnList_Options = pViewMenu->insertItem( "Spawn &List", 
							 pSpawnListMenu);

    pViewMenu->setItemEnabled(m_id_view_SpawnList_Options,
			      (m_spawnList != NULL));

    // if the spawn list exists, set the default menu values
    if (m_spawnList != NULL)
    {
      for (int i = 0; i < SPAWNCOL_MAXCOLS; i++)
	pSpawnListMenu->setItemChecked(m_id_view_SpawnList_Cols[i], 
				       m_spawnList->columnWidth(i) != 0);
    }

    if (pDockedWinMenu == NULL)
      pDockedWinMenu = new QPopupMenu;
    
    pDockedWinMenu->setCheckable(true);
    
    x = pDockedWinMenu->insertItem("Spawn &List");
    pDockedWinMenu->setItemParameter(x, 0);
    pDockedWinMenu->setItemChecked(x, m_isSpawnListDocked);
    
    x = pDockedWinMenu->insertItem("&Player Skills");
    pDockedWinMenu->setItemParameter(x, 1);
    pDockedWinMenu->setItemChecked(x, m_isSkillListDocked);
    
    x = pDockedWinMenu->insertItem("&Player Skills");
    pDockedWinMenu->setItemParameter(x, 2);
    pDockedWinMenu->setItemChecked(x, m_isSkillListDocked);
    
    x = pDockedWinMenu->insertItem("Sp&ell List");
    pDockedWinMenu->setItemParameter(x, 3);
    pDockedWinMenu->setItemChecked(x, m_isSpellListDocked);
    
    x = pDockedWinMenu->insertItem("&Compass");
    pDockedWinMenu->setItemParameter(x, 4);
    pDockedWinMenu->setItemChecked(x, m_isCompassDocked);

    // insert Map docking options 
    // NOTE: Always insert Map docking options at the end of the Docked menu
    for (int i = 0; i < maxNumMaps; i++)
    {     
      QString mapName = "Map";
      if (i > 0)
	mapName += QString::number(i + 1);
      x = pDockedWinMenu->insertItem(mapName);
     pDockedWinMenu->setItemParameter(x, i + mapDockBase);
     pDockedWinMenu->setItemChecked(x, m_isMapDocked[i]);
    }
    
    connect (pDockedWinMenu, SIGNAL(activated(int)), this, 
             SLOT(toggle_view_DockedWin(int)));
    
    pViewMenu->insertSeparator(-1);
    pViewMenu->insertItem( "&Docked", pDockedWinMenu);

   menuBar()->insertItem("&View", pViewMenu);
   m_viewChannelMsgs = true;
   viewUnknownData = false;
   viewExpWindow = false;
   menuBar()->setItemChecked (m_id_view_ChannelMsgs, m_viewChannelMsgs);
   menuBar()->setItemChecked (m_id_view_UnknownData, viewUnknownData);
   menuBar()->setItemChecked (m_id_view_ExpWindow, viewExpWindow);

   // only check for non-NULL for the following options, because they 
   // are only non-NULL if they are to be visible, and isVisble() 
   // won't be set until after show() is called on the top level window
   menuBar()->setItemChecked(m_id_view_SpawnList, (m_spawnList != NULL));
   menuBar()->setItemChecked(m_id_view_PlayerStats, (m_statList != NULL));
   menuBar()->setItemChecked(m_id_view_PlayerSkills, (m_skillList != NULL));
   menuBar()->setItemChecked(m_id_view_Compass, (m_compass != NULL));
   menuBar()->setItemChecked(m_id_view_SpellList, (m_spellList != NULL));
   for (int i = 0; i < maxNumMaps; i++)
     menuBar()->setItemChecked(m_id_view_Map[i], (m_map[i] != NULL));
   
   m_packet->setViewUnknownData (viewUnknownData);

   // Make the options menu
   QPopupMenu* pOptMenu = new QPopupMenu;
   m_id_opt_Fast     = pOptMenu->insertItem("Fast Machine?",
                   this, SLOT(toggle_opt_Fast()));
   m_id_opt_ConSelect = pOptMenu->insertItem("Select on Consider?",
                   this, SLOT(toggle_opt_ConSelect()));
   m_id_opt_TarSelect = pOptMenu->insertItem("Select on Target?",
                   this, SLOT(toggle_opt_TarSelect()));
   m_id_opt_KeepSelectedVisible =
                       pOptMenu->insertItem("Keep Selected Visible?"  ,
                   this, SLOT(toggle_opt_KeepSelectedVisible()));
   m_id_opt_SparrMessages = pOptMenu->insertItem("Show Sparrs messages",
                   this, SLOT(toggle_opt_SparrMessages()));
   m_id_opt_LogSpawns = pOptMenu->insertItem("Log Spawns",
                   this, SLOT(toggle_opt_LogSpawns()));
   m_id_opt_ResetMana = pOptMenu->insertItem("Reset Max Mana",
                   this, SLOT(resetMaxMana()));
   m_id_opt_PvPTeams  = pOptMenu->insertItem("PvP Teams",
                   this, SLOT(toggle_opt_PvPTeams()));
   m_id_opt_PvPDeity  = pOptMenu->insertItem("PvP Deity",
                   this, SLOT(toggle_opt_PvPDeity()));
                   
   pOptMenu->setCheckable(TRUE);
   menuBar()->insertItem("&Options", pOptMenu);
   menuBar()->setItemChecked (m_id_opt_Fast, showeq_params->fast_machine);
   menuBar()->setItemChecked (m_id_opt_ConSelect, showeq_params->con_select);
   menuBar()->setItemChecked (m_id_opt_TarSelect,
                   showeq_params->tar_select);
   menuBar()->setItemChecked (m_id_opt_KeepSelectedVisible,
                   showeq_params->keep_selected_visible);
   menuBar()->setItemChecked (m_id_opt_SparrMessages,
                   showeq_params->sparr_messages);
   menuBar()->setItemChecked (m_id_opt_LogSpawns,
                   showeq_params->logSpawns);
   menuBar()->setItemChecked (m_id_opt_PvPTeams,
                   showeq_params->pvp);
   menuBar()->setItemChecked (m_id_opt_PvPDeity,
                   showeq_params->deitypvp);

   // Make the address select menu
   QPopupMenu* pAddrMenu = new QPopupMenu;
   /* This seems to be the default, or at least things still work
      without this, and it wasn't in the qt-2.0.1 docs */
   pAddrMenu->setCheckable(TRUE);
   pAddrMenu->insertItem("Monitor &Next EQ Client Seen", m_packet, SLOT(resetPcapFilter()));
   menuBar()->insertItem("&Network", pAddrMenu);

   
    if (pCharMenu == NULL)    
       pCharMenu = new QPopupMenu;
       
   /* Character data menu for entering class/race/level and potentially
      other stats later on */
    x = pCharMenu->insertItem("Auto Detect Settings");
    pCharMenu->connectItem(x, this, SLOT(toggleAutoDetectCharSettings(int)));
    pCharMenu->setItemChecked(x, showeq_params->AutoDetectCharSettings);

    QPopupMenu *pCharLevelMenu = new QPopupMenu;

#if (QT_VERSION >= 210)
    levelSpinBox = new QSpinBox(1, 60, 1, this, "levelSpinBox");
    levelSpinBox->setValue(showeq_params->defaultLevel);
    levelSpinBox->setWrapping( true );
    levelSpinBox->setButtonSymbols(QSpinBox::PlusMinus);
    levelSpinBox->setPrefix("Level: ");
    pCharLevelMenu->insertItem( levelSpinBox );
    connect(levelSpinBox, SIGNAL(valueChanged(int)), this, SLOT(SetDefaultCharacterLevel(int)));
#else
    pCharLevelMenu->insertItem("&Level", 0);
    connect(pCharLevelMenu, SIGNAL(activated(int)), this, SLOT(SetDefaultCharacterLevel_COMPATABILITY(void)));
#endif

    int savedclass = pSEQPrefs->getPrefInt("Defaults_DefaultClass", section, 1);
    int savedrace = pSEQPrefs->getPrefInt("Defaults_DefaultRace", section, 1);
    
    pCharMenu->insertItem("Choose &Level", pCharLevelMenu);


    if (pCharClassMenu == NULL)    
       pCharClassMenu = new QPopupMenu;
    
    connect (pCharClassMenu, SIGNAL(activated(int)), this, SLOT(SetDefaultCharacterClass(int)));
    x = pCharClassMenu->insertItem("Warrior", 1);
     if (savedclass == 1)
      savedclass = x;

    x = pCharClassMenu->insertItem("Cleric", 2);
     if (savedclass == 2)
      savedclass = x;

    x = pCharClassMenu->insertItem("Paladin", 3);
     if (savedclass == 3)
      savedclass = x;

    x = pCharClassMenu->insertItem("Ranger", 4);
     if (savedclass == 4)
      savedclass = x;

    x = pCharClassMenu->insertItem("Shadow Knight", 5);
     if (savedclass == 5)
      savedclass = x;

    x = pCharClassMenu->insertItem("Druid", 6);
     if (savedclass == 6)
      savedclass = x;

    x = pCharClassMenu->insertItem("Monk", 7);
     if (savedclass == 7)
      savedclass = x;

    x = pCharClassMenu->insertItem("Bard", 8);
     if (savedclass == 8)
      savedclass = x;

    x = pCharClassMenu->insertItem("Rogue", 9);
     if (savedclass == 9)
      savedclass = x;

    x = pCharClassMenu->insertItem("Shaman", 10);
     if (savedclass == 10)
      savedclass = x;

    x = pCharClassMenu->insertItem("Necromancer", 11);
     if (savedclass == 11)
      savedclass = x;

    x = pCharClassMenu->insertItem("Wizard", 12);
     if (savedclass == 12)
      savedclass = x;

    x = pCharClassMenu->insertItem("Magician", 13);
     if (savedclass == 13)
      savedclass = x;

    x = pCharClassMenu->insertItem("Enchanter", 14);
     if (savedclass == 14)
      savedclass = x;

    pCharMenu->insertItem("Choose &Class", pCharClassMenu);
    

     if (pCharRaceMenu == NULL)    
       pCharRaceMenu = new QPopupMenu;
       
    connect (pCharRaceMenu, SIGNAL(activated(int)), this, SLOT(SetDefaultCharacterRace(int)));
    x = pCharRaceMenu->insertItem("Human", 1);
     if (savedrace == 1)
      savedrace = x;

    x = pCharRaceMenu->insertItem("Barbarian", 2);
     if (savedrace == 2)
      savedrace = x;

    x = pCharRaceMenu->insertItem("Erudite", 3);
     if (savedrace == 3)
      savedrace = x;

    x = pCharRaceMenu->insertItem("Wood Elf", 4);
     if (savedrace == 4) savedrace = x;

    x = pCharRaceMenu->insertItem("High Elf", 5);
     if (savedrace == 5)
      savedrace = x;

    x = pCharRaceMenu->insertItem("Dark Elf", 6);
     if (savedrace == 6)
      savedrace = x;

    x = pCharRaceMenu->insertItem("Half Elf", 7);
     if (savedrace == 7)
      savedrace = x;

    x = pCharRaceMenu->insertItem("Dwarf", 8);
     if (savedrace == 8)
      savedrace = x;

    x = pCharRaceMenu->insertItem("Troll", 9);
     if (savedrace == 9)
      savedrace = x;

    x = pCharRaceMenu->insertItem("Ogre", 10);
     if (savedrace == 10)
      savedrace = x;

    x = pCharRaceMenu->insertItem("Halfling", 11);
     if (savedrace == 11)
      savedrace = x;

    x = pCharRaceMenu->insertItem("Gnome", 12);
     if (savedrace == 12)
      savedrace = x;

    x = pCharRaceMenu->insertItem("Iksar", 13);
     if (savedrace == 13)
     savedrace = x;

    pCharMenu->insertItem("Choose &Race", pCharRaceMenu);

   SetDefaultCharacterClass (savedclass);
   SetDefaultCharacterRace  (savedrace);

    menuBar()->insertItem("&Character", pCharMenu);

   QPopupMenu *pInterfaceMenu = new QPopupMenu;
   pInterfaceMenu->setCheckable(FALSE);
#if ((QT_VERSION >= 210) && !defined (SEQ_OVERRIDE_STYLES))
   QPopupMenu *pStyleMenu = new QPopupMenu;
   pStyleMenu->setCheckable(TRUE);

   int savedtheme = pSEQPrefs->getPrefInt("Theme", section, 2);

   x = pStyleMenu->insertItem( "Platinum (Macintosh)");
   pStyleMenu->setItemParameter(x, 1);
   IDList_StyleMenu.append(x);

   if (savedtheme == 1) savedtheme = x;
   x = pStyleMenu->insertItem( "Windows (Default)");
   pStyleMenu->setItemParameter(x, 2);
   IDList_StyleMenu.append(x);
   if (savedtheme == 2) savedtheme = x;

   x = pStyleMenu->insertItem( "CDE");
   pStyleMenu->setItemParameter(x, 3);
   IDList_StyleMenu.append(x);
   if (savedtheme == 3) savedtheme = x;

   x = pStyleMenu->insertItem( "CDE Polished");
   pStyleMenu->setItemParameter(x, 4);
   IDList_StyleMenu.append(x);
   if (savedtheme == 4) savedtheme = x;

   x = pStyleMenu->insertItem( "Motif");
   pStyleMenu->setItemParameter(x, 5);
   IDList_StyleMenu.append(x);
   if (savedtheme == 5) savedtheme = x;

   x = pStyleMenu->insertItem( "SGI");
   pStyleMenu->setItemParameter(x, 6);
   IDList_StyleMenu.append(x);
   if (savedtheme == 6) savedtheme = x;

   connect (pStyleMenu, SIGNAL(activated(int)), this, SLOT(selectTheme(int)));
   pInterfaceMenu->insertItem( "&Style", pStyleMenu);
#endif /* QT_VERSION */

   menuBar()->insertItem( "&Interface" , pInterfaceMenu);

#if ((QT_VERSION >= 210) && !defined (SEQ_OVERRIDE_STYLES))
   selectTheme(savedtheme);
#endif /* QT_VERSION */


   if(showeq_params->net_stats) 
   {
     QVBox *NetInfo = new QVBox(NULL);
     QLCDNumber* pCounter = new QLCDNumber (7, NetInfo, "cached");
     pCounter->setSegmentStyle (QLCDNumber::Flat);
     QLCDNumber* pSeqexp = new QLCDNumber (7, NetInfo, "seqexp");
     pSeqexp->setSegmentStyle (QLCDNumber::Flat);
     pSeqexp->setHexMode ();
     QLCDNumber* pSeqcur = new QLCDNumber (7, NetInfo, "seqcur");
     pSeqcur->setSegmentStyle (QLCDNumber::Flat);
     pSeqcur->setHexMode ();
     connect (m_packet, SIGNAL (cacheSize(int)), 
	      pCounter, SLOT (display (int)));
     connect (m_packet, SIGNAL (seqExpect (int)), 
	      pSeqexp, SLOT (display (int)));
     connect (m_packet, SIGNAL (seqReceive (int)), 
	      pSeqcur, SLOT (display (int)));
     NetInfo->show();
   }

#if 0 // ZBTEMP: These will go to the new group object
   connect (m_packet, SIGNAL(addGroup(char*, int)), 
	    m_map, SLOT(addGroup(char*, int)));
   connect (m_packet, SIGNAL(remGroup(char*, int)), 
	    m_map, SLOT(remGroup(char*, int)));
   connect (m_packet, SIGNAL(clrGroup()), 
	    m_map, SLOT(clrGroup()));
#endif


   // Now hook up all the signals and slots

   // connect EQInterface slots to EQPacket signals
   connect (m_packet, SIGNAL(toggle_log_AllPackets()),
	    this, SLOT(toggle_log_AllPackets()));
   connect (m_packet, SIGNAL(toggle_log_ZoneData()),  
	    this, SLOT(toggle_log_ZoneData()));
   connect (m_packet, SIGNAL(toggle_log_UnknownData()),
	    this, SLOT(toggle_log_UnknownData()));

   if (m_stsbarZone) 
     connect (this, SIGNAL(newZoneName(const QString&)),
	      m_stsbarZone, SLOT(setText(const QString&)));

   if (m_stsbarExp)
     connect (m_player, SIGNAL(expChangedStr(const QString&)),
	      m_stsbarExp, SLOT(setText(const QString&)));
   
   if (m_stsbarEQTime)
     connect(m_packet, SIGNAL(eqTimeChangedStr(const QString&)),
	     m_stsbarEQTime, SLOT(setText(const QString&)));
   
   // connect ExperienceWindow slots to EQPlayer signals
   connect (m_player, SIGNAL(expGained(const QString &, int, long, QString )),
	    m_expWindow, SLOT(addExpRecord(const QString &, int, long,QString )));

   if (m_stsbarStatus)
   {
     connect (m_packet, SIGNAL(stsMessage(const QString &, int)),
             this, SLOT(stsMessage(const QString &, int)));
     connect (m_player, SIGNAL(stsMessage(const QString &, int)),
             this, SLOT(stsMessage(const QString &, int)));
   }

   if (m_stsbarSpawns) 
     connect (m_spawnShell, SIGNAL(numSpawns(int)),this, SLOT(numSpawns(int)));

   if (m_stsbarPkt) 
     connect (m_packet, SIGNAL(numPacket(int)),this, SLOT(numPacket(int)));


   // set initial view options
   if (pSEQPrefs->getPrefBool("ShowExpWindow", section, 0))
      toggle_view_ExpWindow();
   if (pSEQPrefs->getPrefBool("ShowSpellList", section, 1))
      toggle_view_SpellList();

   /* Start the packet capturing */
   m_packet->start (10);

   // Create message boxes defined in config preferences
   char *title = 0;
   int i = 0;
   int j = 0;
   MsgDialog* pMsgDlg;
   QString msgSection;
   for(i = 1; i < 15; i++)
   {
      // attempt to pull a button title from the preferences
     msgSection.sprintf("MessageBox%d", i);
     if (pSEQPrefs->isPreference("Title", msgSection))
     {
       title = strdup(pSEQPrefs->getPrefString("Title", msgSection));
//        pMsgDlg = new MsgDialog(topLevelWidget(), title, m_StringList);
//        pMsgDlg = new MsgDialog(this, title, m_StringList);
        // using the parentWidget makes this messagebox isolated from the
        // main application
        pMsgDlg = new MsgDialog(parentWidget(), title, m_StringList);
        m_msgDialogList.append(pMsgDlg);

        // connect signal for new messages
        connect (this, SIGNAL (newMessage(int)),
           pMsgDlg, SLOT (newMessage(int)));

        // set Additive mode
        pMsgDlg->setAdditive(pSEQPrefs->getPrefBool("Additive", msgSection));

        // set control mode
        pMsgDlg->showControls(!pSEQPrefs->getPrefBool("HideControls", msgSection));

        // set Msg Type mode
        pMsgDlg->showMsgType(pSEQPrefs->getPrefBool("ShowMsgType", msgSection));

        // Configure buttons
        for(j = 1; j < 15; j++)
        {
          bool name = FALSE;
          bool filter = FALSE;

          // attempt to pull button description from the preferences
          sprintf(tempStr, "Button%dName", j);
          name = pSEQPrefs->isPreference(tempStr, msgSection);
          QString buttonname(pSEQPrefs->getPrefString(tempStr, msgSection));
          sprintf(tempStr, "Button%dFilter", j);
          filter = pSEQPrefs->isPreference(tempStr, msgSection);
          QString buttonfilter(pSEQPrefs->getPrefString(tempStr, msgSection));
          sprintf(tempStr, "Button%dColor", j);
          QString buttoncolor(pSEQPrefs->getPrefString(tempStr, msgSection));
          sprintf(tempStr, "Button%dActive", j);

          // if we have a name and filter string
          if (name && filter)
          {
            // Add the button
            pMsgDlg->addButton(buttonname, buttonfilter,
                  buttoncolor, pSEQPrefs->getPrefBool(tempStr, msgSection));
          }
          else
          {
             if (name || filter)
             {
               fprintf(stderr, "Error: Incomplete definition of Button '%s'\n",
                  title);
             }
// allow skipped numbers
//             break; // no more buttons
          }

        } // end for buttons

        // set Geometry
        QSize s = pMsgDlg->size();
        QPoint p = pMsgDlg->pos();
        int x,y,w,h;
        sprintf(tempStr, "WindowX");
        x = pSEQPrefs->getPrefInt(tempStr, msgSection, p.x() );
        sprintf(tempStr, "WindowY");
        y = pSEQPrefs->getPrefInt(tempStr, msgSection, p.y() );
        sprintf(tempStr, "WindowW");
        w = pSEQPrefs->getPrefInt(tempStr, msgSection, s.width() );
        sprintf(tempStr, "WindowH");
        h = pSEQPrefs->getPrefInt(tempStr, msgSection, s.height() );
        pMsgDlg->resize(w,h);
        pMsgDlg->show();
        if (pSEQPrefs->getPrefBool("UseWindowPos", section, 0))
          pMsgDlg->move(x,y);

        free(title);

      } // end if dialog config section found

// allow skipped numbers
//      else
//        break;

   } // for all message boxes defined in pref file

   // connect signals for receiving string messages
   connect (m_packet, SIGNAL (msgReceived(const QString &)),
           this, SLOT (msgReceived(const QString &)));
   connect (m_player, SIGNAL (msgReceived(const QString &)),
           this, SLOT (msgReceived(const QString &)));
   connect (m_spawnShell, SIGNAL (msgReceived(const QString &)),
           this, SLOT (msgReceived(const QString &)));

   //
   // Geometry Configuration
   //
   QSize s;
   QPoint p;


   //  Add Category filters to spanwlist from config
   reloadCategories();

   // interface components

   // Restore splitter sizes
   QValueList<int> list;
   i = 0;
   for(;;) 
   {
      i++;
      sprintf(tempStr, "SplitV_Size%d", i);
      if (pSEQPrefs->isPreference(tempStr, section)) {
         x = pSEQPrefs->getPrefInt(tempStr, section);
         list.append(x);
      } else break;
   }
   m_splitV->setSizes(list);

   list.clear();
   i = 0;
   for(;;) 
   {
      i++;
      sprintf(tempStr, "SplitH_Size%d", i);
      if (pSEQPrefs->isPreference(tempStr, section)) {
         x = pSEQPrefs->getPrefInt(tempStr, section);
         list.append(x);
      } else break;
   }
   m_splitH->setSizes(list);

   list.clear();
   i = 0;
   for(;;) 
   {
      i++;
      sprintf(tempStr, "SplitT_Size%d", i);
      if (pSEQPrefs->isPreference(tempStr, section)) 
      {
         x = pSEQPrefs->getPrefInt(tempStr, section);
         list.append(x);
      } else break;
   }
   m_splitT->setSizes(list);


   // set mainwindow Geometry
   s = size();
   p = pos();
   x = pSEQPrefs->getPrefInt( "WindowX", section, p.x() );
   y = pSEQPrefs->getPrefInt( "WindowY", section, p.y() );
   w = pSEQPrefs->getPrefInt( "WindowW", section, s.width() );
   h = pSEQPrefs->getPrefInt( "WindowH", section, s.height() );

#ifdef DEBUG
printf("Resizing %d/%d\n", w, h);
#endif
   resize(w,h);
   if (pSEQPrefs->getPrefBool("UseWindowPos", section, 0)) 
   {
#ifdef DEBUG
      printf("Moving window to %d/%d\n", x, y);
#endif
      move(x, y);
   }
   show();

   // Set main window title
   // TODO: Add % replacement values and a signal to update, for ip address currently
   // TODO: being monitored.
   sprintf(tempStr, "Title");
   if (pSEQPrefs->isPreference(tempStr, section))
      setCaption(QString(pSEQPrefs->getPrefString(tempStr, section)));
   else
      setCaption(QString("ShowEQ - Main"));

} // end constructor

EQInterface::~EQInterface()
{
  if (m_packet != NULL)
    delete m_packet;
}

void EQInterface::toggle_view_StatWin( int id )
{
   int statnum;

   statnum = pStatWinMenu->itemParameter(id);

   if (pStatWinMenu->isItemChecked(id))
   {
       pStatWinMenu->setItemChecked(id, FALSE);

       if (m_statList != NULL)
	 m_statList->updateStat(statnum, false);
   }
   else
   {
       pStatWinMenu->setItemChecked(id, TRUE);

       if (m_statList != NULL)
	 m_statList->updateStat(statnum, true);
   }
}

void EQInterface::toggle_view_SkillWin( int id )
{
  int skillnum;

  skillnum = pSkillWinMenu->itemParameter(id);

  if (pSkillWinMenu->isItemChecked(id))
  {
    pSkillWinMenu->setItemChecked(id, FALSE);

    if ((id == m_id_view_PlayerSkills_Languages) &&
	(m_skillList != NULL))
      m_skillList->showLanguages(false);
   }
   else
   {
       pSkillWinMenu->setItemChecked(id, TRUE);

    if ((id == m_id_view_PlayerSkills_Languages) &&
	(m_skillList != NULL))
      m_skillList->showLanguages(true);
   }
}

void EQInterface::toggle_view_SpawnListCol( int id )
{
  int colnum;

  colnum = pSpawnListMenu->itemParameter(id);
  
  if (pSpawnListMenu->isItemChecked(id))
  {
    pSpawnListMenu->setItemChecked(id, FALSE);
    
    if (m_spawnList != NULL)
      m_spawnList->setColumnVisible(colnum, false);
  }
  else
  {
    pSpawnListMenu->setItemChecked(id, TRUE);
    
    if (m_spawnList != NULL)
      m_spawnList->setColumnVisible(colnum, true);
   }
}

void EQInterface::toggle_view_DockedWin( int id )
{
  QPoint point;
  QWidget* widget = NULL;
  QWidget* newParent = NULL;
  int winnum;

  // get the window number parameter
  winnum = pDockedWinMenu->itemParameter(id);

  // get the current menu item state
  bool checked = pDockedWinMenu->isItemChecked(id);

  // flip the menu item state
  pDockedWinMenu->setItemChecked(id, !checked);

  // new parent is none, or the vertical T splitter, 
  // unless theres an override in a specific case
  newParent = checked ? NULL : m_splitT;

  switch(winnum)
  {
  case 0: // Spawn List
    // new parent is none, or the vertical splitter
    newParent = checked ? NULL : m_splitV;

    // note the new setting
    m_isSpawnListDocked = !checked;

    // reparent the Spawn List
    widget = m_spawnList;
    break;
  case 1: // Player Stats
    // note the new setting
    m_isStatListDocked = !checked;

    // reparent the Stat List
    widget = m_statList;
    break;
  case 2: // Player Skills
    // note the new setting
    m_isSkillListDocked = !checked;
    
    // reparent the Skill List
    widget = m_skillList;
    break;
  case 3: // Spell List
    // new parent is none, or the vertical splitter
    newParent = checked ? NULL : m_splitV;

    // note the new setting
    m_isSpellListDocked = !checked;
    
    // reparent the Skill List
    widget = m_spellList;
    break;
  case 4: // Compass
    // note the new setting
    m_isCompassDocked = !checked;
    
    // reparent the Skill List
    widget = m_compass;
    break;
  default:
    // use default for maps since the number of them can be changed via a 
    // constant (maxNumMaps)
    if ((winnum >= mapDockBase) && (winnum < (mapDockBase + maxNumMaps)))
    {
      // new parent is none, or the horizontal splitter
      newParent = checked ? NULL : m_splitH;
      
      // note the new setting
      m_isMapDocked[winnum - mapDockBase] = !checked;
      
      // reparent teh appropriate map
      widget = m_map[winnum - mapDockBase];
    }
    else
      { } // /shrug
    break;
    };

  // attempt to undock the window
  if (widget != NULL)
  {
    // fist hide the widget
    widget->hide();

    // then reparent the widget
    widget->reparent(newParent, point, true);

    // then show the widget again
    widget->show();

    // make the widget update it's geometry
    widget->updateGeometry();
  }
}

/* Choose the character's race */
void EQInterface::SetDefaultCharacterRace(int id)
{   
    for (int i = 1; i < 14; i++)
       pCharRaceMenu->setItemChecked( i, false );
       
    pCharRaceMenu->setItemChecked( id, true);

    if (id == 13) // Map Iksar to 128 in config file.
       id = 128;

    showeq_params->defaultRace = id;

    pSEQPrefs->setPrefInt("DefaultRace", "Defaults", id);
}

/* Choose the character's class */
void EQInterface::SetDefaultCharacterClass(int id)
{
    for (int i = 1; i < 15; i++)
       pCharClassMenu->setItemChecked( i, false );
    
    pCharClassMenu->setItemChecked( id, true);

    showeq_params->defaultClass = id;

    pSEQPrefs->setPrefInt("DefaultClass", "Defaults", id);
}


//
// save prefs
//
void
EQInterface::savePrefs(void)
{
   if( isVisible() ) {
     QString section = "Interface";
      char tempStr[256];
      QRect r;

      // send savePrefs signal out
      emit saveAllPrefs();

      // save experience window location
      if(m_expWindow) {
         QPoint p = m_expWindow->pos();

         //Save X position
         if (pSEQPrefs->isPreference("WindowX", "Experience"))
            pSEQPrefs->setPrefInt("WindowX", "Experience", p.x());

         //Save Y position
         if (pSEQPrefs->isPreference("WindowY", "Experience"))
            pSEQPrefs->setPrefInt("WindowY", "Experience", p.y());
      }
		
      // save message dialog geometries
      MsgDialog* diag;
      int i = 0;
      int x,y;

      sprintf(tempStr, "WindowXOffset");
      x = pSEQPrefs->getPrefInt(tempStr, section, 0);
      sprintf(tempStr, "WindowYOffset");
      y = pSEQPrefs->getPrefInt(tempStr, section, 0);

      QString msgSection;
      for (diag=m_msgDialogList.first(); diag != 0; diag=m_msgDialogList.next() )
      {
         // determine the message box number from the config file
         for(i=1; i<15; i++) {
	   msgSection.sprintf("MessageBox%d", i);
	   if (!strcmp(pSEQPrefs->getPrefString("Title", msgSection, ""), diag->name()) )
               break;
         }

         // get geometry from dialog
         r = diag->frameGeometry();
         // enlightenment
         //    r = diag->topLevelWidget()->geometry();  // x,y take into account frame

         if (pSEQPrefs->isPreference("WindowX", msgSection))
            pSEQPrefs->setPrefInt("WindowX", msgSection, r.left() + x);
         if (pSEQPrefs->isPreference("WindowY", msgSection))
            pSEQPrefs->setPrefInt("WindowY", msgSection, r.top() + y);
         if (pSEQPrefs->isPreference("WindowW", msgSection))
            pSEQPrefs->setPrefInt("WindowW", msgSection, diag->width());
         if (pSEQPrefs->isPreference("WindowH", msgSection))
            pSEQPrefs->setPrefInt("WindowH", msgSection, diag->height());
      }

      QPoint wpos = topLevelWidget()->pos();
      QSize wsize = topLevelWidget()->size();
      pSEQPrefs->setPrefInt( "WindowX", section, wpos.x() );
      pSEQPrefs->setPrefInt( "WindowY", section, wpos.y() );
      pSEQPrefs->setPrefInt( "WindowW", section, wsize.width() );
      pSEQPrefs->setPrefInt( "WindowH", section, wsize.height() );

      QValueList<int> list = m_splitV->sizes();
      QValueList<int>::Iterator it = list.begin();
      i = 0;
      while(it != list.end()) {
         i++;
         sprintf(tempStr, "SplitVSize%d", i);
         pSEQPrefs->setPrefInt(tempStr, section, (*it));
         ++it;
      }

      list = m_splitH->sizes();
      it = list.begin();
      i = 0;
      while(it != list.end()) {
         i++;
         sprintf(tempStr, "SplitHSize%d", i);
         pSEQPrefs->setPrefInt(tempStr, section, (*it));
         ++it;
      }

      list = m_splitT->sizes();
      it = list.begin();
      i = 0;
      while(it != list.end()) {
         i++;
         sprintf(tempStr, "SplitT_Size%d", i);
         pSEQPrefs->setPrefInt(tempStr, section, (*it));
         ++it;
      }

      // save visible state of interface objects
      pSEQPrefs->setPrefInt("ShowPlayerSkills", section,
			      ((m_skillList != NULL) &&
			       m_skillList->isVisible()));
      if ((m_skillList != NULL) && (m_skillList->isVisible()))
	pSEQPrefs->setPrefInt("DockedPlayerSkills", section, 
				m_isSkillListDocked);
	
      pSEQPrefs->setPrefInt("ShowPlayerStats", section, 
			      ((m_statList != NULL) &&
			       (m_statList->isVisible())));
      if ((m_statList != NULL) && (m_statList->isVisible()))
	pSEQPrefs->setPrefInt("DockedPlayerSkills", section, 
				m_isStatListDocked);

      pSEQPrefs->setPrefInt("ShowCompass", section, 
			      ((m_compass != NULL) && 
			       m_compass->isVisible()));

      pSEQPrefs->setPrefInt("ShowSpawnList", section, 
			      ((m_spawnList != NULL) &&
			       m_spawnList->isVisible()));
      if ((m_spawnList != NULL) && (m_spawnList->isVisible()))
	pSEQPrefs->setPrefInt("DockedSpawnList", section, 
				m_isSpawnListDocked);

      QString tmpPrefSuffix;
      QString tmpPrefName;
      for (int i = 0; i < maxNumMaps; i++)
      {
	tmpPrefSuffix = "";
	if (i > 0)
	  tmpPrefSuffix = QString::number(i + 1);

	tmpPrefName = QString("DockedMap") + tmpPrefSuffix;
	pSEQPrefs->setPrefInt(tmpPrefName, section, m_isMapDocked[i]);

	tmpPrefName = QString("ShowMap") + tmpPrefSuffix;
	pSEQPrefs->setPrefInt(tmpPrefName, section, 
				((m_map[i] != NULL) &&
				 m_map[i]->isVisible()));
      }

      pSEQPrefs->setPrefInt("ShowSpellList", section, 
			      ((m_spellList != NULL) && 
			       m_spellList->isVisible()));

      // save prefs to file
      pSEQPrefs->save();
   }
} // end savePrefs

/* Capture resize events and reset the geometry */
void
EQInterface::resizeEvent (QResizeEvent *e)
{
}

void
EQInterface::select_filter_file(void)
{
  QString filterFile = QFileDialog::getOpenFileName( QString(LOGDIR),
                                                     QString("ShowEQ Filter Files (*.conf)"),
                                                     0,
                                                     "Select Filter Config..."
                                                   );
  if (!filterFile.isEmpty())
    m_filterMgr->loadFilters(filterFile);
}

void EQInterface::listSpawns (void)
{
#ifdef DEBUG
  debug ("listSpawns()");
#endif /* DEBUG */

  // open the output data stream
  QTextStream out(stdout, IO_WriteOnly);
  
   // dump the coin spawns 
  m_spawnShell->dumpSpawns(tSpawn, out);
}

void EQInterface::listDrops (void)
{
#ifdef DEBUG
  debug ("listDrops()");
#endif /* DEBUG */

  // open the output data stream
  QTextStream out(stdout, IO_WriteOnly);

  // dump the coin spawns 
  m_spawnShell->dumpSpawns(tDrop, out);
}

void EQInterface::listCoins (void)
{
#ifdef DEBUG
  debug ("listCoins()");
#endif /* DEBUG */

  // open the output data stream
  QTextStream out(stdout, IO_WriteOnly);

  // dump the coin spawns 
  m_spawnShell->dumpSpawns(tCoins, out);
}

void EQInterface::listMapInfo(void)
{
#ifdef DEBUG
  debug ("listMapInfo()");
#endif /* DEBUG */

  // open the output data stream
  QTextStream out(stdout, IO_WriteOnly);

  // dump map managers info
  m_mapMgr->dumpInfo(out);

  // iterate over all the maps
  for (int i = 0; i < maxNumMaps; i++)
  {
    // if this map has been instantiated, dump it's info
    if (m_map[i] != NULL)
      m_map[i]->dumpInfo(out);
  }
}

void EQInterface::dumpSpawns (void)
{
#ifdef DEBUG
  debug ("dumpSpawns()");
#endif /* DEBUG */
  
  // open the output data stream
  QFile file(pSEQPrefs->getPrefString("DumpSpawnsFilename", "Interface",
				      LOGDIR "/dumpspawns.txt"));
  file.open(IO_WriteOnly);
  QTextStream out(&file);
  
  // dump the coin spawns 
  m_spawnShell->dumpSpawns(tSpawn, out);
}

void EQInterface::dumpDrops (void)
{
#ifdef DEBUG
  debug ("dumpDrops()");
#endif /* DEBUG */
  
  // open the output data stream
  QFile file(pSEQPrefs->getPrefString("DumpSpawnsFilename", "Interface",
				      LOGDIR "/dumpdrops.txt"));
  file.open(IO_WriteOnly);
  QTextStream out(&file);

  // dump the coin spawns 
  m_spawnShell->dumpSpawns(tDrop, out);
}

void EQInterface::dumpCoins (void)
{
#ifdef DEBUG
  debug ("dumpCoins()");
#endif /* DEBUG */

  // open the output data stream
  QFile file(pSEQPrefs->getPrefString("DumpSpawnsFilename", "Interface",
				      LOGDIR "/dumpcoins.txt"));
  file.open(IO_WriteOnly);
  QTextStream out(&file);

  // dump the coin spawns 
  m_spawnShell->dumpSpawns(tCoins, out);
}

void EQInterface::dumpMapInfo(void)
{
#ifdef DEBUG
  debug ("dumpMapInfo()");
#endif /* DEBUG */

  // open the output data stream
  QFile file(pSEQPrefs->getPrefString("DumpMapInfoFilename", "Interface",
				      LOGDIR "/mapinfo.txt"));
  file.open(IO_WriteOnly);
  QTextStream out(&file);

  // dump map managers info
  m_mapMgr->dumpInfo(out);

  // iterate over all the maps
  for (int i = 0; i < maxNumMaps; i++)
  {
    // if this map has been instantiated, dump it's info
    if (m_map[i] != NULL)
      m_map[i]->dumpInfo(out);
  }
}

void
EQInterface::launch_editor_filters(void)
{
  EditorWindow * ew = new EditorWindow(m_filterMgr->filterFile());
  ew->setCaption(m_filterMgr->filterFile());
  ew->show();
}

void
EQInterface::launch_editor_spawns(void)
{
 EditorWindow * ew = new EditorWindow(showeq_params->spawnfilter_spawnfile);
 ew->setCaption(showeq_params->spawnfilter_spawnfile);
 ew->show();
}

void
EQInterface::toggle_opt_ConSelect (void)
{
  showeq_params->con_select = !(showeq_params->con_select);
  menuBar()->setItemChecked (m_id_opt_ConSelect, showeq_params->con_select);
}

void
EQInterface::toggle_opt_TarSelect (void)
{
  showeq_params->tar_select = !(showeq_params->tar_select);
  menuBar()->setItemChecked (m_id_opt_TarSelect, showeq_params->tar_select);
}

void
EQInterface::toggle_opt_Fast (void)
{
  showeq_params->fast_machine = !(showeq_params->fast_machine);
  menuBar()->setItemChecked (m_id_opt_Fast, showeq_params->fast_machine);
}

void
EQInterface::toggle_opt_KeepSelectedVisible (void)
{
  showeq_params->keep_selected_visible = !(showeq_params->keep_selected_visible);
  menuBar()->setItemChecked (m_id_opt_KeepSelectedVisible, showeq_params->keep_selected_visible);
}

/* Check and uncheck Log menu options & set EQPacket logging flags */
void
EQInterface::toggle_log_AllPackets (void)
{
    showeq_params->logAllPackets = !showeq_params->logAllPackets;
    menuBar()->setItemChecked (m_id_log_AllPackets, showeq_params->logAllPackets);
    // The below line seems to be superfluous.  Check and delete later.
    m_packet->setLogAllPackets (showeq_params->logAllPackets);
}

void
EQInterface::toggle_log_ZoneData (void)
{
    showeq_params->logZonePackets = !showeq_params->logZonePackets;
    menuBar()->setItemChecked (m_id_log_ZoneData, showeq_params->logZonePackets);
    // The below line seems to be superfluous.  Check and delete later.
    m_packet->setLogZoneData (showeq_params->logZonePackets);
}

void
EQInterface::toggle_log_UnknownData (void)
{
    showeq_params->logUnknownZonePackets = !showeq_params->logUnknownZonePackets;
    menuBar()->setItemChecked (m_id_log_UnknownData, showeq_params->logUnknownZonePackets);
    // The below line seems to be superfluous.  Check and delete later.
    m_packet->setLogUnknownData (showeq_params->logUnknownZonePackets);
}

/* Check and uncheck View menu options */
void
EQInterface::toggle_view_ChannelMsgs (void)
{
    m_viewChannelMsgs = !m_viewChannelMsgs;
    menuBar()->setItemChecked (m_id_view_ChannelMsgs, m_viewChannelMsgs);
    /* From Daisy, hide Channel Messages if the view flag is false */
    for(MsgDialog *diag=m_msgDialogList.first(); diag != 0; diag=m_msgDialogList.next() ) {
      if (m_viewChannelMsgs) {
	diag->show();
      }
      else {
	diag->hide();
      }
    }
}

void
EQInterface::toggle_view_UnknownData (void)
{
    viewUnknownData = !viewUnknownData;
    menuBar()->setItemChecked (m_id_view_UnknownData, viewUnknownData);
    m_packet->setViewUnknownData (viewUnknownData);
}

void
EQInterface::toggle_view_ExpWindow (void)
{
    viewExpWindow = !viewExpWindow;
    menuBar()->setItemChecked (m_id_view_ExpWindow, viewExpWindow);
    if (viewExpWindow)
    {
       m_expWindow->show();
       // set exp window location
       if(m_expWindow){
	   int x, y;
	   QPoint p = m_expWindow->pos();
	
	   // get X position
	   x = pSEQPrefs->getPrefInt("WindowX", "Experience",
				     p.x());
	   // get Y position
	   y = pSEQPrefs->getPrefInt("WindowY", "Experience",
				     p.y());
	   // move window to new position
	   if(pSEQPrefs->getPrefBool("UseWindowPos", "Interface", 0))
	       m_expWindow->move(x, y);
       }
    }
    else
       m_expWindow->hide();

}

void
EQInterface::toggle_view_SpawnList(void)
{
  bool wasVisible = ((m_spawnList != NULL) && (m_spawnList->isVisible()));

  menuBar()->setItemChecked (m_id_view_SpawnList, !wasVisible);

  if (!wasVisible)
  {
    showSpawnList();

    for (int i = 0; i < SPAWNCOL_MAXCOLS; i++)
      pSpawnListMenu->setItemChecked(m_id_view_SpawnList_Cols[i], 
				     m_spawnList->columnWidth(i) != 0);

    // enable it's options sub-menu
    menuBar()->setItemEnabled(m_id_view_SpawnList_Options, true);
  }
  else 
  {
    // save it's preferences
    m_spawnList->savePrefs();

    // hide it
    m_spawnList->hide();

    // disable it's options sub-menu
    menuBar()->setItemEnabled(m_id_view_SpawnList_Options, false);

    // then delete it
    delete m_spawnList;

    // make sure to clear it's variable
    m_spawnList = NULL;
  }
}

void
EQInterface::toggle_view_SpellList(void)
{
  bool wasVisible = ((m_spellList != NULL) && (m_spellList->isVisible()));
  
  menuBar()->setItemChecked (m_id_view_SpellList, !wasVisible);

  if (!wasVisible)
  {
    showSpellList();

    // only do this move stuff iff the spell list isn't docked
    // and the user set the option to do so.
    if (!m_isSpellListDocked &&
	pSEQPrefs->getPrefBool("UseWindowPos", "Interface", 0)) 
    {
      // Set window location
      int x, y, w, h;
      QPoint p = m_spellList->pos();
      QSize s = m_spellList->size();
      x = pSEQPrefs->getPrefInt("WindowX", "SpellList", p.x());
      y = pSEQPrefs->getPrefInt("WindowY", "SpellList", p.y());
      w = pSEQPrefs->getPrefInt("WindowW", "SpellList", s.width());
      h = pSEQPrefs->getPrefInt("WindowH", "SpellList", s.height());

      // Move window to new position
      m_spellList->resize(w, h);
      m_spellList->move(x, y);
    }
  }
  else
  {
    // save it's preferences
    m_spellList->savePrefs();

    // hide it
    m_spellList->hide();

    // then delete it
    delete m_spellList;

    // make sure to clear it's variable
    m_spellList = NULL;
  }
}

void
EQInterface::toggle_view_PlayerStats(void)
{
  bool wasVisible = ((m_statList != NULL) && (m_statList->isVisible()));

  menuBar()->setItemChecked (m_id_view_PlayerStats, !wasVisible);

  if (!wasVisible)
  {
    showStatList();

    for (int i = 0; i < LIST_MAXLIST; i++)
      pStatWinMenu->setItemChecked(m_id_view_PlayerStats_Stats[i], 
				   m_statList->statShown(i));

    // enable it's options sub-menu
    menuBar()->setItemEnabled(m_id_view_PlayerStats_Options, true);
  }
  else 
  {
    // save it's preferences
    m_statList->savePrefs();

    // hide it
    m_statList->hide();

    // disable it's options sub-menu
    menuBar()->setItemEnabled(m_id_view_PlayerStats_Options, false);

    // then delete it
    delete m_statList;

    // make sure to clear it's variable
    m_statList = NULL;
  }
}

void
EQInterface::toggle_view_PlayerSkills(void)
{
  bool wasVisible = ((m_skillList != NULL) && (m_skillList->isVisible()));

  menuBar()->setItemChecked (m_id_view_PlayerSkills,!wasVisible);

  if (!wasVisible)
  {
    showSkillList();

    menuBar()->setItemChecked(m_id_view_PlayerSkills_Languages, 
			      m_skillList->showLanguages());

    menuBar()->setItemEnabled(m_id_view_PlayerSkills_Options, true);
  }
  else
  {
    // save any preference changes
    m_skillList->savePrefs();

    // if it's not visible, hide it
    m_skillList->hide();

    menuBar()->setItemEnabled(m_id_view_PlayerSkills_Options, false);

    // then delete it
    delete m_skillList;

    // make sure to clear it's variable
    m_skillList = NULL;
  }
}

void
EQInterface::toggle_view_Compass(void)
{
  bool wasVisible = ((m_compass != NULL) && (m_compass->isVisible()));

  menuBar()->setItemChecked (m_id_view_Compass, !wasVisible);

  if (!wasVisible)
    showCompass();
  else
  {
    // if it's not visible, hide it
    m_compass->hide();

    // then delete it
    delete m_compass;

    // make sure to clear it's variable
    m_compass = NULL;
  }
}


void EQInterface::toggle_view_Map(int id)
{
  int mapNum = menuBar()->itemParameter(id);

  bool wasVisible = ((m_map[mapNum] != NULL) && 
		     (m_map[mapNum]->isVisible()));

  menuBar()->setItemChecked(m_id_view_Map[mapNum], !wasVisible);
  
  if (!wasVisible)
    showMap(mapNum);
  else
  {
    // save any preference changes
    m_map[mapNum]->savePrefs();

    // hide it 
    m_map[mapNum]->hide();

    // then delete it
    delete m_map[mapNum];

    // make sure to clear it's variable
    m_map[mapNum] = NULL;
  }
}

void
EQInterface::ToggleOpCodeMonitoring (int id)
{
  if(!showeq_params->monitorOpCode_Usage)
  {
    if (!((QString)showeq_params->monitorOpCode_List).isEmpty())
    {
        if (m_packet->m_bOpCodeMonitorInitialized == false)
          m_packet->InitializeOpCodeMonitor();

        pSEQPrefs->setPrefInt ("Enable", "OpCodeMonitoring", 1, true );
        showeq_params->monitorOpCode_Usage = true;

        printf("OpCode monitoring is now ENABLED...\nUsing list:\t%s\n", showeq_params->monitorOpCode_List);

        menuBar()->changeItem( id, "Disable &OpCode Monitoring" );
    }

    else
      QMessageBox::critical(this, "ERROR", "Unable to enable OpCode monitoring!!!\n\n"
                                               "It would appear as if you have not yet set the\n"
                                               "List value under the [OpCodeMonitoring] section\n"
                                               "of your ShowEQ.conf file...\n\n"
                                               "Please reffer to ShowEQ's instructions for more details...\0"
                               );
  }

  else
  {
    pSEQPrefs->setPrefInt ("Enable", "OpCodeMonitoring", 0, true );
    showeq_params->monitorOpCode_Usage = false;

    printf("OpCode monitoring has been DISABLED...\n");

    menuBar()->changeItem( id, "Enable &OpCode Monitoring" );
  }
}

void EQInterface::ReloadMonitoredOpCodeList (void)
{
  pSEQPrefs->revert();
  showeq_params->monitorOpCode_List = strdup(pSEQPrefs->getPrefString ("List", "OpCodeMonitoring", ""));
  m_packet->InitializeOpCodeMonitor();
  printf("The monitored OpCode list has been reloaded...\nUsing list:\t%s\n", showeq_params->monitorOpCode_List);
}

void
EQInterface::toggle_opt_SparrMessages (void)
{
    showeq_params->sparr_messages = !(showeq_params->sparr_messages);
    menuBar()->setItemChecked (m_id_opt_SparrMessages, showeq_params->sparr_messages);
}

void EQInterface::resetMaxMana(void)
{
  if (m_statList != NULL)
    m_statList->resetMaxMana();
}

void
EQInterface::toggle_opt_LogSpawns (void)
{
    showeq_params->logSpawns = !(showeq_params->logSpawns);
    menuBar()->setItemChecked (m_id_opt_LogSpawns, showeq_params->logSpawns);
}

void
EQInterface::toggle_opt_PvPTeams (void)
{
    showeq_params->pvp = !(showeq_params->pvp);
    menuBar()->setItemChecked (m_id_opt_PvPTeams, showeq_params->pvp);
}

void
EQInterface::toggle_opt_PvPDeity (void)
{
    showeq_params->deitypvp = !(showeq_params->deitypvp);
    menuBar()->setItemChecked (m_id_opt_PvPDeity, showeq_params->deitypvp);
}

void
EQInterface::createMessageBox(void)
{
  MsgDialog* pMsgDlg = new MsgDialog(parentWidget(), 
                                "MessageBox", m_StringList);
  m_msgDialogList.append(pMsgDlg);

  // connect signal for new messages
  connect (this, SIGNAL (newMessage(int)), pMsgDlg, SLOT (newMessage(int)));

  pMsgDlg->show();
}


void
EQInterface::msgReceived(const QString &instring)
{
  //
  // getting reports of a lot of blank lines being spammed about....
  // thinking its CR's in the msgs themselves.  Putting a CR/LF stripper here
  // WARNING:  If you send a msg with a CR or LF in the middle of it, this will
  // replace it with a space.  So far, I don't think we do that anywhere.
  //  - Maerlyn 3/28
  QString string(instring);
  int index = 0;

  if (string.left(28) != "System: Players in EverQuest") {
  while( -1 != (index = string.find('\n')) )
    string.replace(index, 1, " ");
  //  string.remove(index);
  while( -1 != (index = string.find('\r')) )
    string.replace(index, 1, " ");
  //  string.remove(index);
   }

  if (pSEQPrefs->getPrefBool("UseStdout", "Interface")) 
  {
    	if (!strncmp(string.ascii(), "Guild", 5)) fprintf(stdout, "\e[1;32m"); // Light Green
	if (!strncmp(string.ascii(), "Group", 5)) fprintf(stdout, "\e[0;36m"); // Cyan
	if (!strncmp(string.ascii(), "Shout", 5)) fprintf(stdout, "\e[1;31m"); // Light Red
	if (!strncmp(string.ascii(), "Aucti", 5)) fprintf(stdout, "\e[1;42m"); // Light White on Green Background
	if (!strncmp(string.ascii(), "OOC",   3)) fprintf(stdout, "\e[0;32m"); // Green
	if (!strncmp(string.ascii(), "Tell",  4)) fprintf(stdout, "\e[0;35m"); // Magenta
	if (!strncmp(string.ascii(), "Say",   3)) fprintf(stdout, "\e[1;37m"); // White
	if (!strncmp(string.ascii(), "GM-Te", 5)) fprintf(stdout, "\e[5;31m"); // Blinking Red

	fprintf(stdout, "%s", string.ascii());
	fprintf(stdout, "\e[0;0m\n"); // Shut off all ANSI
    	fflush(stdout);
  }

  m_StringList.append(string);

  emit 
    newMessage(m_StringList.count() - 1);
}


//
// TODO:  clear after timeout miliseconds
//
void
EQInterface::stsMessage(const QString &string, int timeout)
{
  if (m_stsbarStatus)
    m_stsbarStatus->setText(string);
}

void
EQInterface::numSpawns(int num)
{
  // only update once per sec
  static int lastupdate = 0;
  if ( (mTime() - lastupdate) < 1000)
    return;
  lastupdate = mTime();

   QString tempStr;
   tempStr.sprintf("Mobs: %d", num);
   m_stsbarSpawns->setText(tempStr);
}

void
EQInterface::numPacket(int num)
{
  static int initialcount = 0;

  // if passed 0 reset the average
  if (num == 0)
  {
    m_lPacketStartTime = mTime();
    initialcount = num;
    return;
  }

  // start the timer of not started
  if (!m_lPacketStartTime)
    m_lPacketStartTime = mTime();

  // only update once per sec
  static int lastupdate = 0;
  if ( (mTime() - lastupdate) < 1000)
    return;
  lastupdate = mTime();
  

   QString tempStr;
   int delta = mTime() - m_lPacketStartTime;
   num -=initialcount;
   if (num && delta)
     tempStr.sprintf("Pkt: %d (%2.1f)", num, (float) (num<<10) / (float) delta);
   else   
     tempStr.sprintf("Pkt: %d", num);

   m_stsbarPkt->setText(tempStr);
}
void EQInterface::attack1Hand1(const attack1Struct * atk1)
{
#if 0
printf("Attack1: '%s' hit by %d damage (%d:%d:%d:%d:%d:%d)\n", 
  spawns[ID2Index(atk1->spawnId)].name, atk1->unknown2, 
  atk1->unknown1,
  atk1->unknown2,
  atk1->unknown3,
  atk1->unknown4,
  atk1->unknown5
 );
#endif
  if (showeq_params->sparr_messages)
  {
    {
        QString temp("");
        temp.sprintf("ATTACK1: %i, u1:%i, u2:%i, u3:%i, u4:%i, u5:%i",
           atk1->spawnId,
           atk1->unknown1,
           atk1->unknown2,
           atk1->unknown3,
           atk1->unknown4,
           atk1->unknown5);
        emit
           msgReceived(temp);
    }
  }
}

void EQInterface::attack2Hand1(const attack2Struct * atk2)
{
#if 0
printf("Attack2: '%s' hit by %d damage (%d:%d:%d:%d:%d:%d)\n", 
  spawns[ID2Index(atk2->spawnId)].name, atk2->unknown2, 
  atk2->unknown1,
  atk2->unknown2,
  atk2->unknown3,
  atk2->unknown4,
  atk2->unknown5
 );
#endif
  if (showeq_params->sparr_messages)
  {
    {
        QString temp("");
        temp.sprintf("ATTACK2: %i, u1:%i, u2:%i, u3:%i, u4:%i, u5:%i",
           atk2->spawnId,
           atk2->unknown1,
           atk2->unknown2,
           atk2->unknown3,
           atk2->unknown4,
           atk2->unknown5);
        emit
           msgReceived(temp);
    }
  }
}

void EQInterface::itemShop(const itemShopStruct* items)
{
  QString tempStr;

  // Add Item to Database
  if (pItemDB != NULL) 
    pItemDB->AddItem(&items->item);
  
  tempStr = QString("Item Shop: ") + items->item.lore + "(" 
    + QString::number(items->item.itemNr) + "), Value: "
    + reformatMoney(items->item.cost);
  
  emit msgReceived(tempStr);
  
  if (items->itemType == 1)
  {
    tempStr = QString("Item Shop: Container: Slots: ") 
      + QString::number(items->item.common.container.numSlots)
      + ", Size Capacity: " 
      + size_name(items->item.common.container.sizeCapacity)
      + ", Weight Reduction: "
      + QString::number(items->item.common.container.weightReduction)
      + "%";
    
    emit msgReceived(tempStr);
  }
}

void EQInterface::moneyOnCorpse(const moneyOnCorpseStruct* money)
{
  QString tempStr;

  if( money->platinum || money->gold || money->silver || money->copper )
  {
    bool bneedComma = false;
    
    tempStr = "Money: You receive ";
    
    if(money->platinum)
    {
      tempStr += QString::number(money->platinum) + " platinum";
      bneedComma = true;
    }
    
    if(money->gold)
    {
      if(bneedComma)
	tempStr += ", ";
      
      tempStr += QString::number(money->gold) + " gold";
      bneedComma = true;
    }
    
    if(money->silver)
    {
      if(bneedComma)
	tempStr += ", ";
      
      tempStr += QString::number(money->silver) + " silver";
      bneedComma = true;
    }
    
    if(money->copper)
      {
	if(bneedComma)
	  tempStr += ", ";
	
	tempStr += QString::number(money->copper) + " copper";
      }
    
    tempStr += " from the corpse";
    
    emit msgReceived(tempStr);
  }
}

void EQInterface::itemPlayerReceived(const itemReceivedPlayerStruct* itemc)
{
  QString tempStr;

  // Add Item to Database
  if (pItemDB != NULL) 
    pItemDB->AddItem(&itemc->item);
  
  tempStr = QString("Item Looted: ") + itemc->item.lore
    + "(" + QString::number(itemc->item.itemNr)
    + "), Value: " + reformatMoney(itemc->item.cost);
  
  emit msgReceived(tempStr);
}

void EQInterface::tradeItemOut(const tradeItemStruct* itemt)
{
  QString tempStr;

  // Add Item to Database
  if (pItemDB != NULL) 
    pItemDB->AddItem(&itemt->item);
  
  tempStr = QString("Item Trade [OUT]: ") + itemt->item.lore
    + "(" + QString::number(itemt->item.itemNr)
    + "), Value: "
    + reformatMoney(itemt->item.cost);
  
  emit msgReceived(tempStr);
}

void EQInterface::tradeItemIn(const itemReceivedPlayerStruct* itemr)
{
  QString tempStr;

  // Add Item to Database
  if (pItemDB != NULL) 
    pItemDB->AddItem(&itemr->item);
  
  tempStr = QString("Item Trade [IN]: ")
    + itemr->item.lore
    + "(" + QString::number(itemr->item.itemNr)
    + "), Value: "
    + reformatMoney(itemr->item.cost);
  
  emit msgReceived(tempStr);
}

void EQInterface::wearItem(const itemPlayerStruct* itemp)
{
  QString tempStr;

  // Add Item to Database
  if (pItemDB != NULL) 
    pItemDB->AddItem(&itemp->item);
  
  if (!showeq_params->no_bank)
  {
    tempStr = QString("Item: ") + itemp->item.lore
      + "(" + QString::number(itemp->item.itemNr)
      + "), Slot: " + QString::number(itemp->item.equipSlot)
      + ", Value: " + reformatMoney(itemp->item.cost);
    
    emit msgReceived(tempStr);
  }
}

void EQInterface::summonedItem(const summonedItemStruct* itemsum)
{
  QString tempStr;

  // Add Item to Database
  if (pItemDB != NULL) 
    pItemDB->AddItem(&itemsum->item);
  
  tempStr = QString("ITEMSUM: ") + itemsum->item.lore
    + "(" + QString::number(itemsum->item.itemNr)
    + "), Value: " + reformatMoney(itemsum->item.cost);
  
  emit msgReceived(tempStr);
}

void EQInterface::channelMessage(const channelMessageStruct* cmsg, bool client)
{
  QString tempStr;

  // avoid client chatter and do nothing if not viewing channel messages
  if (client || !m_viewChannelMsgs)
    return;

  switch (cmsg->chanNum)
  {
  case 0:
    tempStr.sprintf("Guild");
    break;
    
  case 2:
    tempStr.sprintf("Group");
    break;
    
  case 3:
    tempStr.sprintf("Shout");
    break;
    
  case 4:
    tempStr.sprintf("Auction");
    break;
    
  case 5:
    tempStr.sprintf("OOC");
    break;
    
  case 7:
    tempStr.sprintf("Tell");
    break;
    
  case 8:
    tempStr.sprintf("Say");
    break;
    
  case 15:
    tempStr.sprintf("GM-Tell");
    break;
    
  default:
    tempStr.sprintf("Chan%02x", cmsg->chanNum);
    break;
  }
  
  if (cmsg->language)
  {
    tempStr.sprintf( "%s: '%s' - %s {%s}",
		       tempStr.ascii(),
		     cmsg->sender,
		     cmsg->message,
		     (const char*)language_name(cmsg->language)
		     );
  }
  else // Don't show common, its obvious
  {
    tempStr.sprintf( "%s: '%s' - %s",
		     tempStr.ascii(),
		     cmsg->sender,
		     cmsg->message
		     );
  }
  
  emit msgReceived(tempStr);
}

void EQInterface::random(const randomStruct* randr)
{
  printf ( "RANDOM: Request random number between %d and %d\n",
	   randr->bottom,
	   randr->top);
}

void EQInterface::emoteText(const emoteTextStruct* emotetext)
{
  QString tempStr;

  if (!m_viewChannelMsgs)
    return;

  tempStr.sprintf("Emote: %s", emotetext->text);
  emit msgReceived(tempStr);
}

void EQInterface::playerBook(const bookPlayerStruct* bookp)
{
  QString tempStr;

  // Add Item to Database
  if (pItemDB != NULL) 
    pItemDB->AddItem(&bookp->item);
  
  if (!showeq_params->no_bank)
  {
    tempStr = QString("Item: Book: ") + bookp->item.name
      + ", " + bookp->item.lore
      + ", " + bookp->item.book.file
      + ", Value: " + reformatMoney(bookp->item.cost);
    
    emit msgReceived(tempStr);
  }
}

void EQInterface::playerContainer(const containerPlayerStruct *containp)
{
  QString tempStr;

  // Add Item to Database
  if (pItemDB != NULL) 
    pItemDB->AddItem(&containp->item);
  
  if (!showeq_params->no_bank)
  {
    tempStr = QString("Item: Container: ") + containp->item.lore
      + "(" + QString::number(containp->item.itemNr)
      + "), Slot: " + QString::number(containp->item.equipSlot)
      + ", Value: " + reformatMoney(containp->item.cost);
    
    emit msgReceived(tempStr);
    
    tempStr = QString("Item: Container: Slots: ") 
      + QString::number(containp->item.common.container.numSlots)
      + ", Size Capacity: " 
      + size_name(containp->item.common.container.sizeCapacity)
      + ", Weight Reduction: " 
      + QString::number(containp->item.common.container.weightReduction)
      + "%";
    
    emit msgReceived(tempStr);
  }
}

void EQInterface::inspectData(const inspectingStruct *inspt)
{
  for (int inp = 0; inp < 21; inp ++)
    printf("He has %s (icn:%i)\n", inspt->itemNames[inp], inspt->icons[inp]);
  
  printf("His info: %s\n", inspt->mytext);
}

void EQInterface::spMessage(const spMesgStruct *spmsg)
{
  QString tempStr;

  // Seems to be lots of blanks
  if (!spmsg->message[0])
    return;
  
  //printf("== msgType=%d, msg='%s'\n", spmsg->msgType, spmsg->message);
  // This seems to be several type of message...
  // CJD - why was there no breaks in each case?
  switch(spmsg->msgType)
  {
  case 13:
  {
    if (!strncmp(spmsg->message, "Your target is too far away", 20))
      tempStr.sprintf("Attack: %s", spmsg->message);
    
    else if (!strncmp(spmsg->message, "You can't see your target", 18))
      tempStr.sprintf("Attack: %s", spmsg->message);
    
    // CJD - is this resist any longer type 13, or was it
    // moved to 289 (what I see from some brief logging)?
    else if (!strncmp(spmsg->message, "Your target resisted", 18))
    {
      printf("== let cjd1@users.sourceforge.net know if you ever see this message.\n");
      tempStr.sprintf("Spell: %s", spmsg->message);
    }
    else
      tempStr.sprintf("Special: %s (%d)", spmsg->message, spmsg->msgType);
    break;
  }
  case 15:
    tempStr.sprintf("\e[0;33mExp: %s\e[0;0m", spmsg->message);
    break;
    
    // CJD TODO - make these signals themselves? or just one
    // spellMessage(QString&) and let spellshell split them up...
  case 284: // Your xxx spell has worn off.
    // CJD - No way with this to tell which spell wore off
    // if the same spell is on two different targets...
    tempStr.sprintf("Spell: %s", spmsg->message);
    break;
    
    // Your target resisted the xxx spell.  OR
    // Your spell fizzles.
  case 289:
    tempStr.sprintf("Spell: %s", spmsg->message);
    break;
    
  default:
    tempStr.sprintf("Special: %s (%d)", spmsg->message, spmsg->msgType);
  }
  
  if (tempStr.left(6) == "Spell:")
    emit spellMessage(tempStr);
  
  emit msgReceived(tempStr);
}

void EQInterface::handleSpell(const spellCastStruct* mem, bool client)
{
  QString tempStr;

  if (!showeq_params->showSpellMsgs)
    return;
  
  tempStr = "";
  
  switch (mem->param2)
  {
  case 0:
    {
      if (!client)
	tempStr = "You have finished scribing '";
      break;
    }
    
  case 1:
    {
      if (!client)
	tempStr = "You have finished memorizing '";
      break;
    }
    
  case 2:
    {
      if (!client)
	tempStr = "You forget '";
      break;
    }
    
  case 3:
    {
      if (!client)
	tempStr = "You finish casting '";
      break;
    }
    
  default:
    {
      tempStr.sprintf( "Unknown Spell Event ( %s ) - '",
		       client  ?
		     "Client --> Server"   :
		       "Server --> Client"
		       );
      break;
    }
  }
  
  
  if (!tempStr.isEmpty())
  {
    if (mem->param2 != 3)
      tempStr.sprintf("SPELL: %s%s', slot %d.", 
		      tempStr.ascii(), 
		      (const char*)spell_name (mem->spellId), 
		      mem->spawnId);
    else 
    {
      tempStr.sprintf("SPELL: %s%s'.", 
		      tempStr.ascii(), 
		      (const char*)spell_name (mem->spellId));
    }

    emit msgReceived(tempStr);
  }
}

void EQInterface::beginCast(const beginCastStruct *bcast)
{
  QString tempStr;

  if (!showeq_params->showSpellMsgs)
    return;
  
  tempStr = "";

  if (bcast->spawnId == m_player->getPlayerID())
    tempStr = "You begin casting '";
  else
  {
    const Item* item = m_spawnShell->findID(tSpawn, bcast->spawnId);
    if (item != NULL)
      tempStr = item->name();
    
    if (tempStr == "" || tempStr.isEmpty())
      tempStr.sprintf("UNKNOWN (ID: %d)", bcast->spawnId);
    
    tempStr += " has begun casting '";
  }
  float casttime = ((float)bcast->param1 / 1000);
  tempStr.sprintf( "SPELL: %s%s' - Casting time is %g Second%s", tempStr.ascii(),
		   (const char*)spell_name(bcast->spellId), casttime,
		   casttime == 1 ? "" : "s"
		   );
  emit msgReceived(tempStr);
}

void EQInterface::interruptSpellCast(const interruptCastStruct *icast)
{
  const Item* item = m_spawnShell->findID(tSpawn, icast->spawnId);

  if (item != NULL)
    printf("SPELL: %s(%d): %s\n", 
	   (const char*)item->name(), icast->spawnId, icast->message);
  else
    printf("SPELL: spawn(%d): %s\n", 
	   icast->spawnId, icast->message);
}

void EQInterface::startCast(const castStruct* cast)
{
  printf("SPELL: You begin casting %s.  Current Target is ", 
	 (const char*)spell_name(cast->spellId) );
  
  const Item* item = m_spawnShell->findID(tSpawn, cast->targetId);

  if (item != NULL)
    printf("%s(%d)", (const char*)item->name(), cast->targetId);
  else
    printf("spawn(%d)", cast->targetId);
  
  printf("\n");
}

void EQInterface::systemMessage(const systemMessageStruct* smsg)
{
  QString tempStr;

  // Seems to be lots of blanks
  if (!smsg->message[0])
    return;
  
  // This seems to be several type of message...
  if (!strncmp(smsg->message, "Your faction", 12))
    tempStr.sprintf("Faction: %s", smsg->message);
  else
    tempStr.sprintf("System: %s", smsg->message);
  
  emit msgReceived(tempStr);
}

void EQInterface::newGroundItem(const dropThingOnGround* adrop, bool client)
{
  QString tempStr;

  if (!client)
    return;

  /* If the packet is outbound  ( Client --> Server ) then it
     should not be added to the spawn list... The server will
     send the client a packet when it has actually placed the
     item on the ground.
  */
  if (pItemDB != NULL) 
    tempStr = pItemDB->GetItemLoreName(adrop->itemNr);
  else
    tempStr = "";
  
  if (tempStr != "")
  {
    tempStr.prepend("Item: Drop: You have dropped your '");
    tempStr.append("' on the ground!");
  }
  else
    tempStr = QString("Item: Drop: You have dropped your *UNKNOWN ITEM* (ID: %1)  on the ground!\nNOTE:\tIn order for ShowEQ to know the name of the item you dropped it is suggested that you pickup and drop the item again...").arg(adrop->itemNr);
  
  emit msgReceived(tempStr);
}

void EQInterface::moneyUpdate(const moneyUpdateStruct* money)
{
  emit msgReceived("Money: Update");
}

void EQInterface::moneyThing(const moneyUpdateStruct* money)
{
  emit msgReceived("Money: Thing");
}

void EQInterface::groupInfo(const groupMemberStruct* gmem)
{
  printf ("Member: %s - %s (%i)\n", 
	  gmem->yourname, gmem->membername, gmem->oper);
}

void EQInterface::zoneEntry(const ClientZoneEntryStruct* zsentry)
{
  QString tempStr;
  emit newZoneName("- Unknown -");
  stsMessage("- Busy Zoning -");
#ifdef ZONE_ORDER_DIAG
  tempStr = "Zone: EntryCode: Client";
  emit msgReceived(tempStr);
#endif
  emit stsMessage(QString("Zoning..."));
  emit msgReceived("Zone: Cleared stat modifiers.");
}

void EQInterface::zoneEntry(const ServerZoneEntryStruct* zsentry)
{
  QString tempStr;

#ifdef ZONE_ORDER_DIAG
  tempStr = "Zone: EntryCode: Server, Zone: ";
  tempStr += zsentry->zoneShortName;
  emit msgReceived(tempStr);
#endif
  tempStr = QString("Zone: Zoning, Please Wait...\t\t(Zone: '")
    + zsentry->zoneShortName + "')";
  emit msgReceived(tempStr);
  emit newZoneName(zsentry->zoneShortName);
}

void EQInterface::zoneChange(const zoneChangeStruct* zoneChange, bool client)
{
  QString tempStr;

  emit newZoneName("- Unknown -");
  stsMessage("- Busy Zoning -");

  if (client)
  {
#ifdef ZONE_ORDER_DIAG
    tempStr = "Zone: ChangeCode: Client, Zone:";
    tempStr += zoneChange->zoneName;
    emit msgReceived(tempStr);
#endif
  }
  else
  {
    printf("Loading, Please Wait...\t\t(Zone:\t\'%s\')\n",
	   zoneChange->zoneName);
#ifdef ZONE_ORDER_DIAG
    tempStr = "Zone: ChangeCode: Server, Zone:";
    tempStr += zoneChange->zoneName;
    emit msgReceived(tempStr);
#endif
    tempStr = QString("Zone: Zoning, Please Wait...\t\t(Zone: '")
      + zoneChange->zoneName + "')";
    emit msgReceived(tempStr);
  }
}

void EQInterface::zoneNew(const newZoneStruct* zoneNew, bool client)
{
  QString tempStr;

#ifdef ZONE_ORDER_DIAG
  tempStr = "Zone: NewCode: Zone: ";
  tempStr += QString(zoneNew->shortName) + " ("
    + zoneNew->longName + ")";
  emit msgReceived(tempStr);
#endif
  tempStr = QString("Zone: Entered: ShortName = '") + 
    zoneNew->shortName +
    "' LongName = " + 
    zoneNew->longName;
  emit msgReceived(tempStr);

   if (pSEQPrefs->getPrefBool("UseStdout", "Interface"))
     printf("Loading Complete...\t\t(Zone:\t'%s')\n", zoneNew->shortName);

  emit newZoneName(zoneNew->longName);
  stsMessage("");
}

void EQInterface::clientTarget(const clientTargetStruct* cts)
{
  if (!showeq_params->tar_select)
    return;

  // try to find the targeted spawn in the spawn shell
  const Item* item = m_spawnShell->findID(tSpawn, cts->newTarget);

  // if found, make it the currently selected target
  if (item)
  {
    // note the new selection
    m_selectedSpawn = item;
    
    // notify others of the new selected spawn
    emit selectSpawn(m_selectedSpawn);

    // update the spawn status
    updateSelectedSpawnStatus(m_selectedSpawn);
  }
}

void EQInterface::spawnSelected(const Item* item)
{
  if (item == NULL)
    return;

  // note the new selection
  m_selectedSpawn = item;
  
  // notify others of the new selected spawn
  emit selectSpawn(m_selectedSpawn);

  // update the spawn status
  updateSelectedSpawnStatus(m_selectedSpawn);
}

void EQInterface::spawnConsidered(const Item* item)
{
  if (item == NULL)
    return;

  if (!showeq_params->con_select)
    return;

  // note the new selection
  m_selectedSpawn = item;
  
  // notify others of the new selected spawn
  emit selectSpawn(m_selectedSpawn);
  
  // update the spawn status
  updateSelectedSpawnStatus(m_selectedSpawn);
}

void EQInterface::addItem(const Item* item)
{
  uint32_t filterFlags = item->filterFlags();
  if (filterFlags & FILTER_FLAG_LOCATE)
  {
    printf ("\n-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
    printf ("LocateSpawn: %s spawned LOC %dy, %dx, %dz at %s",
	    (const char*)item->name(), 
	    item->yPos(), item->xPos(), item->yPos(),
	    (const char*)item->spawnTimeStr());
    printf ("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n\n");
    
    if (showeq_params->spawnfilter_loglocates)
      logFilteredSpawn(item, FILTER_FLAG_LOCATE);

    // Gereric system beep for those without a soundcard
    //
    if (!showeq_params->spawnfilter_audio)  fprintf(stderr,"\a");
    else
      doAlertCommand(item, 
		     pSEQPrefs->getPrefString("LocateSpawnAudioCommand", "Filters"),
		     "Locate Spawned");

    // note the new selection
    m_selectedSpawn = item;
    
    // notify others of the new selected spawn
    emit selectSpawn(m_selectedSpawn);
    
    // update the spawn status
    updateSelectedSpawnStatus(m_selectedSpawn);
  } // End LOCATE Filter alerting
  
  if (filterFlags & FILTER_FLAG_CAUTION)
  {
    printf ("CautionSpawn: %s spawned LOC %dy, %dx, %dz at %s", 
	    (const char*)item->name(), 
	    item->yPos(), item->xPos(), item->yPos(),
	    (const char*)item->spawnTimeStr());
    
    if (showeq_params->spawnfilter_logcautions)
      logFilteredSpawn(item, FILTER_FLAG_CAUTION);

    // Gereric system beep for those without a soundcard
    //
    if (!showeq_params->spawnfilter_audio)  fprintf(stderr,"\a");
    else
      doAlertCommand(item, 
		     pSEQPrefs->getPrefString("CautionSpawnAudioCommand", "Filters"),
		     "Caution Spawned");
  } // End CAUTION Filter alerting
  
  if (filterFlags & FILTER_FLAG_HUNT)
  {
    printf ("HuntSpawn: %s spawned LOC %dy, %dx, %dz at %s", 
	    (const char*)item->name(), 
	    item->yPos(), item->xPos(), item->yPos(),
	    (const char*)item->spawnTimeStr());
    
    if (showeq_params->spawnfilter_loghunts)
      logFilteredSpawn(item, FILTER_FLAG_HUNT);

    // Gereric system beep for those without a soundcard
    //
    if (!showeq_params->spawnfilter_audio)  fprintf(stderr,"\a");
    else
      doAlertCommand(item, 
		     pSEQPrefs->getPrefString("HuntSpawnAudioCommand", "Filters"),
		     "Hunt Spawned");
  } // End HUNT Filter alerting
  
  if (filterFlags & FILTER_FLAG_DANGER)
  {
    printf ("DangerSpawn: %s spawned LOC %dy, %dx, %dz at %s", 
	    (const char*)item->name(), 
	    item->yPos(), item->xPos(), item->yPos(),
	    (const char*)item->spawnTimeStr());
    
    if (showeq_params->spawnfilter_logdangers)
      logFilteredSpawn(item, FILTER_FLAG_DANGER);

    // Gereric system beep for those without a soundcard
    //
    if (!showeq_params->spawnfilter_audio)  fprintf(stderr,"\a");
    else
      doAlertCommand(item, 
		     pSEQPrefs->getPrefString("DangerSpawnAudioCommand", "Filters"),
		     "Danger Spawned");
  } // End DANGER Filter alerting
}


void EQInterface::delItem(const Item* item)
{
  // if this is the selected spawn, then there isn't a selected spawn anymore
  if (m_selectedSpawn == item)
  {
    m_selectedSpawn = NULL;
  
    // notify others of the new selected spawn
    emit selectSpawn(m_selectedSpawn);
  }
}

void EQInterface::killSpawn(const Item* item)
{
  if (item == NULL)
    return;

  if (m_selectedSpawn != item)
    return;

  // update status message, notifying that selected spawn has died
  QString string = m_selectedSpawn->name() + " died";

  stsMessage(string);
}

void EQInterface::changeItem(const Item* item)
{
  // if this isn't the selected spawn, nothing more to do
  if (item != m_selectedSpawn)
    return;

  updateSelectedSpawnStatus(item);
}

void EQInterface::handleAlert(const Item* item, 
			      alertType type)
{
  QString prefix;
  switch (type)
    {
    case tNewSpawn:
      prefix = "Spawn:";
      break;
    case tFilledSpawn:
      prefix = "Filled:";
      break;
    case tKillSpawn:
      prefix = "Died:";
      break;
    case tDelSpawn:
      prefix = "DeSpawn:";
      break;
    default:
      prefix = "WTF:";
    }

  QString msg;
  if (pSEQPrefs->getPrefBool("AlertInfo", "Filters"))
  {
    long timeval;
    struct tm *tp;
    
    time(&timeval);
    tp=localtime(&timeval);

    QString temp;

    msg = prefix + item->name() + "/" + item->raceName() 
      + "/" + item->className();

    const Spawn* spawn = spawnType(item);

    if (spawn)
      msg += QString("/") + spawn->lightName();

    // aditional info or new spawns
    if (type == tNewSpawn)
    {
      if (spawn)
	temp.sprintf(" [%d-%d-%d %d:%d:%d] (%d,%d,%d) LVL %d, HP %d/%d", 
		     1900 + tp->tm_year, tp->tm_mon, tp->tm_mday,
		     tp->tm_hour, tp->tm_min, tp->tm_sec,
		     item->xPos(), item->yPos(), item->zPos(),
		     spawn->level(), spawn->HP(), spawn->maxHP());
      else
	temp.sprintf(" [%d-%d-%d %d:%d:%d] (%d,%d,%d)", 
		     1900 + tp->tm_year, tp->tm_mon, tp->tm_mday,
		     tp->tm_hour, tp->tm_min, tp->tm_sec,
		     item->xPos(), item->yPos(), item->zPos());
    }
    else
      temp.sprintf(" [%d-%d-%d %d:%d:%d]", 
		    1900 + tp->tm_year, tp->tm_mon, tp->tm_mday,
		    tp->tm_hour, tp->tm_min, tp->tm_sec);
      
    msg += temp;
  }
  else
    msg = prefix + item->name();

  emit msgReceived(msg);

  // Gereric system beep for those without a soundcard
  //
  if (!pSEQPrefs->getPrefBool("SpawnFilterAudio", "Filters"))
    printf ("\a");
  else
  {
    // execute a command
    QString command;
    QString audioCmd;
    QString audioCue;
    switch (type)
    {
    case tNewSpawn:
      audioCmd = "SpawnAudioCommand";
      audioCue = "Spawned";
      break;
    case tFilledSpawn:
      audioCmd = "SpawnAudioCommand";
      audioCue = "Filled";
      break;
    case tKillSpawn:
      audioCmd = "DeathAudioCommand";
      audioCue = "Died";
      break;
    case tDelSpawn:
      audioCmd = "DeSpawnAudioCommand";
      audioCue = "Despawned";
      break;
    }
    
    doAlertCommand(item, pSEQPrefs->getPrefString(audioCmd, "Filters"), audioCue);
  }
}

void EQInterface::doAlertCommand(const Item* item, 
				 const QString& rawCommand, 
				 const QString& audioCue)
{
  if (rawCommand.isEmpty())
    return;

  // time to cook the command line
  QString command = rawCommand;

  QRegExp nameExp("%n");
  QRegExp cueExp("%c");
  
  // get the name, and replace all occurrances of %n with the name
  QString name = item->transformedName();
  command.replace(nameExp, name);
  
  // now, replace all occurrances of %c with the audio cue
  command.replace(cueExp, audioCue);
  
  // fire off the command
  system ((const char*)command);
}

void EQInterface::logFilteredSpawn(const Item* item, uint32_t flag)
{
  FILE *rar;
  rar = fopen("/usr/local/share/showeq/filtered.spawns","at");
  if (rar) 
  {
    fprintf (rar, "%s %s spawned LOC %dy, %dx, %dz at %s", 
	     (const char*)m_filterMgr->filterString(flag),
	     (const char*)item->name(), 
	     item->yPos(), item->xPos(), item->yPos(),
	     (const char*)item->spawnTimeStr());
    fclose(rar);
  }
}

void EQInterface::updateSelectedSpawnStatus(const Item* item)
{
  if (item == NULL)
    return;

  const Spawn* spawn = spawnType(item);

  // construct a message for the status message display
  QString string("");
  if (spawn != NULL)
    string.sprintf("%d: %s:%d (%d/%d) Pos:", // "%d/%d/%d (%d) %s %s Item:%s", 
		   item->id(),
		   (const char*)item->name(), 
		   spawn->level(), spawn->HP(),
		   spawn->maxHP());
  else
    string.sprintf("%d: %s: Pos:", // "%d/%d/%d (%d) %s %s Item:%s", 
		   item->id(),
		   (const char*)item->name());

  if (showeq_params->retarded_coords)
    string += QString::number(item->yPos()) + "/" 
      + QString::number(item->xPos()) + "/" 
      + QString::number(item->zPos());
  else
    string += QString::number(item->xPos()) + "/" 
      + QString::number(item->yPos()) + "/" 
      + QString::number(item->zPos());

  string += QString(" (") 
    + QString::number(item->calcDist(m_player->getPlayerX(),
				     m_player->getPlayerY(),
				     m_player->getPlayerZ()))
    + ") " + item->raceName() + " " + item->className();

  // just call the status message method
  stsMessage(string);
}

void EQInterface::addCategory(void)
{
  if (m_categoryMgr)
    m_categoryMgr->addCategory();
}

void EQInterface::reloadCategories(void)
{
  if (m_categoryMgr)
    m_categoryMgr->reloadCategories();
}

void EQInterface::rebuildSpawnList()
{
  if (m_spawnList)
    m_spawnList->rebuildSpawnList();
}

void EQInterface::selectNext(void)
{
  if (m_spawnList)
    m_spawnList->selectNext();
}

void EQInterface::selectPrev(void)
{
  if (m_spawnList)
    m_spawnList->selectPrev();
}

void EQInterface::toggleAutoDetectCharSettings (int id)
{
    showeq_params->AutoDetectCharSettings = !showeq_params->AutoDetectCharSettings;
    menuBar()->setItemChecked (id, showeq_params->AutoDetectCharSettings);
    pSEQPrefs->setPrefInt("AutoDetectCharSettings", "Defaults", showeq_params->AutoDetectCharSettings);
}

/* Choose the character's level */
void EQInterface::SetDefaultCharacterLevel(int level)
{
#if (QT_VERSION >= 210) /* Won't work on older versions of QT... */
    showeq_params->defaultLevel = level;
    pSEQPrefs->setPrefInt("DefaultLevel", "Defaults", level);
#endif
}

void EQInterface::SetDefaultCharacterLevel_COMPATABILITY (void)
{
#if (QT_VERSION < 210)
   dialogbox = new QDialog(this, "dialogbox", TRUE);
   QBoxLayout *levelLayout = new QVBoxLayout(dialogbox,5);

   QLabel *levelMessage = new QLabel(dialogbox);
   levelMessage->setText("Please enter your character's level in the box below.");

   levelSpinBox = new QSpinBox(dialogbox);
   levelSpinBox->setValue(showeq_params->defaultLevel);
   levelSpinBox->setRange(1,60);

   levelLayout->add(levelMessage);
   levelLayout->add(levelSpinBox);

   QBoxLayout *buttonLayout = new QHBoxLayout(levelLayout,5);
   QPushButton *ok = new QPushButton( "Apply", dialogbox);
   QPushButton *reset = new QPushButton( "Cancel", dialogbox);
   buttonLayout->add(ok);
   buttonLayout->add(reset);

   // This has a really kludgy feel to it.  I'm sure there's a magical
   // Qt way to pop open a dialog and return the value of a spinbox
   // via pushbuttons without having to be this explicit.  cpphack

   connect( ok,    SIGNAL(clicked()), SLOT(SetCharLevel()));
   connect( reset, SIGNAL(clicked()), SLOT(ResetCharLevel()));
   dialogbox->exec();
#endif
}

void EQInterface::SetCharLevel(void)
{
#if (QT_VERSION < 210)
  showeq_params->defaultLevel = levelSpinBox->value();
  dialogbox->hide();
#endif
}

void EQInterface::ResetCharLevel(void)
{
#if (QT_VERSION < 210)
   levelSpinBox->setValue(showeq_params->defaultLevel);
   dialogbox->hide();
#endif
}

/* Simple routine to set "greb next client address spotted" flag */
void EQInterface::grabNextAddr ()
{
  printf("Grabbing the client IP from the next EverQuest packet seen...\n");
  showeq_params->ip = strdup ("127.0.0.0");
}

void EQInterface::selectTheme( int id )
{
#if ((QT_VERSION >= 210) && !defined (SEQ_OVERRIDE_STYLES))
/*
 moc seems to ignore the fact that this code would
 not have been compiled, and adds the connect
 statement for this function to the m_interface file
 (which then complains about the class having no
 member selectTheme). So the function stays, empty,
 if qt is less than 210.
*/
    static QFont OrigFont = qApp->font();
    static QPalette OrigPalette = qApp->palette();;

    MenuIDList::Iterator iter;
    for ( iter = IDList_StyleMenu.begin(); iter != IDList_StyleMenu.end(); ++iter)
      menuBar()->setItemChecked( (*iter), false );

    menuBar()->setItemChecked( id, true );
    int theme = menuBar()->itemParameter(id);

    switch ( theme )
    {
    case 1: // platinum
    {
      QPalette p( QColor( 239, 239, 239 ) );
      qApp->setStyle( (QStyle *) new QPlatinumStyle );
      qApp->setPalette( p, TRUE );
      pSEQPrefs->setPrefInt("Theme", "Interface", theme);
    }
    break;
    case 2: // windows
    {
      qApp->setStyle( (QStyle *) new QWindowsStyle );
      qApp->setFont( OrigFont, TRUE );
      qApp->setPalette( OrigPalette, TRUE );
      pSEQPrefs->setPrefInt("Theme", "Interface", theme);
    }
    break;
    case 3: // cde 
    case 4: // cde polished
    {
      QPalette p( QColor( 75, 123, 130 ) );
      qApp->setStyle( (QStyle *) new QCDEStyle( theme == 3 ? TRUE : FALSE ) );
      p.setColor( QPalette::Active, QColorGroup::Base, QColor( 55, 77, 78 ) );
      p.setColor( QPalette::Inactive, QColorGroup::Base, QColor( 55, 77, 78 ) );
      p.setColor( QPalette::Disabled, QColorGroup::Base, QColor( 55, 77, 78 ) );
      p.setColor( QPalette::Active, QColorGroup::Highlight, Qt::white );
      p.setColor( QPalette::Active, QColorGroup::HighlightedText, QColor( 55, 77, 78 ) );
      p.setColor( QPalette::Inactive, QColorGroup::Highlight, Qt::white );
      p.setColor( QPalette::Inactive, QColorGroup::HighlightedText, QColor( 55, 77, 78 ) );
      p.setColor( QPalette::Disabled, QColorGroup::Highlight, Qt::white );
      p.setColor( QPalette::Disabled, QColorGroup::HighlightedText, QColor( 55, 77, 78 ) );
      p.setColor( QPalette::Active, QColorGroup::Foreground, Qt::white );
      p.setColor( QPalette::Active, QColorGroup::Text, Qt::white );
      p.setColor( QPalette::Active, QColorGroup::ButtonText, Qt::white );
      p.setColor( QPalette::Inactive, QColorGroup::Foreground, Qt::white );
      p.setColor( QPalette::Inactive, QColorGroup::Text, Qt::white );
      p.setColor( QPalette::Inactive, QColorGroup::ButtonText, Qt::white );
      p.setColor( QPalette::Disabled, QColorGroup::Foreground, Qt::lightGray );
      p.setColor( QPalette::Disabled, QColorGroup::Text, Qt::lightGray );
      p.setColor( QPalette::Disabled, QColorGroup::ButtonText, Qt::lightGray );
      qApp->setPalette( p, TRUE );
      qApp->setFont( QFont( "times", OrigFont.pointSize() ), TRUE );
      pSEQPrefs->setPrefInt("Theme", "Interface", theme);
    }
    break;
    case 5: // motif
    {
      QPalette p( QColor( 192, 192, 192 ) );
      qApp->setStyle( (QStyle *) new QMotifStyle );
      qApp->setPalette( p, TRUE );
      qApp->setFont( OrigFont, TRUE );
      pSEQPrefs->setPrefInt("Theme", "Interface", theme);
    }
    break;
    case 6: // SGI
    {
      //QPalette p( QColor( 192, 192, 192 ) );
      qApp->setStyle( (QStyle *) new QSGIStyle( FALSE ) );
      qApp->setPalette( OrigPalette, TRUE );
      qApp->setFont( OrigFont, TRUE );
      pSEQPrefs->setPrefInt("Theme", "Interface", theme);
    }
    break;
    default: // system default
    {
      QPalette p( QColor( 192, 192, 192 ) );
      qApp->setStyle( (QStyle *) new QMotifStyle );
      qApp->setPalette( p, TRUE );
      qApp->setFont( OrigFont, TRUE );
      pSEQPrefs->setPrefInt("Theme", "Interface", 2);
    }
    break;
    }
#endif /* QT_VERSION */
}

void EQInterface::showMap(int i)
{
  if ((i > maxNumMaps) || (i < 0))
    return;

  // if it doesn't exist, create it
  if (m_map[i] == NULL)
  {
    int mapNum = i + 1;
    QString mapPrefName = "Map";
    QString mapName = QString("map") + QString::number(mapNum);
    QString mapCaption = "Map ";

    if (i != 0)
    {
      mapPrefName += QString::number(mapNum);
      mapCaption += QString::number(mapNum);
    }

    if (m_isMapDocked[i])
      m_map[i] = new MapFrame(m_filterMgr,
			      m_mapMgr,
			      m_player, 
			      m_spawnShell, 
			      mapPrefName, 
			      mapCaption,
			      mapName, 
			      m_splitH);
    else
      m_map[i] = new MapFrame(m_filterMgr,
			      m_mapMgr,
			      m_player, 
			      m_spawnShell, 
			      mapPrefName, 
			      mapCaption,
			      mapName, 
			      NULL);
      
    connect(this, SIGNAL(saveAllPrefs(void)),
	    m_map[i], SLOT(savePrefs(void)));
    
    // Get the map...
    Map* map = m_map[i]->map();
    
    // supply the Map slots with signals from EQInterface
    connect (this, SIGNAL(selectSpawn(const Item*)), 
	     map, SLOT(selectSpawn(const Item*)));
    
    // supply the Map slots with signals from EQPacket
    connect (m_packet, SIGNAL(refreshMap(void)), 
	     map, SLOT(refreshMap(void)));
    
    // supply EQInterface slots with signals from Map
    connect (map, SIGNAL(spawnSelected(const Item*)),
	     this, SLOT(spawnSelected(const Item*)));

    // restore it's position if necessary and practical
    if (!m_isMapDocked[i] && 
	(pSEQPrefs->getPrefBool("UseWindowPos", "Interface", 0) != 0))
    {
      m_map[i]->restorePosition();
      m_map[i]->restoreSize();
    }
  }
      
  // make sure it's visible
  m_map[i]->show();
}

void EQInterface::showStatList(void)
{
  // if it doesn't exist, create it
  if (m_statList == NULL)
  {
    if (m_isStatListDocked)
      m_statList = new EQStatList(m_player, m_splitT, "skills");
    else
      m_statList = new EQStatList(m_player, NULL, "skills");

    connect(this, SIGNAL(saveAllPrefs(void)),
	    m_statList, SLOT(savePrefs(void)));
  }

  // make sure it's visible
  m_statList->show();
}

void EQInterface::showSkillList(void)
{
  // if it doesn't exist, create it
  if (m_skillList == NULL)
  {
    if (m_isSkillListDocked)
      m_skillList = new EQSkillList(m_player, m_splitT, "skills");
    else
      m_skillList = new EQSkillList(m_player, NULL, "skills");
    
    connect(this, SIGNAL(saveAllPrefs(void)),
	    m_skillList, SLOT(savePrefs(void)));
  }
      
  // make sure it's visible
  m_skillList->show();
}

void EQInterface::showSpawnList(void)
{
  // if it doesn't exist, create it.
  if (m_spawnList == NULL)
  {
    if (m_isSpawnListDocked)
      m_spawnList = new CSpawnList (m_player, m_spawnShell, m_categoryMgr,
				    m_splitV, "spawnlist");
    else
      m_spawnList = new CSpawnList (m_player, m_spawnShell, m_categoryMgr,
				    NULL, "spawnlist");

     // connectsion from spawn list to interface
     connect (m_spawnList, SIGNAL(spawnSelected(const Item*)),
	      this, SLOT(spawnSelected(const Item*)));

     // connections from interface to spawn list
     connect (this, SIGNAL(selectSpawn(const Item*)),
	      m_spawnList, SLOT(selectSpawn(const Item*)));
     connect(this, SIGNAL(saveAllPrefs(void)),
	     m_spawnList, SLOT(savePrefs(void)));
  }

  // make sure it's visible
  m_spawnList->show();
}

void EQInterface::showSpellList(void)
{
  // if it doesn't exist, create it
  if (m_spellList == NULL)
  {
    if (m_isSpellListDocked)
      m_spellList = new SpellList(m_splitV, "spelllist");
    else
      m_spellList = new SpellList(NULL, "spelllist");

    // connect SpellShell to SpellList
    connect(m_spellShell, SIGNAL(addSpell(SpellItem *)),
	    m_spellList, SLOT(addSpell(SpellItem *)));
    connect(m_spellShell, SIGNAL(delSpell(SpellItem *)),
	    m_spellList, SLOT(delSpell(SpellItem *)));
    connect(m_spellShell, SIGNAL(changeSpell(SpellItem *)),
	    m_spellList, SLOT(changeSpell(SpellItem *)));
    connect(m_spellShell, SIGNAL(clearSpells()),
	    m_spellList, SLOT(clear()));
    connect(this, SIGNAL(saveAllPrefs(void)),
	    m_spellList, SLOT(savePrefs(void)));

  }
      
  // make sure it's visible
  m_spellList->show();
}

void EQInterface::showCompass(void)
{
  // if it doesn't exist, create it.
  if (m_compass == NULL)
  {
    if (m_isCompassDocked)
      m_compass = new CompassFrame(m_player, m_splitT, "compass");
    else
      m_compass = new CompassFrame(m_player, NULL, "compass");

    // supply the compass slots with EQInterface signals
     connect (this, SIGNAL(selectSpawn(const Item*)),
	      m_compass, SLOT(selectSpawn(const Item*)));
  }

  // make sure it's visible
  m_compass->show();
}

