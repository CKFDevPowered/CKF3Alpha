	// Command Menu Definition
// 
// "filename.res"
// {
//    "menuitem1"
//    {
//      "label"		"#GoToB"          // lable name shown in game, # = localized string
//      "command"	"echo hallo"      // a command string
//      "toggle"	"sv_cheats" 	  // a 0/1 toggle cvar 
//      "rule"		"map"             // visibility rules : "none", "team", "map","class"	
//      "ruledata"	"de_dust"	  // rule data, eg map name or team number
//    }
//   
//   "menuitem2"
//   {
//	...
//   }
//
//   ...
//
// }
//
//--------------------------------------------------------
// Everything below here is editable

"spectatormenu.res"
{
	"menuitem1"
	{
		"label"		"#Valve_Close"	// name shown in game
		"command"	"spec_menu 0"	// type data
	}
	
	"menuitem2"
	{
		"label"		"#Valve_Settings"
		
		"menuitem21"
		{
			"label"		"#Valve_Chat_Messages"
			"toggle"	"hud_saytext_internal"
		}
		
		"menuitem22"
		{
			"label"		"#Valve_Show_Status"
			"toggle"	"spec_drawstatus_internal"
		}
		
		"menuitem23"
		{
			"label"		"#Valve_View_Cone"
			"toggle"	"spec_drawcone_internal"
		}
		
		"menuitem24"
		{
			"label"		"#Valve_Player_Names"
			"toggle"	"spec_drawnames_internal"
		}
	}
	
	"menuitem3"
	{
		"label"		"#Valve_PIP"
		"command"	"spec_mode -1 -1"
	}
	
	"menuitem4"
	{
		"label"		"#Valve_Auto_Director"
		"toggle"	"spec_autodirector_internal"
	}
	
	"menuitem5"
	{
		"label"		"#Valve_Show_Scores"
		"command"	"togglescores"
	}
}

// Here are the rest of the buttons and submenus
// You can change these safely if you want.






