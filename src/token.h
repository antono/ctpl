/* 
 * 
 * Copyright (C) 2009-2010 Colomban Wendling <ban@herbesfolles.org>
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

#include "value.h"
#include <glib.h>

G_BEGIN_DECLS


/**
 * CtplToken:
 * 
 * The #CtplToken opaque structure.
 */
typedef struct _CtplToken             CtplToken;
/**
 * CtplTokenExpr:
 * 
 * Represents an expression token.
 */
typedef struct _CtplTokenExpr         CtplTokenExpr;

void          ctpl_token_free               (CtplToken *token,
                                             gboolean   chain);
void          ctpl_token_expr_free          (CtplTokenExpr *token,
                                             gboolean       recurse);
void          ctpl_token_dump               (const CtplToken *token,
                                             gboolean         chain);
void          ctpl_token_expr_dump          (const CtplTokenExpr *token);


G_END_DECLS

#endif /* guard */
