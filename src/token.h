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

#ifndef H_CTPL_TOKEN_H
#define H_CTPL_TOKEN_H

#include <glib.h>

G_BEGIN_DECLS


/**
 * CtplTokenType:
 * @CTPL_TOKEN_TYPE_DATA: Data flow, not a real token
 * @CTPL_TOKEN_TYPE_VAR: A variable that should be replaced
 * @CTPL_TOKEN_TYPE_FOR: A loop through an array of value
 * @CTPL_TOKEN_TYPE_IF: A conditional branching
 * 
 * 
 */
typedef enum e_CtplTokenType
{
  CTPL_TOKEN_TYPE_DATA,
  CTPL_TOKEN_TYPE_VAR,
  CTPL_TOKEN_TYPE_FOR,
  CTPL_TOKEN_TYPE_IF
} CtplTokenType;

typedef struct s_CtplToken    CtplToken;
typedef struct s_CtplTokenFor CtplTokenFor;
typedef struct s_CtplTokenIf  CtplTokenIf;

struct s_CtplTokenFor
{
  char       *array;
  char       *iter;
  CtplToken  *children;
};

struct s_CtplTokenIf
{
  char       *condition;
  CtplToken  *if_children;
  CtplToken  *else_children;
};

struct s_CtplToken
{
  CtplTokenType type;
  union {
    char         *t_data;
    char         *t_var;
    CtplTokenFor  t_for;
    CtplTokenIf   t_if;
  } token;
  CtplToken    *prev;
  CtplToken    *next;
};


CtplToken    *ctpl_token_new_data (const char *data,
                                   gssize      len);
CtplToken    *ctpl_token_new_var  (const char *var,
                                   gssize      len);
CtplToken    *ctpl_token_new_for  (const char *array,
                                   const char *iterator,
                                   CtplToken  *children);
CtplToken    *ctpl_token_new_if   (const char *condition,
                                   CtplToken  *if_children,
                                   CtplToken  *else_children);
void          ctpl_token_free     (CtplToken *token,
                                   gboolean   chain);
void          ctpl_token_append   (CtplToken *token,
                                   CtplToken *brother);
void          ctpl_token_prepend  (CtplToken *token,
                                   CtplToken *brother);
void          ctpl_token_dump     (const CtplToken *token,
                                   gboolean         chain);


G_END_DECLS

#endif /* guard */
