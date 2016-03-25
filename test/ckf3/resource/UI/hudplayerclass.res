"Resource/UI/HudPlayerClass.res"
{
	// player class data
	"HudPlayerClass"
	{
		"ControlName"	"EditablePanel"
		"fieldName"		"HudPlayerClass"
		"xpos"			"0"
		"ypos"			"0"
		"zpos"			"1"
		"wide"			"f0"
		"tall"			"480"
		"visible"		"1"
		"enabled"		"1"
	}
	"PlayerStatusClassImage"
	{
		"ControlName"	"TFClassImage"
		"fieldName"		"PlayerStatusClassImage"
		"xpos"			"25"
		"ypos"			"r88"
		"zpos"			"2"
		"wide"			"75"
		"tall"			"75"
		"visible"		"1"
		"enabled"		"1"
		"image"			""
		"scaleImage"	"1"	
	}
	"PlayerStatusSpyImage"
	{
		"ControlName"	"ImagePanel"
		"fieldName"		"PlayerStatusSpyImage"
		"xpos"			"3"
		"ypos"			"r67"
		"zpos"			"2"
		"wide"			"55"
		"tall"			"55"
		"visible"		"1"
		"enabled"		"1"
		"image"			"resource/tga/class_spy_red"
		"scaleImage"	"1"
	}	
	"PlayerStatusSpyOutlineImage"
	{
		"ControlName"	"ImagePanel"
		"fieldName"		"PlayerStatusSpyOutlineImage"
		"xpos"			"3"	
		"ypos"			"r67"
		"zpos"			"7"
		"wide"			"55"
		"tall"			"55"
		"visible"		"0"
		"enabled"		"1"
		"image"			"resource/tga/disguise_hint"
		"scaleImage"	"1"	
	}		
	"PlayerStatusClassImageBG"
	{
		"ControlName"	"ImagePanel"
		"fieldName"		"PlayerStatusClassImageBG"
		"xpos"			"9"
		"ypos"			"r60"
		"zpos"			"1"		
		"wide"			"100"
		"tall"			"50"
		"visible"		"1"
		"enabled"		"1"
		"scaleImage"	"1"
		"image"			"resource/tga/mask_class_red"	
	}
	"PlayerStatusPlayerModel"
	{
		"ControlName"	"TFPlayerModelPanel"
		"fieldName"		"PlayerStatusPlayerModel"
		"xpos"			"0"
		"ypos"			"r120"
		"zpos"			"3"
		"wide"			"100"
		"tall"			"109"
		"visible"		"1"
		"enabled"		"1"
		
		"model"			""
		"skin"			"0"
		"angles" 		"0 150 0"
		"origin" 		"200 0 0"
		
		"customclassdata"
		{
			"scout"
			{
				"angles" 		"0 200 0"
				"origin" 		"125 -3 -15"
			}
			"soldier"
			{
				"angles" 		"0 155 0"
				"origin" 		"175 -10 -15"
			}
			"pyro"
			{
				"angles" 		"0 170 0"
				"origin" 		"175 -10 -18"
			}
			"demoman"
			{
				"angles" 		"0 180 0"
				"origin" 		"175 -10 -15"
			}
			"heavy"
			{
				"angles" 		"0 180 0"
				"origin" 		"190 -5 -20"
			}
			"engineer"
			{
				"angles" 		"0 165 0"
				"origin" 		"130 -5 -20"
			}
			"medic"
			{
				"angles" 		"0 170 0"
				"origin" 		"150 -6 -20"
			}
			"sniper"
			{
				"angles" 		"0 165 0"
				"origin" 		"175 -10 -18"
			}
			"spy"
			{
				"angles" 		"0 175 0"
				"origin" 		"150 -8 -20"
			}
		}
	}
}
