#include "hud.h"
#include "hud_util.h"
#include "tfclassmenu.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <FileSystem.h>

#include <cl_entity.h>
#include <cdll_dll.h>

#include <vgui_controls/Button.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/ImagePanel.h>
#include <game_controls/TFClassTips.h>
#include <vgui_controls/ScalableImagePanel.h>

#include "CounterStrikeViewport.h"

using namespace vgui;

static const char *g_sDialogVariables[] = {
	"numScout",
	"numSoldier",
	"numPyro",

	"numDemoman",
	"numHeavy",
	"numEngineer",
	
	"numMedic",
	"numSniper",
	"numSpy"
};

static int g_iRemapClassIndex[] = 
{
	CLASS_SCOUT,
	CLASS_SOLDIER,
	CLASS_PYRO,

	CLASS_DEMOMAN,
	CLASS_HEAVY,
	CLASS_ENGINEER,

	CLASS_MEDIC,
	CLASS_SNIPER,
	CLASS_SPY,

	CLASS_RANDOM
};

static int g_iRemapClassButtonIndex[] = 
{
	0,
	0,//CLASS_SCOUT
	4,//CLASS_HEAVY
	1,//CLASS_SOLDIER
	2,//CLASS_PYRO
	7,//CLASS_SNIPER
	6,//CLASS_MEDIC
	5,//CLASS_ENGINEER
	3,//CLASS_DEMOMAN
	8//CLASS_SPY
};

static const char *g_sClassTitle[] = {
	"",
	"scout",
	"soldier",
	"pyro",
	
	"demoman",
	"heavy",
	"engineer",
	
	"medic",
	"sniper",
	"spy",

	"random"
};

static const char *g_sClassImagesBlue[] = {
	"",
	"class_sel_sm_scout_blu",
	"class_sel_sm_soldier_blu",
	"class_sel_sm_pyro_blu",

	"class_sel_sm_demoman_blu",
	"class_sel_sm_heavy_blu",
	"class_sel_sm_engineer_blu",

	"class_sel_sm_medic_blu",
	"class_sel_sm_sniper_blu",
	"class_sel_sm_spy_blu",

	"class_sel_sm_random_blu"
};

static const char *g_sClassImagesRed[] = {
	"",
	"class_sel_sm_scout_red",
	"class_sel_sm_soldier_red",
	"class_sel_sm_pyro_red",
	
	"class_sel_sm_demoman_red",
	"class_sel_sm_heavy_red",
	"class_sel_sm_engineer_red",
	
	"class_sel_sm_medic_red",
	"class_sel_sm_sniper_red",
	"class_sel_sm_spy_red",

	"class_sel_sm_random_red"
};

static char *g_sClassTips[][4] =
{
	{"#CKF3_ClassTips_Scout_1","#CKF3_ClassTips_Scout_2",NULL,NULL},
	{"#CKF3_ClassTips_Soldier_1","#CKF3_ClassTips_Soldier_2",NULL,NULL},
	{"#CKF3_ClassTips_Pyro_1","#CKF3_ClassTips_Pyro_2",NULL,NULL},
	{"#CKF3_ClassTips_Demoman_1","#CKF3_ClassTips_Demoman_2",NULL,NULL},
	{"#CKF3_ClassTips_Heavy_1",NULL,NULL,NULL},
	{"#CKF3_ClassTips_Engineer_1","#CKF3_ClassTips_Engineer_2","#CKF3_ClassTips_Engineer_3",NULL},
	{"#CKF3_ClassTips_Medic_1","#CKF3_ClassTips_Medic_2",NULL,NULL},
	{"#CKF3_ClassTips_Sniper_1","#CKF3_ClassTips_Sniper_2",NULL,NULL},
	{"#CKF3_ClassTips_Spy_1","#CKF3_ClassTips_Spy_2","#CKF3_ClassTips_Spy_3","#CKF3_ClassTips_Spy_4"},
	{"#CKF3_ClassTips_Random_1",NULL,NULL,NULL}
};

static model_t *g_modPlayerModel[11];

bool TFClassMenuButton::LoadClassPage(void)
{
	for(int i = 0; i < 10; ++i)
	{
		if (!Q_strcmp(g_sClassTitle[i+1], GetName()))
		{
			m_pPanel->SetZPos(4);

			m_iClassIndex = g_iRemapClassIndex[i];

			TFClassTipsItemPanel *pItem[4];
			for(int j = 0; j < 4; ++j)
			{;
				if(g_sClassTips[i][j] != NULL)
				{
					pItem[j] = new TFClassTipsItemPanel(m_pPanel, "ClassTipsItem");

					pItem[j]->SetBounds(0, 
						scheme()->GetProportionalScaledValueEx( GetScheme(), 10 + j * 35 ), 
						scheme()->GetProportionalScaledValueEx( GetScheme(), 230 ), 
						scheme()->GetProportionalScaledValueEx( GetScheme(), 30 ));

					pItem[j]->SetText(g_sClassTips[i][j]);

					pItem[j]->SetVisible(true);					
				}
			}
			return true;
		}
	}

	return false;
}

