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

"commandmenu.res"
{
	"menuitem1"
	{
		"label"		"#Cstrike_HELP"
										
		"menuitem11"
		{
			"label"		"#Cstrike_Map_Desc"
			"command"	"!MAPBRIEFING"
		}
		
		"menuitem12"
		{
			"label"		"#Cstrike_Time_Left"
			"command"	"timeleft"
		}
		
		"menuitem13"
		{
			"label"		"#Cstrike_Adjust_Crosshair"
			"command"	"adjust_crosshair"
		}
		
		"menuitem14"
		{
			"label"		"#Cstrike_Use_Right_Hand"
			"toggle"	"cl_righthand"
		}
	}
	
	"menuitem2"
	{
		"label"		"#Cstrike_CHANGE_TEAM"
		"command"	"chooseteam"
	}
		
	"menuitem3"
	{
		"label"		"#Cstrike_TEAM_MESSAGE"

		"menuitem1"
		{
			"label"		"#Cstrike_Acknowledged"
			"command"	"say_team Acknowledged"
		}
		
		"menuitem2"
		{
			"label"		"#Cstrike_Negative"
			"command"	"say_team Negative"
		}
		
		"menuitem3"
		{
			"label"		"#Cstrike_Go"
			"command"	"say_team Go Go Go!"
		}
		
		"menuitem4"
		{
			"label"		"#Cstrike_On_My_Way"
			"command"	"say_team On my way"
		}
		
		"menuitem5"
		{
			"label"		"#Cstrike_Need_Backup"
			"command"	"say_team Need backup!"
		}
	}
	
	"menuitem4"
	{
		"label"		"#Cstrike_DROP_CURRENT_ITEM"
		"command"	"drop"
	}
}




