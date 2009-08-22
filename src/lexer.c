/* 
 * 
 * Copyright (C) 2007-2009 Colomban "Ban" Wendling <ban@herbesfolles.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include "lexer.h"
#include "token.h"
#include <mb.h>
#include <glib.h>
#include <string.h>


/* TODO: throw GError in place of error messages */


#define SYMBOLCHARS "abcdefghijklmnopqrstuvwxyz" \
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
                    "0123456789" \
                    "_"
#define BLANKCHARS " \t\v\r\n"
#define EXPRCHARS "()+-/*=><" \
                  "." /* for floating point values */ \
                  BLANKCHARS /* allow any blank character in expr */ \
                  SYMBOLCHARS /* for references to symbols */


typedef struct s_LexerState LexerState;

struct s_LexerState
{
  int block_depth;
};


static CtplToken   *ctpl_lexer_lex_internal   (MB          *mb,
                                               LexerState  *state);
static CtplToken   *ctpl_lexer_read_token     (MB          *mb,
                                               LexerState  *state);


static char *
read_word (MB          *mb,
           const char  *accept)
{
  int   c;
  gsize start;
  gsize len;
  char *word;
  
  start = mb_tell (mb);
  do {
    c = mb_getc (mb);
  } while (! mb_eof (mb) && strchr (accept, c));
  len = (mb_tell (mb) - start) - 1;
  word = g_malloc (len + 1);
  if (word) {
    mb_seek (mb, start, MB_SEEK_SET);
    mb_read (mb, word, len);
    word[len] = 0;
    /*g_debug ("Next read character will be '%c'", mb_cur_char (mb));*/
  }
  
  return word;
}

static char *
read_symbol (MB *mb)
{
  return read_word (mb, SYMBOLCHARS);
}

static char *
read_expr (MB *mb)
{
  return read_word (mb, EXPRCHARS);
}

static gsize
skip_blank (MB *mb)
{
  gsize n = 0;
  int   c;
  
  do {
    c = mb_getc (mb);
    n++;
  } while (! mb_eof (mb) && strchr (BLANKCHARS, c));
  if (! strchr (BLANKCHARS, c))
    mb_seek (mb, -1, MB_SEEK_CUR);
  
  return n;
}


/* reads the data part of a if, aka the expression (e.g. " a > b" in "if a > b")
 * Return a new token or %NULL on error */
static CtplToken *
ctpl_lexer_read_token_tpl_if (MB          *mb,
                              LexerState  *state)
{
  char       *expr;
  CtplToken  *token = NULL;
  
  g_debug ("if?");
  skip_blank (mb);
  expr = read_expr (mb);
  if (expr) {
    skip_blank (mb);
    if (mb_getc (mb) != CTPL_END_CHAR) {
      /* fail */
      g_error ("if: invalid character in condition or missing end character");
    } else {
      g_debug ("if token: `if %s`", expr);
      /* FIXME: read if and else children */
      token = ctpl_token_new_if (expr, NULL, NULL);
      state->block_depth ++;
    }
  }
  g_free (expr);
  
  return token;
}

/* reads the data part of a for, eg " i in array" for a "for i in array"
 * Return a new token or %NULL on error */
static CtplToken *
ctpl_lexer_read_token_tpl_for (MB          *mb,
                               LexerState  *state)
{
  CtplToken *token = NULL;
  char *iter_name;
  char *keyword_in;
  char *array_name;
  
  g_debug ("for?");
  skip_blank (mb);
  iter_name = read_symbol (mb);
  if (! iter_name) {
    /* fail */
    g_error ("for: failed to read iter name");
  } else {
    g_debug ("for: iter is '%s'", iter_name);
    skip_blank (mb);
    keyword_in = read_symbol (mb);
    if (! keyword_in || strcmp (keyword_in, "in") != 0) {
      /* fail */
      g_error ("for: 'in' keyword missing after iter, got '%s'", keyword_in);
    } else {
      skip_blank (mb);
      array_name = read_symbol (mb);
      if (! array_name) {
        /* fail */
        g_error ("for: failed to read array name");
      } else {
        skip_blank (mb);
        if (mb_getc (mb) != CTPL_END_CHAR) {
          /* fail */
          g_error ("for: no ending character %c", CTPL_END_CHAR);
        } else {
          g_debug ("for token: `for %s in %s`", iter_name, array_name);
          /* FIXME: read children */
          state->block_depth ++;
          token = ctpl_token_new_for (array_name, iter_name,
                                      ctpl_lexer_lex_internal (mb, state));
        }
      }
      g_free (array_name);
    }
    g_free (keyword_in);
  }
  g_free (iter_name);
  
  return token;
}

/* reads a end block end (} of a {end} block)
 * Return a new token or %NULL on error or at data end */
static gboolean
ctpl_lexer_read_token_tpl_end (MB          *mb,
                               LexerState  *state)
{
  gboolean rv = FALSE;
  
  g_debug ("end?");
  skip_blank (mb);
  if (mb_getc (mb) != CTPL_END_CHAR) {
    /* fail, missing } at the end */
    g_error ("end: missing '%c' at the end", CTPL_END_CHAR);
  } else {
    g_debug ("block end");
    state->block_depth --;
    if (state->block_depth < 0) {
      /* fail */
      g_error ("found end of non-existing block");
    } else {
      rv = TRUE;
    }
  }
  
  return rv;
}

