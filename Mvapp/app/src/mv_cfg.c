#include "mv_cfg.h"

extern BOOL Str2Hex(U8 *data, U8 *value);

char	CFG_CHECK_STRING[MVCFG_MAX][30] = {
						"source_dir",
						"set_type",
						"png_test",
						"yinhe_test",
						"default_value",
						"noback_image",
						"factory_mode",
						"skin"
					};

void MV_Parser_CFG_Value( char *Temp, char *tempSection )
{
	U16		i, j;

	for ( i = 0 ; i < strlen(tempSection) ; i++ )
	{
		if ( tempSection[i] == '=' )
			break;
	}

	j = i+1;

	for ( i = 0 ; i < strlen(tempSection) - j ; i++ )
	{
		if ( tempSection[i+j] == '\n' )
			break;

		Temp[i] = tempSection[i+j];
	}
}

MV_CFG MV_CFG_Namecheck(char *tempSection)
{
	int 	i;
	char	Temp[20];

	for ( i = 0 ; i < MVCFG_MAX ; i++ )
	{
		memset (Temp, 0, sizeof(char) * 20);
		strncpy(Temp, tempSection, strlen(CFG_CHECK_STRING[i]));

		if ( strcmp ( CFG_CHECK_STRING[i], Temp ) == 0 )
			break;
	}

	return i;
}

void Parser_Color_CFG(MV_COLOR_CFG Color_cfg, char *temp_str)
{
	int 			length = strlen(temp_str);
	int				i, j = 0;
	char			color_str[5];
	stMV_Color_RGB	stTemp;

	memset(color_str, 0x00, 5 );

	strncpy ( color_str, &temp_str[0], 2);
	Str2Hex(color_str, &stTemp.MV_R);

	memset(color_str, 0x00, 5 );

	for ( i = 0 ; i < length ; i++ )
	{
		if ( temp_str[i] == ':' )
		{
			strncpy ( color_str, &temp_str[i+1], 2);
			switch(j)
			{
				case 0:
					Str2Hex(color_str, &stTemp.MV_G);
					break;
				case 1:
					Str2Hex(color_str, &stTemp.MV_B);
					break;
				default:
					Str2Hex(color_str, &stTemp.MV_A);
					break;
			}
			j++;
			memset(color_str, 0x00, 5 );
		}
	}

	switch(Color_cfg)
	{
		case MV_COLOR_MAIN_BACK:
			CFG_Back_Color = stTemp;
			break;

		case MV_COLOR_MENU_BACK_COLOR:
			CFG_Menu_Back_Color = stTemp;
			break;

		case MV_COLOR_INFO_TOP_BACK:
			CFG_Info_top_Color = stTemp;
			break;

		case MV_COLOR_INFO_BOT_BACK:
			CFG_Info_bot_Color = stTemp;
			break;

		default:
			break;
	}
}

MV_CFG_RETURN MV_LoadCFGFile(void)
{
	FILE* 			fp;
    char 			tempSection [CFG_MAX_COL + 2];
	char			Temp[100];
	MV_CFG_RETURN	ret = CFG_OK;

	if (!(fp = fopen(CFG_FILE, "r")))
	{
         ret = CFG_NOFILE;
		 return ret;
	}

	while (!feof(fp)) {
		memset (Temp, 0, sizeof(char) * 100);

        if (!fgets(tempSection, CFG_MAX_COL, fp)) {
			fclose (fp);
			ret = CFG_READ_FAIL;
			return ret;
        }

		MV_Parser_CFG_Value(Temp, tempSection);

		switch( MV_CFG_Namecheck(tempSection) )
		{
			case MVCFG_RESOUCE:					// Resource File Directory .. Directory full name
				strcpy ( CFG_Resource , Temp);
				sprintf(BOOT_LOGO, "%sbootlogo.mpg", CFG_Resource);
				sprintf(RADIO_BACK, "%sradioback.mpg", CFG_Resource);
				break;
			case MVCFG_SET_TYPE:						// DVB-C, T, S, Combo
				if ( strcmp (Temp, "on" ) == 0 )
					CFG_Set_Type = TRUE;
				else
					CFG_Set_Type = FALSE;
				break;
			case MVCFG_PNG_TEST:						// PNG File Test .. on/off
				if ( strcmp (Temp, "on" ) == 0 )
					CFG_PNG_Test = TRUE;
				else
					CFG_PNG_Test = FALSE;
				break;
			case MVCFG_FOR_INHEO_TEST:				// Yinhe TEST --> Auto Display Menu rootine .. on/off
				if ( strcmp (Temp, "on" ) == 0 )
					CFG_Yinhe_Test = TRUE;
				else
					CFG_Yinhe_Test = FALSE;
				break;
			case MVCFG_LOAD_DEFAILT_VALUE:			// Auto Setting default value on Booting Time .. on/off
				if ( strcmp (Temp, "on" ) == 0 )
					CFG_Default_Value = TRUE;
				else
					CFG_Default_Value = FALSE;
				break;
			case MVCFG_NO_BACK_IMAGE:
				if ( strcmp (Temp, "on" ) == 0 )
					CFG_Noback_Image = TRUE;
				else
					CFG_Noback_Image = FALSE;
				break;
			case MVCFG_FAC_TEST:
				if ( strcmp (Temp, "on" ) == 0 )
					CFG_Factory_Mode = TRUE;
				else
					CFG_Factory_Mode = FALSE;
				break;
			case MVCFG_SKIN:
				CFG_Skin_Kind = (U8)atoi(Temp);
				break;
			default:
				ret = CFG_NO_NAME;
				break;
		}
    }

	fclose (fp);
	return ret;
}

MV_CFG_RETURN MV_Resource_Change_Cfg_File(U8 u8Skin_kind)
{
	FILE* 	fp;
	FILE*	fp_temp;
    char 	tempSection [CFG_MAX_COL + 2];
	char	Temp[100];
	char 	ShellCommand[64];
	int		i = 0;

	if (!(fp = fopen(CFG_FILE, "r")))
	{
		printf("\n File open Error\n");
        return CFG_NOFILE;
	}
	if (!(fp_temp = fopen(TEMP_FILE, "w+a")))
	{
		printf("\n Temp File create Error\n");
		fclose (fp);
        return CFG_NOFILE;
	}

	while (!feof(fp)) {
		memset (Temp, 0, sizeof(char) * 100);

        if (!fgets(tempSection, CFG_MAX_COL, fp)) {
			printf("\n End of mv_app.cfg file!\n");
			break;
        }

		if ( MV_CFG_Namecheck(tempSection) == MVCFG_RESOUCE )
		{
			sprintf(Temp, "source_dir=/usr/work0/resource%d/\n", u8Skin_kind);
			sprintf(CFG_Resource, "/usr/work0/resource%d/", u8Skin_kind);
			fputs(Temp, fp_temp);
		} else {
			fputs(tempSection, fp_temp);
		}
		//printf("%d\n",i);
		i++;
    }

	fclose (fp);
	fclose (fp_temp);

	sprintf(ShellCommand, "rm %s", CFG_FILE);
	if ( system( ShellCommand ) )
		return CFG_READ_FAIL;

	sprintf(ShellCommand, "cp %s %s", TEMP_FILE, CFG_FILE);
	if ( system( ShellCommand ) )
		return CFG_READ_FAIL;

	sprintf(ShellCommand, "rm %s", TEMP_FILE);
	if ( system( ShellCommand ) )
		return CFG_READ_FAIL;

	return CFG_OK;
}

