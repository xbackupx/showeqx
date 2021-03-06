/*
 * skilllist.cpp
 *
 *  ShowEQ Distributed under GPL
 *  http://seq.sourceforge.net/
 */

#include "player.h"
#include "skilllist.h"
#include "util.h"
#include "main.h" // for pSEQPrefs & showeq_params

EQSkillList::EQSkillList(EQPlayer* player,
			 QWidget* parent, 
			 const char* name)
  : QListView(parent, name), 
  m_pPlayer(player)
{
  int i;

  // initialize the lists
  for (i = 0; i < MAX_KNOWN_SKILLS; i++)
    m_skillList[i] = NULL;

  for (i = 0; i < MAX_KNOWN_LANGS; i++)
    m_languageList[i] = NULL;

  QString section = "SkillList";
#if QT_VERSION >= 210
   setShowSortIndicator(TRUE);
#endif
   setRootIsDecorated(false);
   setCaption(pSEQPrefs->getPrefString("Caption", section,
				       "ShowEQ - Skills"));

  // set font and add the columns
  setFont(QFont("Helvetica", showeq_params->fontsize));
  addColumn("Skill");
  addColumn("Value");
  setAllColumnsShowFocus(true);

  // get preferences

  // column sizes
  if (pSEQPrefs->isPreference("SkillWidth", section))
  {
    i = pSEQPrefs->getPrefInt("SkillWidth", section, columnWidth(0));
    setColumnWidthMode(0, QListView::Manual);
    setColumnWidth(0, i);
  }

  if (pSEQPrefs->isPreference("ValueWidth", section))
  {
    i = pSEQPrefs->getPrefInt("ValueWidth", section, columnWidth(1));
    setColumnWidthMode(1, QListView::Manual);
    setColumnWidth(1, i);
  }

  // connect to player signals
   connect (m_pPlayer, SIGNAL(addSkill(int,int)), 
	    this, SLOT(addSkill(int,int)));
   connect (m_pPlayer, SIGNAL(changeSkill(int,int)), 
	    this, SLOT(changeSkill(int,int)));
   connect (m_pPlayer, SIGNAL(deleteSkills()), 
	    this, SLOT(deleteSkills()));
   connect (m_pPlayer, SIGNAL(addLanguage(int,int)), 
	    this, SLOT(addLanguage(int,int)));
   connect (m_pPlayer, SIGNAL(changeLanguage(int,int)), 
	    this, SLOT(changeLanguage(int,int)));
   connect (m_pPlayer, SIGNAL(deleteLanguages()), 
	    this, SLOT(deleteLanguages()));

   for (i = 0; i < MAX_KNOWN_SKILLS; i++)
     addSkill(i, m_pPlayer->getSkill(i));

   // show the languages or not according to the user preference
   showLanguages(bool(pSEQPrefs->getPrefBool("ShowLanguages", section, 1)));
}

EQSkillList::~EQSkillList()
{
}

/* Called to add a skill to the skills list */
void EQSkillList::addSkill (int skillId, int value)
{
  if (skillId >= MAX_KNOWN_SKILLS)
  {
    printf("Warning: skillId (%d) is more than max skillId (%d)\n", 
	   skillId, MAX_KNOWN_SKILLS - 1);

    return;
  }

  /* Check if this is a valid skill */
  if (value == 255)
    return;

  QString str;

  /* Check if this is a skill not learned yet */
  if (value == 254)
    str = " NA";
  else
    str.sprintf ("%3d", value);

  /* If the skill is not added yet, look up the correct skill namd and add it
   * to the list
   */
  if (!m_skillList[skillId])
    m_skillList[skillId] =
      new QListViewItem (this, skill_name (skillId), str);
  else
    m_skillList[skillId]->setText (1, str);
}

/* Skill update */
void EQSkillList::changeSkill (int skillId, int value)
{
  if (skillId >= MAX_KNOWN_SKILLS)
  {
    printf("Warning: skillId (%d) is more than max skillId (%d)\n", 
	   skillId, MAX_KNOWN_SKILLS - 1);

    return;
  }
                                
  QString str;
  /* Update skill value with new value */
  str.sprintf ("%3d", value);

  // create skill entry if needed or set the value of the existing item
  if (!m_skillList[skillId])
    m_skillList[skillId] =
      new QListViewItem (this, skill_name (skillId), str);
  else
    m_skillList[skillId]->setText (1, str);
}

/* Delete all skills when zoning */
void EQSkillList::deleteSkills() 
{
  for(int i=0; i < MAX_KNOWN_SKILLS; i++)
    if(m_skillList[i] != NULL) 
    {
      delete m_skillList[i];
      m_skillList[i] = NULL;
    }
}

// Called to add a language to the skills list
void EQSkillList::addLanguage (int langId, int value)
{
  // only add it if languages are being shown
  if (!m_showLanguages)
    return;

  if (langId >= MAX_KNOWN_LANGS)
  {
    printf("Warning: langId (%d) is more than max langId (%d)\n", 
	   langId, MAX_KNOWN_LANGS - 1);

    return;
  }
                                
  // Check if this is a valid skill 
  if (value == 255)
    return;

  QString str;

  // Check if this is a skill not learned yet
  if (value == 00)
    str = " NA";
  else
    str.sprintf ("%3d", value);

  // If the language is not added yet, look up the correct skill namd and 
  // add it to the list
  if (!m_languageList[langId])
    m_languageList[langId] =
      new QListViewItem (this, language_name (langId), str);
  else
      m_languageList[langId]->setText (1, str);
}

/* Language update */
void EQSkillList::changeLanguage (int langId, int value)
{
  // only change it if languages are being shown
  if (!m_showLanguages)
    return;

  if (langId >= MAX_KNOWN_LANGS)
  {
    printf("Warning: langId (%d) is more than max langId (%d)\n", 
	   langId, MAX_KNOWN_LANGS - 1);

    return;
  }
                                
  QString str;

  /* Update skill value with new value */
  str.sprintf ("%3d", value);

  // create language entry if needed or set the value of the existing item
  if (!m_languageList[langId])
    m_languageList[langId] =
      new QListViewItem (this, language_name (langId), str);
  else
    m_languageList[langId]->setText (1, str);
}

/* Delete all skills when zoning */
void EQSkillList::deleteLanguages() 
{
  for(int i=0; i < MAX_KNOWN_LANGS; i++)
    if(m_languageList[i] != NULL) 
    {
      delete m_languageList[i];
      m_languageList[i] = NULL;
    }
}

void EQSkillList::showLanguages(bool show)
{
  m_showLanguages = show;

  if (m_showLanguages)
    for (int i = 0; i < MAX_KNOWN_LANGS; i++)
      addLanguage(i, m_pPlayer->getLanguage(i));
  else
    deleteLanguages();
}

void EQSkillList::savePrefs(void)
{
  QString section = "SkillList";
  // only save the preferences if visible
  if (isVisible())
  {
    pSEQPrefs->setPrefString("Caption", section, caption());

    // only save column widths if the user has set for it
    if (pSEQPrefs->getPrefBool("SaveWidth", section, 1))
    {
      pSEQPrefs->setPrefInt("SkillWidth", section, 
			      columnWidth(0));
      pSEQPrefs->setPrefInt("ValueWidth", section, 
			      columnWidth(1));
    }

    // only save language visibility if the user has set for it
    if (pSEQPrefs->getPrefBool("SaveShowLanguages", section, 1))
    {
      pSEQPrefs->setPrefInt("ShowLanguages", section, m_showLanguages);
    }
  }
}