CTFClassMenu::CTFClassMenu( ) : CClassMenu( )
{
	m_iClassMenuKey = KEY_NONE;
	m_iTeam = TEAM_UNASSIGNED;

	m_pCountLabel = new Label( this, "CountLabel", "" );
	m_pClassTipsPanel = new EditablePanel( this, "ClassTipsPanel" );
	m_pClassMenuSelectLabel = new Label( this, "ClassMenuSelect", "#CKF3_SelectAClass" );
	m_pLocalPlayerImage = new ImagePanel( this, "LocalPlayerImage" );
	m_pLocalPlayerBG = new ScalableImagePanel( this, "LocalPlayerBG" );
	m_pPlayerModel = new TFPlayerModelPanel( this, "PlayerModel" );

	char tempName[64];

	for ( int i = 0 ; i < CLASS_COUNT_IMAGES ; ++i )
	{
		Q_snprintf( tempName, sizeof( tempName ), "countImage%d", i );
		m_pClassCountImages[i] = new ImagePanel( this, tempName );
	}
	for ( int i = 0 ; i < 9 ; ++i )
	{
		m_pClassCount[i] = new Label( this, g_sDialogVariables[i], "" );
	}

	for ( int i = 0 ; i < 2; ++i )
	{
		for(int j = 0; j < 10; ++j)
		{
			if(i == 0)
			{
				Q_snprintf(tempName, sizeof(tempName), "gfx/vgui/class/%s", g_sClassImagesRed[j+1]);
			}
			else
			{
				Q_snprintf(tempName, sizeof(tempName), "gfx/vgui/class/%s", g_sClassImagesBlue[j+1]);
			}
			m_pImageClassImage[i][j] = scheme()->GetImage(tempName, true);
		}
	}

	SetPaintBackgroundEnabled(false);

	LoadControlSettings( "Resource/UI/TFClassMenu.res" );

	vgui::ivgui()->AddTickSignal( GetVPanel() );
}

Panel *CTFClassMenu::CreateControlByName(const char *controlName)
{
	if(!Q_strcmp(controlName, "TFClassMenuButton"))
	{
		TFClassMenuButton *newButton = new TFClassMenuButton(this, "TFClassMenuButton", m_pClassTipsPanel);
		m_mouseoverButtons.AddToTail(newButton);
		return newButton;
	}
	return BaseClass::CreateControlByName(controlName);
}

void CTFClassMenu::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	for(int i = 0; i < m_mouseoverButtons.Count(); ++i)
	{
		TFClassMenuButton *button = dynamic_cast< TFClassMenuButton * >(m_mouseoverButtons[i]);
		if(button)
		{
			button->SetPreserveArmedButtons(true);
			button->SetArmedImage(m_pImageClassImage[m_iTeam-1][i]);
			button->SetSelectedImage(m_pImageClassImage[m_iTeam-1][i]);
		}
	}
}

void CTFClassMenu::PerformLayout()
{
	BaseClass::PerformLayout();

	m_pCountLabel = dynamic_cast< Label * >( FindChildByName( "CountLabel" ) );

	if ( m_pCountLabel )
	{
		m_pCountLabel->SizeToContents();
	}
}

void CTFClassMenu::PaintBackground()
{
	
}

void CTFClassMenu::ShowPanel( bool bShow )
{
	if ( bShow )
	{
		if (gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly())
			return;

		if(m_iTeam != TEAM_RED && m_iTeam != TEAM_BLUE)
			return;

		// can't change class if you're on the losing team during the "bonus time" after a team has won the round
		if ( *gCKFVars.g_iRoundStatus == ROUND_END ) //win
			return;

		Activate();
		SetMouseInputEnabled( true );

		for (int i = 0; i < m_mouseoverButtons.Count(); ++i)
		{
			TFClassMenuButton *button = dynamic_cast<TFClassMenuButton *>(m_mouseoverButtons[i]);
			if(button)
			{
				if(button->GetClassIndex() == (*gCKFVars.g_iDesiredClass))
				{
					button->SetArmed(true);
					button->ShowPage();
				}
				else
				{
					button->HidePage();
				}
			}
		}

		m_iScoreBoardKey = gameuifuncs->GetVGUI2KeyCodeForBind("showscores");
		m_iClassMenuKey = gameuifuncs->GetVGUI2KeyCodeForBind("changeclass"); 
		if(!m_iClassMenuKey)
			m_iClassMenuKey = gameuifuncs->GetVGUI2KeyCodeForBind("chooseclass"); 
	}
	else
	{
		for (int i = 0; i < m_mouseoverButtons.Count(); ++i)
		{
			TFClassMenuButton *button = dynamic_cast<TFClassMenuButton *>(m_mouseoverButtons[i]);
			if(button)
			{
				button->SetArmed(false);
				button->SetSelected(false);
			}
		}

		SetVisible( false );
		//SetMouseInputEnabled( false );
	}
}

