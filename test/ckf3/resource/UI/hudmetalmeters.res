"Resource/UI/HudMetalMeters.res"
{
	"HudMetalMeter"
	{
		"fieldName"		"HudMetalMeter"
		"visible"		"1"
		"enabled"		"1"
		"xpos"			"r192"
		"ypos"			"r50"
		"wide"			"100"
		"tall"			"50"
	}
	
	"HudMetalMeterBG"
	{
		"ControlName"	"ImagePanel"
		"fieldName"		"HudMetalMeterBG"
		"xpos"			"12"
		"ypos"			"0"
		"zpos"			"0"
		"wide"			"64"
		"tall"			"40"
		"visible"		"1"
		"enabled"		"1"
		"image"			"resource/tga/mask_weapon_small_red"
		"scaleImage"	"1"
	}
	
	"MetalIcon"
	{
		"ControlName"	"ImagePanel"
		"fieldName"		"MetalIcon"
		"xpos"			"21"
		"ypos"			"12"
		"wide"			"10"
		"tall"			"10"
		"visible"		"1"
		"enabled"		"1"
		"image"			"resource/tga/ico_metal"
		"scaleImage"	"1"
		"drawcolor"		"TanLight"
	}
	
	"MetalMeterLabel"
	{
		"ControlName"			"Label"
		"fieldName"				"MetalMeterLabel"
		"xpos"					"25"
		"ypos"					"23"
		"zpos"					"2"
		"wide"					"41"
		"tall"					"15"
		"autoResize"			"1"
		"pinCorner"				"2"
		"visible"				"1"
		"enabled"				"1"
		"tabPosition"			"0"
		"labelText"				"#CKF3_Metal"
		"textAlignment"			"center"
		"dulltext"				"0"
		"brighttext"			"0"
		"font"					"TFFontSmall"
	}

	"MetalMeterCount"
	{
		"ControlName"			"Label"
		"fieldName"				"MetalMeterCount"
		"xpos"					"28"
		"ypos"					"10"
		"zpos"					"2"
		"wide"					"40"
		"tall"					"20"	
		"pinCorner"				"2"
		"visible"				"1"
		"enabled"				"1"
		"tabPosition"			"0"
		"labelText"				"%metal%"
		"textAlignment"			"north"
		"dulltext"				"0"
		"brighttext"			"0"
		"font"					"HudFontMediumSmall"
	}
}