/* reads a real token */
static CtplToken *
ctpl_lexer_read_token_tpl (MB          *mb,
                           LexerState  *state)
{
  CtplToken *token = NULL;
  
  /* ensure the first character is a start character */
  if (mb_getc (mb) != CTPL_START_CHAR) {
    /* fail */
    g_error ("expected '%c' before '%c'", CTPL_START_CHAR, mb_cur_char (mb));
  } else {
    gboolean  need_end = TRUE; /* whether the block needs an {end} statement */
    char     *first_word;
    
    skip_blank (mb);
    first_word = read_symbol (mb);
    g_debug ("read word '%s'", first_word);
    if        (strcmp (first_word, "if") == 0) {
      /* an if condition:
       * if expr */
      token = ctpl_lexer_read_token_tpl_if (mb, state);
    } else if (strcmp (first_word, "for") == 0) {
      /* a for loop:
       * for iter in array */
      token = ctpl_lexer_read_token_tpl_for (mb, state);
    } else if (strcmp (first_word, "end") == 0) {
      /* a block end:
       * {end} */
      if (! ctpl_lexer_read_token_tpl_end (mb, state)) {
        /* fail */
      } else {
        /*token = ctpl_lexer_read_token (mb, state);*/
      }
    } else if (first_word != NULL) {
      /* a var:
       * {:BLANKCHARS:?:WORDCHARS::BLANKCHARS:?} */
      g_debug ("var?");
      need_end = FALSE;
      skip_blank (mb);
      if (mb_getc (mb) != CTPL_END_CHAR) {
        /* fail, missing } at the end */
        g_error ("var: missing '%c' at block end", CTPL_END_CHAR);
      } else {
        g_debug ("var: %s", first_word);
        token = ctpl_token_new_var (first_word, -1);
      }
    } else {
      g_error ("WTF???");
    }
    g_free (first_word);
  }
  
  return token;
}


/* skips data characters
 * Returns: %TRUE on success, %FALSE otherwise (syntax error) */
static gboolean
forward_to_non_data (MB *mb)
{
  int prev_c;
  int c = 0;
  gboolean rv = TRUE;
  
  do {
    prev_c = c;
    c = mb_getc (mb);
  } while (! mb_eof (mb) &&
           ((c != CTPL_START_CHAR && c != CTPL_END_CHAR) || prev_c == '\\'));
  
  if (! mb_eof (mb)) {
    mb_seek (mb, -1, MB_SEEK_CUR);
    /* if we reached an unescaped end character ('}'), fail */
    rv = (c == CTPL_START_CHAR);
  }
  
  return rv;
}

/* reads a data token
 * Returns: A new token on success, %NULL otherwise (syntax error) */
static CtplToken *
ctpl_lexer_read_token_data (MB         *mb,
                            LexerState *state)
{
  gsize start;
  CtplToken *token = NULL;
  
  start = mb_tell (mb);
  if (! forward_to_non_data (mb)) {
    /* fail */
    g_error ("unexpected '%c'", CTPL_END_CHAR);
  } else {
    gsize len;
    char *buf;
    
    len = mb_tell (mb) - start;
    buf = g_malloc (len);
    if (buf) {
      mb_seek (mb, start, MB_SEEK_SET);
      mb_read (mb, buf, len);
      token = ctpl_token_new_data (buf, len);
    }
    g_free (buf);
  }
  
  return token;
}

static CtplToken *
ctpl_lexer_read_token (MB          *mb,
                       LexerState  *state)
{
  CtplToken *token = NULL;
  
  g_debug ("Will read a token (starts with %c)", mb_cur_char (mb));
  switch (mb_cur_char (mb)) {
    case CTPL_START_CHAR:
      g_debug ("start of a template recognised syntax");
      token = ctpl_lexer_read_token_tpl (mb, state);
      break;
    
    case CTPL_END_CHAR:
      /* fail here */
      g_error ("syntax error near '%c' token", CTPL_END_CHAR);
      break;
    
    default:
      token = ctpl_lexer_read_token_data (mb, state);
  }
  
  return token;
}

static CtplToken *
ctpl_lexer_lex_internal (MB          *mb,
                         LexerState  *state)
{
  CtplToken  *token = NULL;
  CtplToken  *root = NULL;
  
  while ((token = ctpl_lexer_read_token (mb, state)) != NULL) {
    if (! root) {
      root = token;
    } else {
      ctpl_token_append (root, token);
    }
  }
  
  return root;
}

CtplToken *
ctpl_lexer_lex (MB *mb)
{
  CtplToken  *root;
  LexerState  lex_state = {0};
  
  root = ctpl_lexer_lex_internal (mb, &lex_state);
  if (lex_state.block_depth != 0) {
    g_error ("syntax error: block close count doesn't match block open count");
  }
  
  return root;
}

void
ctpl_lexer_free_tree (CtplToken *root)
{
  ctpl_token_free (root, TRUE);
}

void
ctpl_lexer_dump_tree (const CtplToken *root)
{
  ctpl_token_dump (root, TRUE);
}