void CTFClassMenu::OnKeyCodePressed( KeyCode code )
{
	m_KeyRepeat.KeyDown( code );

	if ( m_iClassMenuKey != KEY_NONE && m_iClassMenuKey == code )
	{
		if ( (*gCKFVars.g_iDesiredClass) != 0 )
		{
			ShowPanel( false );
		}
	}
	else if( code >= KEY_0 && code <= KEY_9 )
	{
		int buttonIndex = code-KEY_1;
		if(buttonIndex < 0)
			buttonIndex = m_mouseoverButtons.Count()-1;
		m_mouseoverButtons[buttonIndex]->DoClick();
	}
	else
	{
		BaseClass::OnKeyCodePressed( code );
	}
}

void CTFClassMenu::OnKeyCodeReleased( vgui::KeyCode code )
{
	m_KeyRepeat.KeyUp( code );

	BaseClass::OnKeyCodeReleased( code );
}

void CTFClassMenu::OnThink()
{
	vgui::KeyCode code = m_KeyRepeat.KeyRepeated();
	if ( code )
	{
		OnKeyCodePressed( code );
	}

	BaseClass::OnThink();
}

void CTFClassMenu::OnClose()
{
	ShowPanel( false );

	BaseClass::OnClose();
}

void CTFClassMenu::Update()
{
	// Force them to pick a class if they haven't picked one yet.
	if ( (*gCKFVars.g_iDesiredClass) != 0 )
	{
		SetVisibleButton( "CancelButton", true );
		m_pClassMenuSelectLabel->SetVisible( false );
	}
	else
	{
		SetVisibleButton( "CancelButton", false );
		m_pClassMenuSelectLabel->SetVisible( true );
	}
	InvalidateLayout( false, true );
}

int CTFClassMenu::GetSelectedClass( void )
{
	for (int i = 0; i < m_mouseoverButtons.Count(); ++i)
	{
		TFClassMenuButton *button = dynamic_cast<TFClassMenuButton *>(m_mouseoverButtons[i]);
		if(button && (button->IsArmed() || button->IsSelected()))
			return button->GetClassIndex();
	}
	return 0;
}

void CTFClassMenu::OnTick( void )
{
	//When a player changes teams, their class and team values don't get here 
	//necessarily before the command to update the class menu. This leads to the cancel button 
	//being visible and people cancelling before they have a class. check for class == TF_CLASS_UNDEFINED and if so
	//hide the cancel button

	if ( !IsVisible() )
		return;

	// Force them to pick a class if they haven't picked one yet.
	if ( (*gCKFVars.g_iDesiredClass) == 0 )
	{
		SetVisibleButton( "CancelButton", false );
		m_pClassMenuSelectLabel->SetVisible( true );
	}

	int iClassIndex = GetSelectedClass();
	m_pPlayerModel->SetClass(m_iTeam, iClassIndex);

	UpdateNumClassLabels( m_iTeam );

	BaseClass::OnTick();
}

void CTFClassMenu::SetVisible( bool state )
{
	BaseClass::SetVisible( state );

	m_KeyRepeat.Reset();

	if ( state )
	{
		//engine->pfnServerCmd( "menuopen\n" );			// to the server
		//engine->pfnClientCmd( "_cl_classmenuopen 1\n" );	// for other panels
	}
	else
	{
		engine->pfnClientCmd( "closemenu\n" );
		//engine->pfnClientCmd( "_cl_classmenuopen 0\n" );
	}
}

