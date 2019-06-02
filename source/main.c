#include <gba_input.h>

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "smgbalib.h"
#include "wallpaper.h"
#include "gscript.h"

#define TEXT_BUFFER_MAX 200
#define MAX_QUESTIONS 2
#define JUMP_BUFFER_SIZE 5

#define ERROR 0
#define OK 1

#define ever \
	;        \
	;

typedef enum gscript_type
{
	TYPE_UNDEFINED = 0,
	TYPE_ERROR = 1,
	TYPE_PRINT = 2,
	TYPE_QUESTION = 3,
	TYPE_JUMP = 4,
	TYPE_END = INT_MAX
} gscript_type;

typedef struct gscript_context
{
	int char_id;
	char text_buffer[TEXT_BUFFER_MAX];
	gscript_type line_type;
	int question_id;
	int line_questions_num;
	char line_questions_text[MAX_QUESTIONS][TEXT_BUFFER_MAX];
	int line_questions_jump[MAX_QUESTIONS];
} gscript_context;

void gscript_context_init(gscript_context *ctx)
{
	ctx->char_id = 0;
	memset(ctx->text_buffer, 0, TEXT_BUFFER_MAX);
	ctx->line_type = TYPE_UNDEFINED;
	ctx->question_id = 0;
	ctx->line_questions_num = 0;
	memset(ctx->line_questions_text, 0, TEXT_BUFFER_MAX * MAX_QUESTIONS);
	memset(ctx->line_questions_jump, 0, sizeof(int) * MAX_QUESTIONS);
}

void gscript_parse_question(gscript_context *ctx)
{
	char *token;
	ctx->line_questions_num = 0;
	int count = 0;

	// Skipping the first two characters
	// Because they're useless

	// TODO: Refactor this
	for (token = strtok(ctx->text_buffer + 2, ";"); token; token = strtok(NULL, ";"))
	{
		// Counts to multiple of two
		// The first is going to be the text
		// The second is going to be the index

		//TODO: Refactor this
		if ((count + 1) % 2)
		{
			// If it is the text
			memcpy(ctx->line_questions_text[ctx->line_questions_num], token, TEXT_BUFFER_MAX);
		}
		else
		{
			// If it's the jump index
			ctx->line_questions_jump[ctx->line_questions_num] = atoi(token);
			ctx->line_questions_num++;
		}

		count++;
	}
}

void gscript_parse_jump(gscript_context *ctx)
{
}

void gscript_question_jump(gscript_context *ctx)
{
	ctx->char_id = ctx->line_questions_jump[ctx->question_id];
}

void gscript_get_line_type(gscript_context *ctx)
{
	switch (ctx->text_buffer[0])
	{
	case 'P':
		ctx->line_type = TYPE_PRINT;
		break;

	case 'Q':
		ctx->line_type = TYPE_QUESTION;
		break;

	case 'J':
		ctx->line_type = TYPE_JUMP;
		break;

	case 'E':
		ctx->line_type = TYPE_END;
		break;

	default:
		ctx->line_type = TYPE_UNDEFINED;
		break;
	}
}

void gscript_process_line(gscript_context *ctx)
{
	switch (ctx->line_type)
	{
	case TYPE_UNDEFINED:
		break;

	case TYPE_ERROR:
		break;

	case TYPE_PRINT:
		break;

	case TYPE_QUESTION:
		gscript_parse_question(ctx);
		break;

	case TYPE_JUMP:
		break;

	case TYPE_END:
		break;
	}
}

void gscript_print_line(gscript_context *ctx)
{
	switch (ctx->line_type)
	{
	case TYPE_UNDEFINED:
		break;

	case TYPE_ERROR:
		break;

	case TYPE_PRINT:
		WriteStringWide(0, 100, ctx->text_buffer + 2);
		break;

	case TYPE_QUESTION:
		for (int i = 0; i < ctx->line_questions_num; i++)
		{
			// iprintf("- * %s -> %d\n", ctx->line_questions_text[i], ctx->line_questions_jump[i]);
			if (ctx->question_id == i)
			{
				WriteStringWide(0, 100 + (i * 10), "*");
				WriteStringWide(10, 100 + (i * 10), ctx->line_questions_text[i]);
			}
			else
			{
				WriteStringWide(10, 100 + (i * 10), ctx->line_questions_text[i]);
			}
		}
		break;

	case TYPE_JUMP:
		break;

	case TYPE_END:
		WriteStringWide(0, 100, "END OF THE STORY");
		break;
	}
}

void gscript_parse_current_line(gscript_context *ctx)
{
	memset(ctx->text_buffer, 0, TEXT_BUFFER_MAX);

	int buffer_id = 0;
	while (gscript_text[ctx->char_id] != '|')
	{
		ctx->text_buffer[buffer_id] = gscript_text[ctx->char_id];
		ctx->char_id++;
		buffer_id++;
	}

	ctx->char_id++;
}

void gscript_next(gscript_context *ctx)
{
	// Parse line
	gscript_parse_current_line(ctx);

	// Get line type
	gscript_get_line_type(ctx);

	// Process line
	gscript_process_line(ctx);
}

void gscript_question_move_prev(gscript_context *ctx)
{
	ctx->question_id--;

	if (ctx->question_id < 0)
		ctx->question_id = MAX_QUESTIONS - 1;
}

void gscript_question_move_next(gscript_context *ctx)
{
	ctx->question_id++;

	if (ctx->question_id > MAX_QUESTIONS - 1)
		ctx->question_id = 0;
}

void gscript_process_input(gscript_context *ctx)
{
	scanKeys();

	int keys_pressed = keysDown();

	switch (ctx->line_type)
	{
	case TYPE_UNDEFINED:
		break;

	case TYPE_ERROR:
		break;

	case TYPE_PRINT:
		if (keys_pressed & KEY_A)
			gscript_next(ctx);
		break;

	case TYPE_QUESTION:
		if (keys_pressed & KEY_UP)
			gscript_question_move_prev(ctx);
		if (keys_pressed & KEY_DOWN)
			gscript_question_move_next(ctx);
		if (keys_pressed & KEY_A)
		{
			gscript_question_jump(ctx);
			gscript_next(ctx);
		}
		break;

	case TYPE_JUMP:
		break;

	case TYPE_END:
		break;
	}
}

int main()
{
	Initialize();

	SetPalette((u16 *)wallpaperPalette);

	gscript_context ctx;
	gscript_context_init(&ctx);

	gscript_next(&ctx);

	for (ever)
	{
		EraseScreenNoPaletteNoFlip();

		DrawFullScreenOpaque((u16 *)wallpaperData);

		gscript_process_input(&ctx);

		// for (int y = 100; y < 160; y++)
		// {
		// 	for (int x = 0; x < 240; x++)
		// 	{
		// 		PlotSinglePixelOpaque(x, y, 0);
		// 	}
		// }

		gscript_print_line(&ctx);

		Flip();
	}

	return 0;
}
