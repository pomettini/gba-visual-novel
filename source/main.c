
#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#define TEXT_BUFFER_MAX 200
#define MAX_QUESTIONS 2

#define ERROR 0
#define OK 1

const char gscript_text[] = "P;Hello There|P;I'm a VN written in the Ink format|P;Do you like it?|Q;Yes, I like it!;4;No, I do not like it;6|P;Thank you!|J;END|P;Oh, I see|J;END";

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
    int line_questions_num;
    char line_questions_text[MAX_QUESTIONS][TEXT_BUFFER_MAX];
    int line_questions_jump_id[MAX_QUESTIONS];
} gscript_context;

gscript_context gscript_context_init(gscript_context *ctx)
{
    ctx->char_id = 0;
    memset(ctx->text_buffer, 0, TEXT_BUFFER_MAX);
    ctx->line_type = TYPE_UNDEFINED;
    ctx->line_questions_num = 0;
    memset(ctx->line_questions_text, 0, TEXT_BUFFER_MAX * MAX_QUESTIONS);
    memset(ctx->line_questions_jump_id, 0, MAX_QUESTIONS);
}

void gscript_parse_question(gscript_context *ctx)
{
    char *token;
    ctx->line_questions_num = 0;
    int count = 0;

    // Skipping the first two characters
    // Because they're useless
    for (token = strtok(ctx->text_buffer + 2, ";"); token; token = strtok(NULL, ";"))
    {
        // Counts to multiple of two
        // The first is going to be the text
        // The second is going to be the index
        if ((count + 1) % 2)
        {
            // If is't the text
            memcpy(ctx->line_questions_text[ctx->line_questions_num], token, TEXT_BUFFER_MAX);
        }
        else
        {
            // If it's the jump index
            ctx->line_questions_jump_id[ctx->line_questions_num] = atoi(token);
            ctx->line_questions_num++;
        }

        count++;
    }
}

void gscript_parse_jump(gscript_context *ctx)
{
}

gscript_type gscript_get_line_type(gscript_context *ctx)
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
    case TYPE_PRINT:
        break;

    case TYPE_QUESTION:
        gscript_parse_question(ctx);
        break;
    }
}

void gscript_print_line(gscript_context *ctx)
{
    switch (ctx->line_type)
    {
    case TYPE_PRINT:
        printf("- %s\n", ctx->text_buffer + 2);
        break;

    case TYPE_QUESTION:
        for (int i = 0; i < ctx->line_questions_num; i++)
        {
            printf("- * %s -> %d\n", ctx->line_questions_text[i], ctx->line_questions_jump_id[i]);
        }
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

int main()
{
    irqInit();
    irqEnable(IRQ_VBLANK);

    consoleDemoInit();

    gscript_context ctx;
    gscript_context_init(&ctx);

    int i = 0;
    while (i < 5)
    {
        // Parse line
        gscript_parse_current_line(&ctx);

        // Get line type
        gscript_get_line_type(&ctx);

        // Process line
        gscript_process_line(&ctx);

        // Print line
        gscript_print_line(&ctx);

        i++;
    }

    while (1)
    {
        VBlankIntrWait();
    }

    return 0;
}