void CTFClassMenu::OnCommand(const char *command)
{
	if (Q_stricmp(command, "vguicancel"))
	{
		// we're selecting a class, so make sure we're not the class we have already been before sending to the server
		if ( Q_strstr( command, "joinclass " ) )
		{
			const char *pClass = command + Q_strlen( "joinclass " );
			int iClass = 0;

			if ( Q_stricmp( pClass, "scout" ) == 0 || Q_stricmp( pClass, "1" ) == 0)
			{
				iClass = CLASS_SCOUT;
			}
			else if ( Q_stricmp( pClass, "heavy" ) == 0 || Q_stricmp( pClass, "2" ) == 0)
			{
				iClass = CLASS_HEAVY;
			}
			else if ( Q_stricmp( pClass, "soldier" ) == 0 || Q_stricmp( pClass, "3" ) == 0)
			{
				iClass = CLASS_SOLDIER;
			}
			else if ( Q_stricmp( pClass, "pyro" ) == 0 || Q_stricmp( pClass, "4" ) == 0)
			{
				iClass = CLASS_PYRO;
			}
			else if ( Q_stricmp( pClass, "sniper" ) == 0 || Q_stricmp( pClass, "5" ) == 0)
			{
				iClass = CLASS_SNIPER;
			}
			else if ( Q_stricmp( pClass, "medic" ) == 0 || Q_stricmp( pClass, "6" ) == 0)
			{
				iClass = CLASS_MEDIC;
			}
			else if ( Q_stricmp( pClass, "engineer" ) == 0 || Q_stricmp( pClass, "7" ) == 0)
			{
				iClass = CLASS_ENGINEER;
			}
			else if ( Q_stricmp( pClass, "demoman" ) == 0 || Q_stricmp( pClass, "8" ) == 0)
			{
				iClass = CLASS_DEMOMAN;
			}
			else if ( Q_stricmp( pClass, "spy" ) == 0 || Q_stricmp( pClass, "9" ) == 0)
			{
				iClass = CLASS_SPY;
			}
			else if ( Q_stricmp( pClass, "random" ) == 0 || Q_stricmp( pClass, "10" ) == 0 || Q_stricmp( pClass, "0" ) == 0)
			{
				iClass = CLASS_RANDOM;
			}
			// are we selecting the class we have already been?
			if ( *gCKFVars.g_iDesiredClass != iClass )
			{
				engine->pfnClientCmd((char *)command);
				engine->pfnClientCmd("\n");
			}
			else
			{
				engine->pfnClientCmd( "closemenu\n" );
			}
		}
		else if ( !Q_strcmp( command, "cancelmenu" ) )
		{
			engine->pfnClientCmd( "closemenu\n" );
		}
	}
	ShowPanel( false );
}

void CTFClassMenu::UpdateNumClassLabels( int iTeam )
{
	int nTotalCount = 0;

	if ( iTeam < TEAM_RED || iTeam > TEAM_BLUE ) // invalid team number
		return;

	int iLocalClass = (*gCKFVars.g_iDesiredClass);
	if(iLocalClass >= CLASS_SCOUT && iLocalClass <= CLASS_SPY)
	{
		m_pLocalPlayerImage->SetImage( m_pImageClassImage[m_iTeam-1][g_iRemapClassButtonIndex[iLocalClass]] );

		m_pLocalPlayerImage->SetVisible(true);
		m_pLocalPlayerBG->SetVisible(true);
	}
	else
	{
		m_pLocalPlayerImage->SetVisible(false);
		m_pLocalPlayerBG->SetVisible(false);
	}

	for( int i = 0 ; i < 9 ; i++ )
	{
		int iClassIndex = g_iRemapClassIndex[i];
		int classCount = GetClassCounts( iTeam, iClassIndex );

		if ( classCount > 0 )
		{
			SetDialogVariable( g_sDialogVariables[i], classCount );
		}
		else
		{
			SetDialogVariable( g_sDialogVariables[i], "" );
		}

		if(iClassIndex == iLocalClass)
			classCount --;

		if ( nTotalCount < CLASS_COUNT_IMAGES )
		{
			for ( int j = 0 ; j < classCount ; ++j )
			{
				ImagePanel *pImage = m_pClassCountImages[nTotalCount];
				if ( pImage )
				{
					pImage->SetVisible( true );
					pImage->SetImage( m_pImageClassImage[m_iTeam-1][i] );
				}

				nTotalCount++;
				if ( nTotalCount >= CLASS_COUNT_IMAGES )
				{
					break;
				}
			}
		}
	}

	if ( nTotalCount == 0 )
	{
		// no classes for our team yet
		if ( m_pCountLabel && m_pCountLabel->IsVisible() )
		{
			m_pCountLabel->SetVisible( false );
		}
	}
	else
	{
		if ( m_pCountLabel && !m_pCountLabel->IsVisible() )
		{
			m_pCountLabel->SetVisible( true );
		}
	}

	// turn off any unused images
	while ( nTotalCount < CLASS_COUNT_IMAGES )
	{
		ImagePanel *pImage = m_pClassCountImages[nTotalCount];
		if ( pImage )
		{
			pImage->SetVisible( false );
		}

		nTotalCount++;
	}
}