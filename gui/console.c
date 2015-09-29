#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include "GUI.h"
#include "MULTIEDIT.h"
#include "WM.h"

#define CONSOLE_BUFFER 5000
#define CONSOLE_LINES 20

static char buffer[CONSOLE_BUFFER] = {0};
static uint32_t cur = 0;
static uint32_t line[CONSOLE_LINES] = {0};
static uint32_t nb_lines_written = 0;
static bool need_update = false;

static MULTIEDIT_HANDLE term;

void console_init(void)
{
	term = MULTIEDIT_CreateEx(	0, 0,
												500, 480,
												WM_HBKWIN, WM_CF_SHOW,
												MULTIEDIT_CF_AUTOSCROLLBAR_V |
												MULTIEDIT_CF_AUTOSCROLLBAR_H,
												GUI_ID_MULTIEDIT0,
												5000, (const char *)""
											);

	MULTIEDIT_SetFont(term, GUI_FONT_24_ASCII);
}

void console_write(char *str, size_t size)
{
	for (int i = 0; i < size; i++)
	{
		buffer[cur++] = *str;
		MULTIEDIT_AddText(term, (const char *)&buffer[cur-1]);
		if (*str == '\n')
		{
			if (nb_lines_written < CONSOLE_LINES)
			{
				nb_lines_written++;
				line[nb_lines_written] = cur;
			} else {
				need_update = true;
				for (int i = 0; i < CONSOLE_LINES-1; i++)
				{
					line[i] = line[i+1];
				}
				line[CONSOLE_LINES-1] = cur;
			}
		}
		str++;
	}
}

void console_disp(void)
{
	if(need_update)
	{
		GUI_Clear();
		need_update = false;
	}
	GUI_DispStringAt(&buffer[line[0]],0,0);
}
